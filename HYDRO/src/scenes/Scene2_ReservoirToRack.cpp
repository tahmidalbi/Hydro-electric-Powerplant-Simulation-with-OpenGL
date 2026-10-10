#include "Scene2_ReservoirToRack.h"
#include <cstdio>

using namespace DamDims;

void Scene2_ReservoirToRack::enter(bool continuous) {
    t = 0;
    next = false;
    if (!continuous) {                  // jumped here: start from a view above the dam
        world.setPreview();
        fromEye = Vec3(30, 90, 80);
        fromTarget = Vec3(58, 40, 0);
    }
    world.beginJourney();               // intake gate closes, waterway drains
}

void Scene2_ReservoirToRack::update(float dt) {
    bool wasRack = inRack();
    t += dt;
    if (!wasRack && inRack()) {
        world.prepareRack(false);       // cut away the water and near pier, start the trash
        dissolveRequest = true;         // cross-fade from under water to the cut-away view
    }
    world.update(dt);
}

void Scene2_ReservoirToRack::camera(Camera& cam) {
    if (!inRack()) {
        // continuous flight: from wherever the preview camera was, over the crest,
        // over the reservoir, and down through the surface to the rack
        CamKey keys[] = {
            {0.0f, fromEye, fromTarget},
            {2.0f, mix(fromEye, Vec3(20, 70, 110), 0.7f), Vec3(45, 45, 30)},   // pull back and up first
            {4.5f, Vec3(42, 90, 18), Vec3(58, 56, -25)},                       // over the crest
            {7.0f, Vec3(58, 66, -64), Vec3(58, 50, -25)},                      // above the reservoir
            {9.5f, Vec3(58, 52.5f, -58), Vec3(58, 46, -22)},                   // through the surface
            {APPROACH, Vec3(58, 44, -37), Vec3(58, 44, -15)},                  // in front of the rack
        };
        evalCamPath(keys, 6, t, cam.eye, cam.target);
        cam.fovDeg = 52.0f;
    } else {
        // cut-away side view of the cage (the water flows from left to right)
        static const CamKey keys[] = {
            {0.0f,  Vec3(30, 58.5f, -14), Vec3(58, 51, -11)},
            {16.0f, Vec3(31.5f, 58, -11), Vec3(58, 51, -9)},
        };
        evalCamPath(keys, 2, t - APPROACH, cam.eye, cam.target);
        cam.fovDeg = 50.0f;
    }
    cam.nearZ = 0.1f;
    camEye = cam.eye;
}

void Scene2_ReservoirToRack::hud(Ui& ui, const Mat4& vp) {
    if (!inRack()) return;
    float tb = t - APPROACH;
    if (tb > 1.0f) pointerLabel(ui, vp, Vec3(IntakeDims::CAGE_X0, 59.0f, -9.0f), 60, -40, "TRASH RACK - STEEL BARS");
    if (tb > 2.0f) pointerLabel(ui, vp, Vec3(INTAKE_X - 2.6f, 64.2f, -14.5f), 70, -30, "RAKE MOTORS");
    if (tb > 5.0f && world.debris.stuckCount() > 0) pointerLabel(ui, vp, Vec3(INTAKE_X, 54.0f, -19.5f), -110, 70, "TRASH TRAPPED");
    if (tb > 6.5f) pointerLabel(ui, vp, Vec3(CONDUIT_X0 + 0.1f, 39.0f, -1.0f), 60, 70, "CLEAN WATER INTO THE INTAKE");
}

void Scene2_ReservoirToRack::telemetry(std::vector<Telemetry>& out) {
    char buf[64];
    if (!inRack()) {
        float depth = RESERVOIR_Y - camEye.y;
        std::snprintf(buf, sizeof(buf), "%.1f M", depth > 0 ? depth : 0.0f);
        out.push_back({"DEPTH", buf});
        std::snprintf(buf, sizeof(buf), "%.2f BAR", 1.0f + (depth > 0 ? depth : 0.0f) / 10.2f);
        out.push_back({"PRESSURE", buf});
        std::snprintf(buf, sizeof(buf), "%d %%", (int)(world.intakeLift / world.gate.maxLift * 100.0f + 0.5f));
        out.push_back({"INTAKE GATE (CLOSING)", buf});
        std::snprintf(buf, sizeof(buf), "%.0f RPM", world.rotorSpeed / 2.4f * 150.0f);
        out.push_back({"TURBINE (RUNNING DOWN)", buf});
        return;
    }
    std::snprintf(buf, sizeof(buf), "%d", world.debris.stuckCount());
    out.push_back({"PIECES TRAPPED", buf});
    std::snprintf(buf, sizeof(buf), "%d", world.debris.passedCount());
    out.push_back({"SMALL LEAVES PASSED", buf});
    std::snprintf(buf, sizeof(buf), "%.2f M", 0.04f + world.debris.stuckCount() * 0.012f);
    out.push_back({"HEAD LOSS AT RACK", buf});
    out.push_back({"APPROACH FLOW", "2.4 M/S"});
    out.push_back({"BAR GAP", "0.56 M"});
}

void Scene2_ReservoirToRack::buttons(std::vector<Button>& out) {
    if (!inRack()) return;                 // the flight runs by itself
    out.push_back({"ADD MORE TRASH", 1, "T", false});
    out.push_back({"FOLLOW TO INTAKE GATE >", 2, "ENTER", true});
}

void Scene2_ReservoirToRack::onButton(int id) {
    if (!inRack()) return;
    if (id == 1) world.addTrash();
    if (id == 2) next = true;
}
