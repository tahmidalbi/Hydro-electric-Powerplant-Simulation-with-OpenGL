#include "Scene2_DamSurvey.h"

using namespace DamDims;

void Scene2_DamSurvey::enter() {
    t = 0;
    skip = false;
    site.reset(false);          // gates closed, chutes dry (as in the video)
}

void Scene2_DamSurvey::update(float dt) {
    t += dt;
    site.update(dt);
}

void Scene2_DamSurvey::camera(Camera& cam) {
    static const CamKey keys[] = {
        {0.0f,  Vec3(-100, 88, 175), Vec3(-12, 30, 8)},
        {10.5f, Vec3(-80, 76, 140), Vec3(-12, 30, 8)},
    };
    evalCamPath(keys, 2, t, cam.eye, cam.target);
    cam.fovDeg = 46.0f;
}

void Scene2_DamSurvey::lighting(Lighting& L) {
    // bright, slightly hazy day
    L.sunDir = normalize(Vec3(0.65f, 0.6f, 0.4f));
    L.sunColor = Vec3(2.5f, 2.4f, 2.2f);
    L.skyTop = Vec3(0.2f, 0.36f, 0.62f);
    L.skyHorizon = Vec3(0.55f, 0.63f, 0.72f);
    L.ground = Vec3(0.2f, 0.19f, 0.18f);
    L.ambient = 0.55f;
    L.fogColor = Vec3(0.55f, 0.62f, 0.7f);
    L.fogDensity = 0.0011f;
    L.exposure = 0.44f;
}

// Engineering overlay: lines that grow in one after the other, with labels.
void Scene2_DamSurvey::hud(Ui& ui, const Mat4& vp) {
    const Vec4 cyan(0.55f, 0.9f, 1.0f, 0.95f), white(1, 1, 1, 0.95f), dim(0.55f, 0.9f, 1.0f, 0.35f);
    // thin frame like a camera viewfinder
    float m = 40;
    ui.outline(m, m + 30, ui.W - 2 * m, ui.H - 2 * m - 110, 1, dim);
    for (int i = 0; i <= 20; i++) {
        float x = m + (ui.W - 2 * m) * i / 20.0f;
        ui.rect(x, m + 30, 1, i % 5 == 0 ? 10 : 5, dim);
    }

    auto P = [&](const Vec3& p, float& x, float& y) { return projectToScreen(vp, p, ui.W, ui.H, x, y); };
    auto grow = [&](float start) { return clampf((t - start) / 1.2f, 0.0f, 1.0f); };

    // 1) dam height (vertical line at the left end of the spillway)
    struct Dim { Vec3 a, b; const char* label; float start; };
    Dim dims[] = {
        {Vec3(-70, 0.5f, 10), Vec3(-70, CREST_Y, 10), "DAM HEIGHT 63 M", 1.0f},
        {Vec3(-90, RIVER_Y, 9), Vec3(-90, RESERVOIR_Y, 9), "HEAD 53 M", 2.6f},
        {Vec3(-140, CREST_Y + 4, 4), Vec3(140, CREST_Y + 4, 4), "CREST LENGTH 300 M", 4.2f},
        {Vec3(SPILL_X0, 44, 30), Vec3(SPILL_X1, 44, 30), "SPILLWAY 3 GATES X 11 M", 5.8f},
    };
    for (const Dim& d : dims) {
        float k = grow(d.start);
        if (k <= 0) continue;
        float x0, y0, x1, y1;
        if (!P(d.a, x0, y0) || !P(mix(d.a, d.b, k), x1, y1)) continue;
        ui.line(x0, y0, x1, y1, 2, cyan);
        // end ticks
        float dx = x1 - x0, dy = y1 - y0, len = std::sqrt(dx * dx + dy * dy) + 1e-3f;
        float nx = -dy / len * 8, ny = dx / len * 8;
        ui.line(x0 - nx, y0 - ny, x0 + nx, y0 + ny, 2, cyan);
        if (k >= 1) ui.line(x1 - nx, y1 - ny, x1 + nx, y1 + ny, 2, cyan);
        if (k > 0.6f) {
            float lx = (x0 + x1) * 0.5f + 14, ly = (y0 + y1) * 0.5f - 10;
            float w = ui.textWidth(d.label, 2) + 12;
            ui.rect(lx - 6, ly - 5, w, 26, Vec4(0.0f, 0.05f, 0.08f, 0.55f));
            ui.text(lx, ly, d.label, 2, white);
        }
    }
    // reservoir level marker
    if (t > 7.4f) {
        float x, y;
        if (P(Vec3(-60, RESERVOIR_Y, -40), x, y)) {
            ui.line(x - 70, y, x + 70, y, 2, cyan);
            ui.text(x - 70, y - 22, "RESERVOIR +56.0 M", 2, white);
        }
    }
}

void Scene2_DamSurvey::telemetry(std::vector<Telemetry>& out) {
    out.push_back({"RESERVOIR LEVEL", "+56.0 M"});
    out.push_back({"TAILWATER LEVEL", "+3.0 M"});
    out.push_back({"GROSS HEAD", "53.0 M"});
    out.push_back({"SPILLWAY GATES", "CLOSED"});
    out.push_back({"POTENTIAL ENERGY", "E = M G H"});
}

void Scene2_DamSurvey::buttons(std::vector<Button>& out) {
    out.push_back({"DIVE TO THE INTAKE >", 1, "ENTER", true});
}
