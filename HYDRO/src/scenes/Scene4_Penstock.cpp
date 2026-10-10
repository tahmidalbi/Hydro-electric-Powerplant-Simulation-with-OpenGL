#include "Scene4_Penstock.h"
#include <cstdio>

using namespace DamDims;

static const Vec3 CLOSE_EYE(50.0f, 7.4f, 50.5f), CLOSE_TARGET(58.0f, 6.0f, 56.0f);   // close to the pipe in the turbine pit

void Scene4_Penstock::enter(bool continuous) {
    t = 0;
    xrayManual = false;
    arrivedTime = 0;
    leaving = false;
    leaveT = 0;
    world.xrayOn = false;
    if (!continuous) {
        fromEye = Vec3(39, 48.5f, 23);
        fromTarget = Vec3(57, 39.5f, 4);
    }
    world.preparePenstock(!continuous);   // the water may now enter the penstock
    lastEye = fromEye;
    lastTarget = fromTarget;
}

void Scene4_Penstock::update(float dt) {
    t += dt;
    if (leaving) leaveT += dt;
    // the hologram x-ray look switches on when the water is half way down
    if (!xrayManual) world.xrayOn = world.pipeFront > world.pipeLength() * 0.5f;
    if (world.pipeFront >= world.pipeLength() - 1.0f) arrivedTime += dt;
    world.update(dt);
}

// Chase camera: rides beside the water front (on the cut-away side), then
// pulls back to show the turbine in the powerhouse.
void Scene4_Penstock::camera(Camera& cam) {
    float L = world.pipeLength();
    float s = clampf(world.pipeFront, 0.0f, L - 2.0f);
    Vec3 f = world.pipePoint(s);
    Vec3 chaseEye = f + Vec3(-17.0f, 7.0f, -3.0f);
    Vec3 chaseTarget = f + Vec3(0.0f, -1.0f, 5.0f);
    float k = clampf(world.pipeFront / L, 0.0f, 1.0f);
    float w = smoothstepf(0.72f, 0.98f, k);
    Vec3 eye = mix(chaseEye, Vec3(30, 16, 44), w);
    Vec3 target = mix(chaseTarget, Vec3(58, 7, 57), w);
    // first seconds: glide from the gate view to the chase position
    float a = smoothstepf(0.0f, 3.0f, t);
    eye = mix(fromEye, eye, a);
    target = mix(fromTarget, target, a);
    if (leaving) {                          // move in close to the pipe before the ray-traced view
        float b = smoothstepf(0.0f, 1.8f, leaveT);
        eye = mix(lastEye, CLOSE_EYE, b);
        target = mix(lastTarget, CLOSE_TARGET, b);
    } else {
        lastEye = eye;
        lastTarget = target;
    }
    cam.eye = eye;
    cam.target = target;
    cam.fovDeg = 52.0f;
}

void Scene4_Penstock::hud(Ui& ui, const Mat4& vp) {
    float L = world.pipeLength();
    if (world.pipeFront > 1.0f && world.pipeFront < L + 1.0f && !leaving) {
        char buf[64];
        std::snprintf(buf, sizeof(buf), "V = %.1f M/S", world.pipeSpeedAt(world.pipeFront));
        pointerLabel(ui, vp, world.pipePoint(std::fmin(world.pipeFront, L)) - Vec3(0, 2.5f, 0), 40, 70, buf);
    }
    if (world.pipeFront < 0.0f && t > 1.0f) pointerLabel(ui, vp, Vec3(56.0f, 39.0f, 14.5f), 60, 80, "WAITING FOR THE CONDUIT TO FILL");
    if (world.pipeFull() && !leaving) pointerLabel(ui, vp, Vec3(58.0f, 8.5f, 58.8f), -100, -90, "WATER STOPS AT THE CLOSED MAIN INLET VALVE");
}

void Scene4_Penstock::telemetry(std::vector<Telemetry>& out) {
    char buf[64];
    float L = world.pipeLength();
    float s = clampf(world.pipeFront, 0.0f, L);
    float y = world.pipePoint(s).y;
    std::snprintf(buf, sizeof(buf), "%.1f M", world.pipeFront > 0 ? 39.0f - y : 0.0f);
    out.push_back({"DROP SO FAR (H)", buf});
    std::snprintf(buf, sizeof(buf), "%.1f M/S", world.pipeFront > 0 ? world.pipeSpeedAt(s) : 0.0f);
    out.push_back({"FLOW SPEED", buf});
    std::snprintf(buf, sizeof(buf), "%.1f BAR", 1.0f + (RESERVOIR_Y - y) / 10.2f);
    out.push_back({"PIPE PRESSURE", buf});
    std::snprintf(buf, sizeof(buf), "%d %%", (int)(clampf(world.pipeFront / L, 0, 1) * 100));
    out.push_back({"PENSTOCK FILLED", buf});
    out.push_back({"MAIN INLET VALVE", world.valveLift > 0.5f ? "OPEN" : "CLOSED"});
}

void Scene4_Penstock::buttons(std::vector<Button>& out) {
    out.push_back({world.xrayOn ? "X-RAY OFF" : "X-RAY VIEW", 1, "X", false});
    out.push_back({"REPLAY FLOW", 2, "SPACE", false});
    out.push_back({"FOLLOW TO RAY-TRACED PIPE >", 3, "ENTER", true});
}

void Scene4_Penstock::onButton(int id) {
    if (leaving) return;
    if (id == 1) { xrayManual = true; world.xrayOn = !world.xrayOn; }
    if (id == 2) { world.restartPenstockFlow(); arrivedTime = 0; t = 3.0f; xrayManual = false; }
    if (id == 3) { leaving = true; leaveT = 0; world.xrayOn = true; }
}
