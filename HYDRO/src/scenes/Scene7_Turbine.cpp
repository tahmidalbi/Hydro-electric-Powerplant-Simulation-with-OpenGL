#include "Scene7_Turbine.h"
#include <cstdio>

using namespace TurbineDims;

void Scene7_Turbine::enter(bool continuous) {
    t = 0;
    next = false;
    section = 0;
    if (!continuous) {
        world.completeUpTo(ST_TURBINE);
        fromEye = Vec3(42.5f, 9.6f, 52.5f);
        fromTarget = Vec3(60.5f, 6.2f, 62.0f);
    }
    world.turbineSection = false;
}

void Scene7_Turbine::update(float dt) {
    t += dt;
    section += clampf((world.turbineSection ? 1.0f : 0.0f) - section, -dt * 0.7f, dt * 0.7f);
    world.update(dt);
}

void Scene7_Turbine::camera(Camera& cam) {
    // top view: from above the casing, under the generator floor
    CamKey keys[] = {
        {0.0f, fromEye, fromTarget},
        {3.5f, Vec3(54.5f, 11.4f, 69.5f), Vec3(62.5f, 5.2f, 63.8f)},
        {25.0f, Vec3(55.8f, 11.6f, 70.8f), Vec3(62.5f, 5.2f, 63.8f)},
    };
    Vec3 eye, target;
    evalCamPath(keys, 3, t, eye, target);
    // section view: level with the runner, looking at the cut casing
    Vec3 sEye(49.5f, 6.6f, 64.6f), sTarget(62.5f, 5.4f, 64.0f);
    float s = section * section * (3.0f - 2.0f * section);
    cam.eye = mix(eye, sEye, s);
    cam.target = mix(target, sTarget, s);
    cam.fovDeg = 54.0f;
    cam.nearZ = 0.1f;
}

void Scene7_Turbine::hud(Ui& ui, const Mat4& vp) {
    if (t < 3.0f) return;
    if (section < 0.5f) {
        pointerLabel(ui, vp, Vec3(CENTRE.x + 1.5f, CENTRE.y + 1.4f, CENTRE.z + 3.8f), 80, -60, "SPIRAL CASING");
        pointerLabel(ui, vp, Vec3(CENTRE.x - STAY_R * 0.7f, CENTRE.y + 0.8f, CENTRE.z + STAY_R * 0.7f), -110, -70, "STAY VANES (FIXED)");
        pointerLabel(ui, vp, Vec3(CENTRE.x + GUIDE_R * 0.7f, CENTRE.y + 0.7f, CENTRE.z - GUIDE_R * 0.7f), 90, 50, "GUIDE VANES (MOVABLE)");
        pointerLabel(ui, vp, Vec3(CENTRE.x, CENTRE.y + 1.0f, CENTRE.z), -100, 80, "RUNNER - 15 CURVED BLADES");
    } else {
        pointerLabel(ui, vp, Vec3(CENTRE.x, CENTRE.y, CENTRE.z - 1.2f), -110, -70, "RUNNER");
        pointerLabel(ui, vp, Vec3(CENTRE.x, 4.0f, CENTRE.z - 1.2f), -100, 70, "DRAFT TUBE: WATER LEAVES TO THE RIVER");
        pointerLabel(ui, vp, Vec3(CENTRE.x, CENTRE.y + 1.5f, CENTRE.z - 0.3f), 80, -80, "SHAFT TO THE GENERATOR");
    }
}

void Scene7_Turbine::telemetry(std::vector<Telemetry>& out) {
    char buf[64];
    std::snprintf(buf, sizeof(buf), "%d %%", (int)(world.vaneOpening * 100.0f + 0.5f));
    out.push_back({"GUIDE VANE OPENING", buf});
    std::snprintf(buf, sizeof(buf), "%.0f M3/S", Plant::Q_UNIT * world.flow01());
    out.push_back({"FLOW Q", buf});
    std::snprintf(buf, sizeof(buf), "%.0f RPM", world.rpm());
    out.push_back({"RUNNER SPEED", buf});
    float mw = Plant::powerMW(Plant::Q_UNIT * world.flow01(), Plant::HEAD) * clampf(world.rpm() / Plant::RPM, 0, 1);
    std::snprintf(buf, sizeof(buf), "%.1f MN.M", Plant::torqueMNm(mw, world.rpm()));
    out.push_back({"SHAFT TORQUE", buf});
    std::snprintf(buf, sizeof(buf), "%.0f MW", mw);
    out.push_back({"MECHANICAL POWER", buf});
}

void Scene7_Turbine::buttons(std::vector<Button>& out) {
    out.push_back({world.vanesOpen ? "CLOSE GUIDE VANES" : "OPEN GUIDE VANES", 1, "O", !world.vanesOpen});
    out.push_back({world.turbineSection ? "TOP VIEW" : "SECTION VIEW", 2, "X", false});
    Button follow{"FOLLOW UP THE SHAFT >", 3, "ENTER", true};
    follow.enabled = ready();
    follow.hint = world.vanesOpen ? "(WAIT - THE RUNNER IS SPEEDING UP)" : "(OPEN THE GUIDE VANES FIRST)";
    out.push_back(follow);
}

void Scene7_Turbine::onButton(int id) {
    if (id == 1) world.vanesOpen = !world.vanesOpen;
    if (id == 2) world.turbineSection = !world.turbineSection;
    if (id == 3 && ready()) next = true;
}

void Scene7_Turbine::autoPlay() {
    if (t > 4.0f && !world.vanesOpen) world.vanesOpen = true;
    if (t > 13.0f && t < 13.1f && !world.turbineSection) world.turbineSection = true;
    if (ready() && t > 21.0f) next = true;
}
