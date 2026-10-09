#include "Scene8_Shaft.h"
#include <cstdio>

using namespace TurbineDims;

void Scene8_Shaft::build() {
    // thin glowing scan rings around the shaft (drawn as a hologram)
    MeshBuilder b;
    for (int k = 0; k < 3; k++) {
        b.setTransform(translate(Vec3(0, k * 0.25f, 0)));
        b.torus(1.3f + k * 0.35f, 0.02f, 64, 4);
    }
    b.resetTransform();
    rings = b.build();
}

void Scene8_Shaft::enter(bool continuous) {
    t = 0;
    if (!continuous) {
        world.completeUpTo(ST_SHAFT);
        fromEye = Vec3(49.5f, 6.6f, 64.6f);
        fromTarget = Vec3(62.5f, 5.4f, 64.0f);
    }
    world.turbineSection = true;            // keep the turbine cut open at the start
}

void Scene8_Shaft::camera(Camera& cam) {
    CamKey keys[] = {
        {0.0f, fromEye, fromTarget},
        {2.5f, Vec3(57.8f, 9.5f, 61.0f), Vec3(CENTRE.x, 8.4f, CENTRE.z)},
        {DURATION - 1.0f, Vec3(57.7f, 16.6f, 59.3f), Vec3(CENTRE.x, 16.6f, CENTRE.z)},
    };
    evalCamPath(keys, 3, t, cam.eye, cam.target);
    cam.fovDeg = 56.0f;
    cam.nearZ = 0.1f;
    camY = cam.eye.y;
    if (t > 6.0f) world.turbineSection = false;
}

void Scene8_Shaft::lighting(Lighting& L, const Vec3& camPos) {
    world.lighting(L, camPos);
    // darker, bluer "blueprint" mood like the video
    L.ambient *= 0.7f;
    L.exposure *= 0.85f;
    L.fogColor = Vec3(0.08f, 0.12f, 0.18f);
    L.fogDensity = 0.03f;
}

void Scene8_Shaft::drawTransparent(Renderer& r) {
    world.drawTransparent(r);
    if (r.inShadowPass()) return;
    Material holo;
    holo.color = Vec3(0, 0, 0);
    holo.emissive = Vec3(0.3f, 0.85f, 1.6f);
    holo.alpha = 0.6f;
    holo.additive = true;
    holo.castShadow = false;
    // two scan rings: one travels with the camera, one sweeps up and down
    float y1 = camY + 0.4f, y2 = 8.0f + 6.0f * (0.5f + 0.5f * std::sin(t * 1.3f));
    r.drawMesh(rings, translate(Vec3(CENTRE.x, y1, CENTRE.z)), holo);
    r.drawMesh(rings, translate(Vec3(CENTRE.x, y2, CENTRE.z)), holo);
}

void Scene8_Shaft::hud(Ui& ui, const Mat4& vp) {
    // blueprint grid + frame
    const Vec4 faint(0.45f, 0.88f, 1.0f, 0.14f), cyan(0.45f, 0.88f, 1.0f, 0.8f);
    for (int i = 1; i < 16; i++) ui.rect(ui.W * i / 16.0f, 30, 1, ui.H - 60.0f, faint);
    for (int i = 1; i < 9; i++) ui.rect(0, 30 + (ui.H - 60.0f) * i / 9.0f, (float)ui.W, 1, faint);
    ui.corners(60, 150, ui.W - 120.0f, ui.H - 330.0f, 30, 2, cyan);
    // labels appear as the camera passes them
    struct Tag { float y; Vec3 p; const char* txt; };
    Tag tags[] = {
        {6.0f,  Vec3(CENTRE.x - 1.2f, 6.4f, CENTRE.z - 1.0f), "TURBINE RUNNER"},
        {8.0f,  Vec3(CENTRE.x - 0.8f, 8.0f, CENTRE.z - 0.8f), "TURBINE GUIDE BEARING"},
        {10.0f, Vec3(CENTRE.x - 0.6f, 10.6f, CENTRE.z - 0.6f), "COUPLING FLANGE (12 BOLTS)"},
        {12.0f, Vec3(CENTRE.x - 0.9f, 12.7f, CENTRE.z - 0.9f), "THRUST BEARING - CARRIES 600 T"},
        {14.5f, Vec3(CENTRE.x - 2.4f, 16.5f, CENTRE.z - 2.4f), "GENERATOR ROTOR"},
    };
    for (const Tag& g : tags)
        if (camY > g.y - 2.5f && camY < g.y + 4.5f) pointerLabel(ui, vp, g.p, -150, -30, g.txt);
}

void Scene8_Shaft::telemetry(std::vector<Telemetry>& out) {
    char buf[64];
    std::snprintf(buf, sizeof(buf), "%.0f RPM", world.rpm());
    out.push_back({"SHAFT SPEED", buf});
    float mw = Plant::powerMW(Plant::Q_UNIT * world.flow01(), Plant::HEAD);
    std::snprintf(buf, sizeof(buf), "%.1f MN.M", Plant::torqueMNm(mw, world.rpm()));
    out.push_back({"TORQUE", buf});
    std::snprintf(buf, sizeof(buf), "%.0f MW", mw);
    out.push_back({"MECHANICAL POWER", buf});
    out.push_back({"SHAFT DIAMETER", "0.84 M"});
}
