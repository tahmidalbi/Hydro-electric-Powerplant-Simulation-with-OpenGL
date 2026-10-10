#include "Scene10_Transformer.h"
#include <cstdio>

using namespace TrafoDims;

void Scene10_Transformer::enter(bool continuous) {
    t = 0;
    next = false;
    if (!continuous) {
        world.completeUpTo(ST_TRANSFORMER);
        fromEye = Vec3(55.6f, 17.0f, 61.6f);
        fromTarget = Vec3(62.0f, 16.6f, 64.6f);
    }
    world.trafoXrayOn = false;
}

// out of the hall (past the open roof edge), over the roof, down into the yard
void Scene10_Transformer::camera(Camera& cam) {
    CamKey keys[] = {
        {0.0f,  fromEye, fromTarget},
        {2.5f,  Vec3(49.0f, 31.0f, 62.0f), Vec3(64.0f, 20.0f, 64.0f)},
        {5.0f,  Vec3(63.0f, 37.0f, 56.0f), Vec3(77.0f, 15.0f, 62.0f)},
        {7.5f,  Vec3(77.5f, 24.0f, 47.0f), Vec3(82.0f, 11.0f, 64.0f)},
        {10.0f, Vec3(75.5f, 15.0f, 48.5f), Vec3(82.5f, 10.5f, 64.0f)},
        {24.0f, Vec3(76.5f, 14.0f, 50.0f), Vec3(83.0f, 10.5f, 64.0f)},
    };
    evalCamPath(keys, 6, t, cam.eye, cam.target);
    cam.fovDeg = 54.0f;
}

void Scene10_Transformer::hud(Ui& ui, const Mat4& vp) {
    if (t < 9.0f) return;
    pointerLabel(ui, vp, Vec3(X0 + 0.8f, 14.6f, phaseZ(2)), 150, 110,"3 BUS DUCTS - 11 KV");   // where they drop onto the tank
    pointerLabel(ui, vp, Vec3(X0, 9.0f, Z0 + 2.0f), -90, 80, "STEP-UP TRANSFORMER");
    pointerLabel(ui, vp, Vec3(85.6f, TOP_Y + 3.0f, phaseZ(0)), 90, -60, "HV BUSHINGS - 220 KV");
    pointerLabel(ui, vp, Vec3(X0 + 1.8f, BASE_Y + 1.6f, Z0 - 1.6f), -60, -150, "COOLING FANS + RADIATORS");
    if (world.trafoXray > 0.5f) {
        pointerLabel(ui, vp, Vec3(83.0f, BASE_Y + 3.0f, phaseZ(1) - 0.6f), 110, 40, "LV WINDING (FEW TURNS)");
        pointerLabel(ui, vp, Vec3(83.0f, BASE_Y + 4.5f, phaseZ(2) - 1.0f), 110, -10, "HV WINDING (MANY TURNS)");
    }
}

void Scene10_Transformer::telemetry(std::vector<Telemetry>& out) {
    char buf[64];
    std::snprintf(buf, sizeof(buf), "%.1f KV", world.voltageKV());
    out.push_back({"LOW VOLTAGE SIDE", buf});
    std::snprintf(buf, sizeof(buf), "%.0f KV", world.voltageKV() * Plant::GRID_KV / Plant::GEN_KV);
    out.push_back({"HIGH VOLTAGE SIDE", buf});
    out.push_back({"TURNS RATIO", "1 : 20"});
    out.push_back({"CURRENT (86 MW)", "4.5 > 0.23 KA"});
    out.push_back({"COOLING", world.voltage01() > 0.5f ? "FANS RUNNING" : "OFF"});
}

void Scene10_Transformer::buttons(std::vector<Button>& out) {
    out.push_back({world.trafoXrayOn ? "X-RAY OFF" : "X-RAY TRANSFORMER", 1, "X", false});
    out.push_back({"FOLLOW TO SWITCHYARD >", 2, "ENTER", true});
}

void Scene10_Transformer::onButton(int id) {
    if (id == 1) world.trafoXrayOn = !world.trafoXrayOn;
    if (id == 2) next = true;
}

void Scene10_Transformer::autoPlay() {
    if (t > 11.0f && t < 16.0f) world.trafoXrayOn = true;
    if (t > 16.0f) world.trafoXrayOn = false;
    if (t > 19.0f) next = true;
}
