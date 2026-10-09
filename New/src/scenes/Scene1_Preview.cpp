#include "Scene1_Preview.h"
#include <cstdio>

void Scene1_Preview::enter(bool) {
    t = 0;
    next = false;
    world.setPreview();        // the plant is already running when the film starts
}

void Scene1_Preview::update(float dt) {
    t += dt;
    world.update(dt);
}

// Crane shot: start high above the spillway, follow the falling water down
// and swing round to look into the cut-away powerhouse (like the video).
void Scene1_Preview::camera(Camera& cam) {
    static const CamKey keys[] = {
        {0.0f,  Vec3(-12, 96, 128), Vec3(8, 48, 12)},
        {3.5f,  Vec3(14, 52, 112),  Vec3(18, 22, 42)},
        {6.5f,  Vec3(22, 20, 78),   Vec3(52, 10, 60)},
        {10.0f, Vec3(25, 13.5f, 63), Vec3(58, 10, 61)},
    };
    evalCamPath(keys, 4, t, cam.eye, cam.target);
    cam.fovDeg = 50.0f;
}

void Scene1_Preview::telemetry(std::vector<Telemetry>& out) {
    char buf[64];
    std::snprintf(buf, sizeof(buf), "%d %%", (int)(world.spillGate * 100.0f + 0.5f));
    out.push_back({"SPILLWAY GATES", buf});
    float q = world.spill.reachesBottom() ? 2350.0f * world.spillGate : 0.0f;
    std::snprintf(buf, sizeof(buf), "%.0f M3/S", q);
    out.push_back({"SPILL FLOW", buf});
    std::snprintf(buf, sizeof(buf), "%.1f M/S", world.spill.reachesBottom() ? world.spill.speedAt(world.spill.length - 1.0f) : 0.0f);
    out.push_back({"CHUTE EXIT SPEED", buf});
    std::snprintf(buf, sizeof(buf), "%.0f RPM", world.rotorSpeed / 2.4f * 150.0f);
    out.push_back({"TURBINE", buf});
    std::snprintf(buf, sizeof(buf), "%.1f MW", world.rotorSpeed / 2.4f * 86.0f);
    out.push_back({"GENERATOR", buf});
}

void Scene1_Preview::buttons(std::vector<Button>& out) {
    out.push_back({world.spillOpen ? "STOP WATER" : "START WATER", BTN_TOGGLE, "SPACE", false});
    out.push_back({"SHOW WATER MOVEMENT >", BTN_SHOW, "ENTER", true});
}

void Scene1_Preview::onButton(int id) {
    if (id == BTN_TOGGLE) {
        // stop / start ALL falling water: spillway gates and the intake gate
        bool open = !world.spillOpen;
        world.setSpillway(open);
        world.setIntakeGate(open);
    }
    if (id == BTN_SHOW) next = true;
}
