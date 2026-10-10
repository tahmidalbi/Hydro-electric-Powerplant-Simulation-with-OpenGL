#include "Scene6_InletValve.h"
#include <cstdio>

void Scene6_InletValve::enter(bool continuous) {
    t = 0;
    next = false;
    if (!continuous) world.completeUpTo(ST_VALVE);
    world.turbineSection = false;
    world.xrayOn = false;               // back to the real steel look after the ray-traced view
}

// side view into the cut-away pit: penstock (left) -> valve -> spiral casing (right)
void Scene6_InletValve::camera(Camera& cam) {
    static const CamKey keys[] = {
        {0.0f,  Vec3(40.0f, 10.5f, 50.0f), Vec3(60.0f, 6.4f, 61.0f)},
        {16.0f, Vec3(42.5f, 9.6f, 52.5f), Vec3(60.5f, 6.2f, 62.0f)},
    };
    evalCamPath(keys, 2, t, cam.eye, cam.target);
    cam.fovDeg = 54.0f;
}

void Scene6_InletValve::hud(Ui& ui, const Mat4& vp) {
    if (t < 1.0f) return;
    pointerLabel(ui, vp, Vec3(58.0f, 9.0f, 58.8f), -60, 120, "MAIN INLET VALVE");
    pointerLabel(ui, vp, Vec3(56.0f, 7.5f, 53.0f), -80, 70, "PENSTOCK (FULL, 5.9 BAR)");
    pointerLabel(ui, vp, Vec3(63.0f, 6.0f, 67.5f), 40, 110, "SPIRAL CASING");
    if (world.valveLift > 0.05f && world.valveLift < 0.99f) pointerLabel(ui, vp, Vec3(57.0f, 6.0f + 2.25f + world.valveLift * 4.4f, 58.8f), 120, 20, "VALVE GATE RISING");
}

void Scene6_InletValve::telemetry(std::vector<Telemetry>& out) {
    char buf[64];
    std::snprintf(buf, sizeof(buf), "%d %%", (int)(world.valveLift * 100.0f + 0.5f));
    out.push_back({"VALVE OPENING", buf});
    out.push_back({"PRESSURE AT VALVE", "5.9 BAR"});
    std::snprintf(buf, sizeof(buf), "%d %%", (int)(clampf(world.casingFront / world.turbine.casingLength, 0, 1) * 100));
    out.push_back({"SPIRAL CASING FILLED", buf});
    out.push_back({"GUIDE VANES", world.vaneOpening > 0.05f ? "OPEN" : "CLOSED"});
    std::snprintf(buf, sizeof(buf), "%.0f RPM", world.rpm());
    out.push_back({"TURBINE", buf});
}

void Scene6_InletValve::buttons(std::vector<Button>& out) {
    out.push_back({world.valveOpen ? "CLOSE VALVE" : "OPEN VALVE", 1, "O", !world.valveOpen});
    Button follow{"FOLLOW TO TURBINE >", 2, "ENTER", true};
    follow.enabled = ready();
    follow.hint = world.valveOpen ? "(WAIT - THE CASING IS FILLING)" : "(OPEN THE VALVE FIRST)";
    out.push_back(follow);
}

void Scene6_InletValve::onButton(int id) {
    if (id == 1) world.valveOpen = !world.valveOpen;
    if (id == 2 && ready()) next = true;
}

void Scene6_InletValve::autoPlay() {
    if (t > 3.0f && !world.valveOpen) world.valveOpen = true;
    if (ready() && t > 12.0f) next = true;
}
