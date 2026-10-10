#include "Scene11_Grid.h"
#include <cstdio>

using namespace YardDims;

static const float FLIGHT_DELAY = 1.5f, FLIGHT_SPEED = 42.0f;

void Scene11_Grid::enter(bool continuous) {
    t = 0;
    replay = false;
    closedAt = -1.0f;
    if (!continuous) {
        world.completeUpTo(ST_GRID);
        fromEye = Vec3(76.5f, 14.0f, 50.0f);
        fromTarget = Vec3(83.0f, 10.5f, 64.0f);
    }
    world.breakerClosed = false;
    world.trafoXrayOn = false;
}

void Scene11_Grid::update(float dt) {
    t += dt;
    if (world.breakerClosed && closedAt < 0) closedAt = t;
    if (!world.breakerClosed) closedAt = -1.0f;
    world.update(dt);
}

float Scene11_Grid::flightS() const {
    if (closedAt < 0) return 0.0f;
    return clampf((t - closedAt - FLIGHT_DELAY) * FLIGHT_SPEED, 0.0f, world.line.lineLength - 90.0f);
}

void Scene11_Grid::camera(Camera& cam) {
    // the yard: look along the bay towards the breakers and the line gantry
    CamKey keys[] = {
        {0.0f, fromEye, fromTarget},
        {4.0f, Vec3(89.0f, 17.0f, 46.0f), Vec3(104.0f, 9.0f, 63.0f)},
        {40.0f, Vec3(92.0f, 16.0f, 47.0f), Vec3(106.0f, 9.0f, 64.0f)},
    };
    Vec3 eye, target;
    evalCamPath(keys, 3, t, eye, target);
    // after the breaker closes: fly with the energy along the power line
    if (closedAt >= 0) {
        float s = flightS();
        float k = smoothstepf(0.0f, 3.0f, t - closedAt - FLIGHT_DELAY);
        Vec3 p = world.line.pointAlong(s + 20.0f);
        Vec3 flyEye = p + Vec3(-18.0f, 10.0f, -26.0f);
        Vec3 flyTarget = world.line.pointAlong(s + 90.0f) - Vec3(0, 6.0f, 0);
        eye = mix(eye, flyEye, k);
        target = mix(target, flyTarget, k);
        shadowAt = p;
    } else {
        shadowAt = Vec3(100, 8, 70);
    }
    cam.eye = eye;
    cam.target = target;
    cam.fovDeg = 55.0f;
}

void Scene11_Grid::hud(Ui& ui, const Mat4& vp) {
    if (closedAt < 0 && t > 4.0f) {
        // near parts get labels to the right, far parts to the left, so they do not pile up
        pointerLabel(ui, vp, Vec3(97.1f, 11.2f, BAY_Z[0]), 60, 60, "DISCONNECTOR");
        pointerLabel(ui, vp, Vec3(BREAKER_X, 12.5f, BAY_Z[0]), 40, -130, "CIRCUIT BREAKER");
        pointerLabel(ui, vp, Vec3(107.0f, 12.0f, BAY_Z[0]), -130, -110, "CURRENT TRANSFORMER");
        pointerLabel(ui, vp, Vec3(111.0f, 10.5f, BAY_Z[0]), -120, 90, "SURGE ARRESTER");
        pointerLabel(ui, vp, Vec3(LINE_GANTRY_X, 20.0f, 56.5f), 80, -60, "LINE GANTRY");
    }
    // summary of the whole energy chain once the flight is over
    if (closedAt >= 0) {
        float flightEnd = FLIGHT_DELAY + (world.line.lineLength - 90.0f) / FLIGHT_SPEED;
        float a = clampf((t - closedAt - flightEnd + 1.0f) / 1.5f, 0.0f, 1.0f);
        if (a > 0.0f) drawSummary(ui, a);
    }
}

// The energy chain from the reservoir to the grid, with the plant's numbers.
void Scene11_Grid::drawSummary(Ui& ui, float alpha) {
    const Vec4 cyan(0.45f, 0.88f, 1.0f, alpha), white(1, 1, 1, alpha), dim(0.7f, 0.85f, 0.95f, alpha);
    float W = ui.W, H = ui.H;
    float pw = W - 160, ph = 300, px = 80, py = H * 0.5f - 110;
    ui.rect(px, py, pw, ph, Vec4(0.01f, 0.03f, 0.06f, 0.82f * alpha));
    ui.corners(px, py, pw, ph, 24, 3, cyan);
    const char* head = "THE ENERGY CHAIN OF THE HYDRO POWER PLANT";
    float hs = ui.textWidth(head, 3) < pw - 40 ? 3.0f : 2.0f;
    ui.text(px + (pw - ui.textWidth(head, hs)) * 0.5f, py + 16, head, hs, white);

    struct Box { const char* title; const char* l1; const char* l2; const char* l3; Vec4 col; };
    char p1[48], p2[48], p3[48];
    std::snprintf(p1, sizeof(p1), "V = %.0f M/S", Plant::freeFallSpeed(Plant::HEAD));
    std::snprintf(p2, sizeof(p2), "%.1f MN.M", Plant::torqueMNm(Plant::RATED_MW, Plant::RPM));
    std::snprintf(p3, sizeof(p3), "P = %.0f MW", Plant::powerMW(Plant::Q_UNIT, Plant::HEAD));
    Box boxes[5] = {
        {"RESERVOIR", "POTENTIAL ENERGY", "E = M G H", "HEAD 53 M", Vec4(0.3f, 0.6f, 1.0f, alpha)},
        {"PENSTOCK", "KINETIC ENERGY", p1, "Q = 180 M3/S", Vec4(0.3f, 0.75f, 1.0f, alpha)},
        {"TURBINE + SHAFT", "MECHANICAL ENERGY", "150 RPM", p2, Vec4(0.75f, 0.8f, 0.85f, alpha)},
        {"GENERATOR", "ELECTRICAL ENERGY", p3, "11 KV  50 HZ", Vec4(1.0f, 0.6f, 0.2f, alpha)},
        {"TRANSFORMER", "TO THE GRID", "11 -> 220 KV", "HOMES + INDUSTRY", Vec4(0.5f, 0.8f, 1.0f, alpha)},
    };
    float bw = (pw - 60 - 4 * 40) / 5.0f, bh = 150, by = py + 70;
    for (int i = 0; i < 5; i++) {
        float bx = px + 30 + i * (bw + 40);
        float appear = clampf(alpha * 5.0f - i * 0.8f, 0.0f, 1.0f);   // boxes appear one after the other
        if (appear <= 0) continue;
        Vec4 c = boxes[i].col;
        c.w *= appear;
        ui.rect(bx, by, bw, bh, Vec4(c.x * 0.15f, c.y * 0.15f, c.z * 0.15f, 0.8f * appear));
        ui.outline(bx, by, bw, bh, 2, c);
        ui.rect(bx, by, bw, 32, Vec4(c.x * 0.4f, c.y * 0.4f, c.z * 0.4f, 0.9f * appear));
        float s = ui.textWidth(boxes[i].title, 2.0f) < bw - 16 ? 2.0f : 1.5f;
        ui.text(bx + 10, by + 9, boxes[i].title, s, white);
        ui.text(bx + 10, by + 46, boxes[i].l1, 1.5f, c);
        ui.text(bx + 10, by + 78, boxes[i].l2, 2.0f, white);
        ui.text(bx + 10, by + 112, boxes[i].l3, 1.5f, dim);
        if (i < 4) {                                   // arrow to the next box
            float ax = bx + bw + 6, ay = by + bh * 0.5f;
            ui.line(ax, ay, ax + 28, ay, 3, cyan);
            ui.line(ax + 28, ay, ax + 18, ay - 8, 3, cyan);
            ui.line(ax + 28, ay, ax + 18, ay + 8, 3, cyan);
        }
    }
    const char* foot = "OVERALL EFFICIENCY 92 %  -  NO FUEL BURNED  -  ONLY GRAVITY AND WATER";
    float fs = ui.textWidth(foot, 2) < pw - 40 ? 2.0f : 1.5f;
    ui.text(px + (pw - ui.textWidth(foot, fs)) * 0.5f, py + ph - 44, foot, fs, cyan);
}

void Scene11_Grid::telemetry(std::vector<Telemetry>& out) {
    char buf[64];
    out.push_back({"CIRCUIT BREAKER", world.breaker01 > 0.5f ? "CLOSED" : "OPEN"});
    // the line is only live when the breaker connects it to the transformer
    std::snprintf(buf, sizeof(buf), "%.0f KV", world.voltageKV() * Plant::GRID_KV / Plant::GEN_KV * world.breaker01);
    out.push_back({"LINE VOLTAGE", buf});
    std::snprintf(buf, sizeof(buf), "%.1f HZ", world.frequency());
    out.push_back({"FREQUENCY", buf});
    std::snprintf(buf, sizeof(buf), "%.0f MW", world.powerMW());
    out.push_back({"POWER TO THE GRID", buf});
    std::snprintf(buf, sizeof(buf), "%.2f KM", std::fmin(world.gridFront, world.line.lineLength) / 1000.0f);
    out.push_back({"LINE ENERGISED", buf});
}

void Scene11_Grid::buttons(std::vector<Button>& out) {
    out.push_back({world.breakerClosed ? "OPEN BREAKER" : "CLOSE BREAKER", 1, "O", !world.breakerClosed});
    out.push_back({"REPLAY JOURNEY", 2, "ENTER", world.breakerClosed});
}

void Scene11_Grid::onButton(int id) {
    if (id == 1) world.breakerClosed = !world.breakerClosed;
    if (id == 2) replay = true;
}

void Scene11_Grid::autoPlay() {
    if (t > 5.0f && !world.breakerClosed && closedAt < 0) world.breakerClosed = true;
    if (closedAt >= 0 && t - closedAt > 32.0f) replay = true;
}
