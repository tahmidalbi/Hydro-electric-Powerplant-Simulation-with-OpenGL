#include "Scene4_TrashRack.h"
#include "../objects/Materials.h"
#include <cstdio>

// Channel: flow along +x, floor at y = 0, channel width z = -7 .. 7 (front cut at z = 7)
static const float CAGE_X = 4.0f;    // cage spans x = -4 .. 4
static const float CAGE_Z = 6.6f;    // and z = -6.6 .. 6.6

void Scene4_TrashRack::build() {
    MeshBuilder b;
    // concrete floor slab + the rack foundation sunk into the ground
    b.box(Vec3(-60, -1.6f, -9), Vec3(60, 0, 7));
    b.box(Vec3(-CAGE_X - 1.2f, -5.5f, -9), Vec3(CAGE_X + 1.2f, -1.6f, 7));
    // pillars and the heavy beam above the cage
    b.box(Vec3(-CAGE_X - 4.5f, 8.8f, -9), Vec3(-CAGE_X - 0.2f, 13.5f, 7.2f));
    b.box(Vec3(CAGE_X + 0.2f, 8.8f, -9), Vec3(CAGE_X + 4.5f, 13.5f, 7.2f));
    b.box(Vec3(-CAGE_X - 0.2f, 11.4f, -9), Vec3(CAGE_X + 0.2f, 13.5f, 7.2f));
    b.box(Vec3(-60, 8.8f, -9), Vec3(-CAGE_X - 4.5f, 9.6f, -6.5f));      // side ledges
    b.box(Vec3(CAGE_X + 4.5f, 8.8f, -9), Vec3(60, 9.6f, -6.5f));
    floorSlab = b.build();

    b.clear();     // soil under the slab (seen in the cut)
    b.box(Vec3(-60, -6.0f, -9), Vec3(-CAGE_X - 1.2f, -1.6f, 6.98f));
    b.box(Vec3(CAGE_X + 1.2f, -6.0f, -9), Vec3(60, -1.6f, 6.98f));
    soil = b.build();

    b.clear();     // back wall of the channel and the building behind
    b.box(Vec3(-60, -1.6f, -11), Vec3(60, 8.8f, -9));
    b.box(Vec3(-60, 13.5f, -14), Vec3(60, 22, -11));
    b.box(Vec3(-60, 8.8f, -14), Vec3(60, 13.5f, -11));
    backWall = b.build();

    b.clear();     // upper deck with a railing (top right, like the video)
    b.box(Vec3(CAGE_X + 4.5f, 13.5f, -9), Vec3(60, 14.1f, -2));
    upper = b.build();
    b.clear();
    for (float x = CAGE_X + 5.0f; x < 40; x += 1.6f) b.box(Vec3(x, 14.1f, -2.3f), Vec3(x + 0.07f, 15.3f, -2.23f));
    b.box(Vec3(CAGE_X + 5.0f, 15.25f, -2.32f), Vec3(40, 15.35f, -2.2f));
    b.box(Vec3(CAGE_X + 5.0f, 14.7f, -2.3f), Vec3(40, 14.76f, -2.23f));
    railing = b.build();

    // two blue rake motors on the beam
    b.clear();
    for (int i = -1; i <= 1; i += 2) {
        float x = i * 2.1f;
        b.setTransform(translate(Vec3(x - 0.75f, 13.5f + 0.9f, 4.6f)) * rotateZ(-PI * 0.5f) * translate(Vec3(0, 0, 0)));
        b.cylinder(0.75f, 0.75f, 1.5f, 24, true, true);
        b.resetTransform();
        b.box(Vec3(x - 0.9f, 13.5f, 3.6f), Vec3(x + 0.9f, 13.75f, 5.6f));      // base plate
        b.box(Vec3(x - 0.4f, 13.5f, 5.3f), Vec3(x + 0.4f, 14.6f, 6.2f));       // gearbox
    }
    motors = b.build();
    b.clear();
    for (int i = -1; i <= 1; i += 2) {
        b.setTransform(translate(Vec3(i * 2.1f, 13.6f, 5.75f)));
        b.cylinder(0.12f, 0.12f, 1.4f, 10, true, true);
    }
    motorShafts = b.build();

    // ---- the cage of bars: four rack panels --------------------------------
    RackSpec face;                // front & back faces (bars spread along x)
    face.width = 2 * CAGE_X;
    face.y0 = -1.6f;
    face.y1 = 11.4f;
    face.spacing = 0.8f;
    face.barW = 0.24f;
    face.barD = 0.24f;
    face.tips = false;
    face.brackets = false;
    cage.build(face, translate(Vec3(0, 0, CAGE_Z)));
    cage.build(face, translate(Vec3(0, 0, -CAGE_Z)));
    RackSpec side = face;         // upstream & downstream faces (bars spread along z)
    side.width = 2 * CAGE_Z;
    side.spacing = 0.6f;
    side.barW = 0.16f;
    side.barD = 0.3f;
    cage.build(side, translate(Vec3(-CAGE_X, 0, 0)) * rotateY(PI * 0.5f));
    cage.build(side, translate(Vec3(CAGE_X, 0, 0)) * rotateY(PI * 0.5f));
    cage.finish();
    debris.build();

    // ---- water volume: top surface (waves) + the cut front face ------------
    b.clear();
    b.setTransform(translate(Vec3(0, WATER_Y, -1.4f)));
    b.grid(120, 15.2f, 240, 30);
    waterTop = b.build();
    b.clear();
    {
        const int NX = 240;
        for (int i = 0; i <= NX; i++) {
            float x = -60 + 120.0f * i / NX;
            b.addVertex(Vec3(x, 0.0f, 6.15f), Vec3(0, 0, 1), Vec2(x, 0));
            b.addVertex(Vec3(x, WATER_Y, 6.15f), Vec3(0, 0, 1), Vec2(x, WATER_Y));
        }
        for (int i = 0; i < NX; i++) {
            unsigned int k = i * 2;
            b.tri(k, k + 2, k + 3);
            b.tri(k, k + 3, k + 1);
        }
    }
    waterFront = b.build();
}

void Scene4_TrashRack::enter() {
    t = 0;
    next = false;
    wave = 0;
    nextWaveAt = 0.0f;
    debris.reset();
    splash.clear();
    splash.softNoise = false;
}

void Scene4_TrashRack::update(float dt) {
    t += dt;
    if (wave < 3 && t >= nextWaveAt) {          // the video sends trash in three groups
        debris.spawnWave(wave);
        wave++;
        nextWaveAt = t + 2.6f;
    }
    debris.update(dt, t, splash);
    splash.update(dt);
}

void Scene4_TrashRack::camera(Camera& cam) {
    static const CamKey keys[] = {
        {0.0f,  Vec3(-2.5f, 8.2f, 26.0f), Vec3(-1.5f, 5.6f, 0)},
        {13.0f, Vec3(1.5f, 7.8f, 23.5f),  Vec3(-0.5f, 5.6f, 0)},
    };
    evalCamPath(keys, 2, t, cam.eye, cam.target);
    cam.fovDeg = 48.0f;
}

void Scene4_TrashRack::lighting(Lighting& L) {
    // bright "studio" light of a technical cut-away illustration
    L.sunDir = normalize(Vec3(-0.35f, 0.85f, 0.55f));
    L.sunColor = Vec3(2.2f, 2.15f, 2.05f);
    L.skyTop = Vec3(0.62f, 0.66f, 0.7f);
    L.skyHorizon = Vec3(0.78f, 0.8f, 0.82f);
    L.ground = Vec3(0.35f, 0.34f, 0.33f);
    L.ambient = 0.62f;
    L.fogColor = Vec3(0.75f, 0.77f, 0.8f);
    L.fogDensity = 0.004f;
    L.skyMode = 1;
    L.waterLevel = WATER_Y;
    L.caustics = 0.55f;
    L.causticRect = Vec4(-60, -9, 60, 6.2f);
    L.exposure = 0.47f;
}

void Scene4_TrashRack::drawOpaque(Renderer& r) {
    Mat4 I;
    r.drawMesh(floorSlab, I, Mat::concrete(1.6f, Vec3(0.55f, 0.56f, 0.56f)));
    Material soilM;
    soilM.color = Vec3(0.3f, 0.22f, 0.15f);
    soilM.pattern = PAT_SOIL;
    soilM.patternScale = 1.4f;
    r.drawMesh(soil, I, soilM);
    r.drawMesh(backWall, I, Mat::concrete(1.6f, Vec3(0.6f, 0.61f, 0.62f)));
    r.drawMesh(upper, I, Mat::concrete(1.6f, Vec3(0.6f, 0.61f, 0.62f)));
    r.drawMesh(railing, I, Mat::steel(Vec3(0.8f, 0.8f, 0.8f)));
    r.drawMesh(motors, I, Mat::paint(Vec3(0.05f, 0.22f, 0.65f), 0.7f));
    r.drawMesh(motorShafts, I, Mat::steel());
    cage.draw(r, I);
    debris.draw(r);
}

void Scene4_TrashRack::drawTransparent(Renderer& r) {
    WaterStyle top;
    top.mode = 0;
    top.deep = Vec3(0.02f, 0.13f, 0.22f);
    top.shallow = Vec3(0.1f, 0.4f, 0.55f);
    top.alpha = 0.7f;
    top.ripple = 0.6f;
    top.rippleStrength = 1.2f;
    top.flowSpeed = 2.4f;
    top.waveAmp = 0.13f;
    top.waveFreq = 0.9f;
    top.surfaceY = WATER_Y;
    top.foamSpot = Vec4(-CAGE_X - 0.6f, 1.0f, 3.5f, 0.55f);    // churned water in front of the bars
    WaterStyle face = top;
    face.mode = 3;
    face.deep = Vec3(0.02f, 0.14f, 0.26f);
    face.shallow = Vec3(0.05f, 0.4f, 0.6f);
    face.alpha = 0.72f;
    r.drawWater(waterFront, Mat4(), face);
    r.drawWater(waterTop, Mat4(), top);
    splash.draw(r);
}

// small labels pointing at what was caught
void Scene4_TrashRack::hud(Ui& ui, const Mat4& vp) {
    const Vec4 cyan(0.45f, 0.88f, 1.0f, 0.9f), white(1, 1, 1, 0.95f);
    auto label = [&](const Vec3& p, float dx, float dy, const char* txt) {
        float x, y;
        if (!projectToScreen(vp, p, ui.W, ui.H, x, y)) return;
        ui.rect(x - 4, y - 4, 8, 8, cyan);
        ui.line(x, y, x + dx, y + dy, 1.5f, cyan);
        float w = ui.textWidth(txt, 1.5f) + 14;
        float lx = dx > 0 ? x + dx : x + dx - w;
        lx = clampf(lx, 10.0f, ui.W - w - 10.0f);
        ui.rect(lx, y + dy - 12, w, 22, Vec4(0.02f, 0.06f, 0.1f, 0.7f));
        ui.text(lx + 7, y + dy - 6, txt, 1.5f, white);
    };
    if (t > 1.0f) label(Vec3(CAGE_X, 9.5f, CAGE_Z), 70, 30, "STEEL BAR SCREEN");
    if (t > 4.5f && debris.stuckCount() > 0) label(Vec3(-CAGE_X - 2.2f, 2.0f, 2.0f), -120, 60, "TRASH TRAPPED");
    if (t > 6.0f) label(Vec3(CAGE_X + 5.0f, 2.5f, 4.0f), 40, 70, "CLEAN WATER TO TURBINE");
    if (t > 2.0f) label(Vec3(-2.1f, 14.3f, 4.6f), 80, -30, "RAKE DRIVE MOTORS");
}

void Scene4_TrashRack::telemetry(std::vector<Telemetry>& out) {
    char buf[64];
    std::snprintf(buf, sizeof(buf), "%d", debris.stuckCount());
    out.push_back({"PIECES TRAPPED", buf});
    std::snprintf(buf, sizeof(buf), "%d", debris.passedCount());
    out.push_back({"SMALL LEAVES PASSED", buf});
    float loss = 0.04f + debris.stuckCount() * 0.012f;
    std::snprintf(buf, sizeof(buf), "%.2f M", loss);
    out.push_back({"HEAD LOSS AT RACK", buf});
    out.push_back({"FLOW SPEED", "2.4 M/S"});
}

void Scene4_TrashRack::buttons(std::vector<Button>& out) {
    out.push_back({"THROW MORE TRASH", 1, "T", false});
    out.push_back({"RAKE CLEAN", 2, "K", false});
    out.push_back({"GO TO INTAKE GATE >", 3, "ENTER", true});
}

void Scene4_TrashRack::onButton(int id) {
    if (id == 1) debris.spawnWave(1 + (int)(t * 10) % 3);
    if (id == 2) debris.rakeClean();
    if (id == 3) next = true;
}
