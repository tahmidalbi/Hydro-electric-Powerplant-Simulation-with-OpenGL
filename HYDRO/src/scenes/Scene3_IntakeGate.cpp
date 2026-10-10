#include "Scene3_IntakeGate.h"
#include <cstdio>

using namespace DamDims;

void Scene3_IntakeGate::enter(bool continuous) {
    t = 0;
    next = false;
    openedAt = -1.0f;
    if (!continuous) {
        fromEye = Vec3(31.5f, 58, -11);
        fromTarget = Vec3(58, 51, -9);
    }
    world.prepareGate(!continuous);     // the dam cut block lifts away
}

void Scene3_IntakeGate::update(float dt) {
    t += dt;
    world.update(dt);
}

// glide sideways (downstream) from the rack to the gate
void Scene3_IntakeGate::camera(Camera& cam) {
    CamKey keys[] = {
        {0.0f, fromEye, fromTarget},
        {4.0f, Vec3(37, 50, 21), Vec3(57, 39.5f, 3)},     // downstream of the gate, looking back at it
        {20.0f, Vec3(39, 48.5f, 23), Vec3(57, 39.5f, 4)},
    };
    evalCamPath(keys, 3, t, cam.eye, cam.target);
    cam.fovDeg = 52.0f;
}

void Scene3_IntakeGate::hud(Ui& ui, const Mat4& vp) {
    if (t < 3.0f) return;
    pointerLabel(ui, vp, Vec3(INTAKE_X - 3.0f, CONDUIT_Y0 + 4.0f + world.intakeLift, 2.5f), -110, 60, "STEEL INTAKE GATE");
    pointerLabel(ui, vp, Vec3(CONDUIT_X0, CONDUIT_Y0 + 9.0f, 8.0f), 70, -70, "CONDUIT THROUGH THE DAM");
    pointerLabel(ui, vp, Vec3(56.0f, 39.0f, 14.5f), 60, 80, "PENSTOCK ENTRANCE");
}

void Scene3_IntakeGate::telemetry(std::vector<Telemetry>& out) {
    char buf[64];
    std::snprintf(buf, sizeof(buf), "%.1f M (%d %%)", world.intakeLift, (int)(world.intakeLift / world.gate.maxLift * 100 + 0.5f));
    out.push_back({"GATE OPENING", buf});
    float v = std::sqrt(2.0f * 9.81f * (RESERVOIR_Y - 39.0f));
    std::snprintf(buf, sizeof(buf), "%.1f M/S", world.intakeLift > 0.05f ? v : 0.0f);
    out.push_back({"JET  V=SQRT(2GH)", buf});
    std::snprintf(buf, sizeof(buf), "%.0f M3/S", world.intakeLift > 0.05f ? 11.0f * std::fmin(world.intakeLift, 4.0f) * 0.6f * v : 0.0f);
    out.push_back({"DISCHARGE", buf});
    std::snprintf(buf, sizeof(buf), "%d %%", (int)(clampf(world.conduitFront / world.conduitLength(), 0, 1) * clampf(world.conduitLevel / 9.3f, 0, 1) * 100));
    out.push_back({"CONDUIT FILLED", buf});
    bool moving = world.intakeLift > 0.01f && world.intakeLift < world.gate.maxLift - 0.01f;
    out.push_back({"HYDRAULIC PRESSURE", moving ? "180 BAR" : "HOLD"});
}

void Scene3_IntakeGate::buttons(std::vector<Button>& out) {
    out.push_back({world.intakeOpen ? "CLOSE GATE" : "OPEN GATE", 1, "O", !world.intakeOpen});
    Button follow{"FOLLOW TO PENSTOCK >", 2, "ENTER", true};
    follow.enabled = world.gateFullyOpen();       // only once the gate is fully open
    follow.hint = world.intakeOpen ? "(WAIT - THE GATE IS RISING)" : "(OPEN THE GATE FIRST)";
    out.push_back(follow);
}

void Scene3_IntakeGate::onButton(int id) {
    if (id == 1) {
        world.setIntakeGate(!world.intakeOpen);
        if (world.intakeOpen && openedAt < 0) openedAt = t;
    }
    if (id == 2 && world.gateFullyOpen()) next = true;
}

void Scene3_IntakeGate::autoPlay() {
    if (t > 4.5f && !world.intakeOpen) onButton(1);
    if (world.gateFullyOpen() && world.conduitFull() && t > 12.0f) onButton(2);
}
