#include "Scene1_Spillway.h"
#include <cstdio>

void Scene1_Spillway::enter() {
    t = 0;
    next = false;
    site.reset(true);          // water is already pouring when the film starts
}

void Scene1_Spillway::update(float dt) {
    t += dt;
    site.update(dt);
}

// Crane shot: start high above the spillway, follow the falling water down
// and swing round to look into the cut-away powerhouse (like the video).
void Scene1_Spillway::camera(Camera& cam) {
    static const CamKey keys[] = {
        {0.0f,  Vec3(-12, 96, 128), Vec3(8, 48, 12)},
        {3.5f,  Vec3(14, 52, 112),  Vec3(18, 22, 42)},
        {6.5f,  Vec3(22, 20, 78),   Vec3(52, 10, 60)},
        {10.0f, Vec3(25, 13.5f, 63), Vec3(58, 10, 61)},
    };
    evalCamPath(keys, 4, t, cam.eye, cam.target);
    cam.fovDeg = 50.0f;
}

void Scene1_Spillway::lighting(Lighting& L) {
    // dramatic overcast evening, sun low on the left
    L.sunDir = normalize(Vec3(-0.6f, 0.62f, 0.55f));
    L.sunColor = Vec3(2.3f, 2.0f, 1.65f);
    L.skyTop = Vec3(0.07f, 0.1f, 0.16f);
    L.skyHorizon = Vec3(0.32f, 0.33f, 0.36f);
    L.ground = Vec3(0.1f, 0.1f, 0.1f);
    L.ambient = 0.55f;
    L.fogColor = Vec3(0.22f, 0.24f, 0.27f);
    L.fogDensity = 0.0024f;
    L.exposure = 0.56f;
    L.numPoint = 2;
    L.pointPos[0] = Vec3(55, 21, 56);
    L.pointColor[0] = Vec3(14, 12, 9);
    L.pointPos[1] = Vec3(55, 21, 67);
    L.pointColor[1] = Vec3(14, 12, 9);
}

void Scene1_Spillway::telemetry(std::vector<Telemetry>& out) {
    char buf[64];
    std::snprintf(buf, sizeof(buf), "%d %%", (int)(site.gateOpen * 100.0f + 0.5f));
    out.push_back({"SPILLWAY GATES", buf});
    float q = site.spill.reachesBottom() ? 2350.0f * site.gateOpen : 0.0f;
    std::snprintf(buf, sizeof(buf), "%.0f M3/S", q);
    out.push_back({"SPILL FLOW", buf});
    std::snprintf(buf, sizeof(buf), "%.1f M/S", site.spill.reachesBottom() ? site.spill.speedAt(site.spill.length - 1.0f) : 0.0f);
    out.push_back({"CHUTE EXIT SPEED", buf});
    std::snprintf(buf, sizeof(buf), "%.0f RPM", site.rotorSpeed / 2.4f * 150.0f);
    out.push_back({"TURBINE", buf});
    std::snprintf(buf, sizeof(buf), "%.1f MW", site.rotorSpeed / 2.4f * 86.0f);
    out.push_back({"GENERATOR", buf});
}

void Scene1_Spillway::buttons(std::vector<Button>& out) {
    out.push_back({site.gatesOpen ? "STOP WATER" : "START WATER", BTN_TOGGLE, "SPACE", false});
    out.push_back({"FOLLOW WATER TO INTAKE >", BTN_NEXT, "ENTER", true});
}

void Scene1_Spillway::onButton(int id) {
    if (id == BTN_TOGGLE) site.setWaterRunning(!site.gatesOpen);
    if (id == BTN_NEXT) next = true;
}
