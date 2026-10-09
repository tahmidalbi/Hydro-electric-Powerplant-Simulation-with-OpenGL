#include "Scene9_Generator.h"
#include <cstdio>

using namespace GenDims;

void Scene9_Generator::enter(bool continuous) {
    t = 0;
    next = false;
    if (!continuous) {
        world.completeUpTo(ST_GENERATOR);
        fromEye = Vec3(57.7f, 16.6f, 59.3f);
        fromTarget = Vec3(62.5f, 16.6f, 64.0f);
    }
    world.turbineSection = false;
}

void Scene9_Generator::camera(Camera& cam) {
    CamKey keys[] = {
        {0.0f, fromEye, fromTarget},
        {3.5f, Vec3(51.0f, 18.0f, 62.5f), Vec3(62.5f, 16.6f, 64.0f)},     // the whole cut-away
        {12.0f, Vec3(53.0f, 17.4f, 61.5f), Vec3(62.5f, 16.6f, 64.0f)},
        {22.0f, Vec3(55.6f, 17.0f, 61.6f), Vec3(62.0f, 16.6f, 64.6f)},     // close to the rotor
    };
    evalCamPath(keys, 4, t, cam.eye, cam.target);
    cam.fovDeg = 52.0f;
    cam.nearZ = 0.1f;
}

void Scene9_Generator::hud(Ui& ui, const Mat4& vp) {
    if (t > 3.0f) {
        pointerLabel(ui, vp, Vec3(BASE.x - 3.0f, 19.3f, BASE.z - 2.6f), 160, 90, "STATOR: COPPER COILS IN 72 SLOTS");
        pointerLabel(ui, vp, Vec3(BASE.x - 3.4f, 15.2f, BASE.z - 1.0f), 160, -30, "ROTOR: 40 POLES (ELECTROMAGNETS)");
        if (world.excitation > 0.3f) pointerLabel(ui, vp, Vec3(BASE.x - 3.85f, 16.7f, BASE.z + 0.6f), 100, -80, "MAGNETIC FIELD");
        if (world.voltage01() > 0.3f) pointerLabel(ui, vp, Vec3(BASE.x - 3.2f, CORE_Y1 + 0.4f, BASE.z + 2.4f), 60, 150, "INDUCED THREE-PHASE CURRENT");
    }
    // oscilloscope: the three phases, 120 degrees apart
    float px = 24, py = ui.H - 250.0f, pw = 360, ph = 110;
    ui.rect(px, py, pw, ph, Vec4(0.02f, 0.05f, 0.08f, 0.7f));
    ui.corners(px, py, pw, ph, 10, 2, Vec4(0.45f, 0.88f, 1.0f, 0.9f));
    char hdr[64];
    std::snprintf(hdr, sizeof(hdr), "3-PHASE OUTPUT  %.1f KV  %.1f HZ", world.voltageKV(), world.frequency());
    ui.text(px + 10, py + 8, hdr, 1.5f, Vec4(0.8f, 0.92f, 1.0f, 1.0f));
    ui.rect(px + 10, py + 30 + 35, pw - 20, 1, Vec4(0.5f, 0.6f, 0.7f, 0.5f));
    const Vec4 col[3] = {Vec4(1.0f, 0.35f, 0.25f, 1), Vec4(1.0f, 0.85f, 0.2f, 1), Vec4(0.35f, 0.65f, 1.0f, 1)};
    float amp = 30.0f * world.voltage01();
    float cycles = 3.0f, speed = world.frequency() / Plant::FREQ * 1.2f;   // shown slowed down
    for (int p = 0; p < 3; p++) {
        float prevX = 0, prevY = 0;
        for (int i = 0; i <= 120; i++) {
            float u = i / 120.0f;
            float x = px + 10 + u * (pw - 20);
            float y = py + 65 - amp * std::sin(2 * PI * (cycles * u - t * speed) - p * 2 * PI / 3);
            if (i > 0) ui.line(prevX, prevY, x, y, 2, col[p]);
            prevX = x;
            prevY = y;
        }
    }
}

void Scene9_Generator::telemetry(std::vector<Telemetry>& out) {
    char buf[64];
    std::snprintf(buf, sizeof(buf), "%.0f RPM", world.rpm());
    out.push_back({"ROTOR SPEED", buf});
    std::snprintf(buf, sizeof(buf), "%d %%", (int)(world.excitation * 100 + 0.5f));
    out.push_back({"FIELD CURRENT", buf});
    std::snprintf(buf, sizeof(buf), "%.1f HZ", world.frequency());
    out.push_back({"F = POLES X RPM / 120", buf});
    std::snprintf(buf, sizeof(buf), "%.1f KV", world.voltageKV());
    out.push_back({"TERMINAL VOLTAGE", buf});
    out.push_back({"RATED OUTPUT", "86 MW"});
}

void Scene9_Generator::buttons(std::vector<Button>& out) {
    out.push_back({world.excited ? "SWITCH FIELD OFF" : "EXCITE ROTOR", 1, "O", !world.excited});
    Button follow{"FOLLOW THE POWER >", 2, "ENTER", true};
    follow.enabled = ready();
    follow.hint = world.excited ? "(WAIT - VOLTAGE IS RISING)" : "(EXCITE THE ROTOR FIRST)";
    out.push_back(follow);
}

void Scene9_Generator::onButton(int id) {
    if (id == 1) world.excited = !world.excited;
    if (id == 2 && ready()) next = true;
}

void Scene9_Generator::autoPlay() {
    if (t > 4.0f && !world.excited) world.excited = true;
    if (ready() && t > 14.0f) next = true;
}
