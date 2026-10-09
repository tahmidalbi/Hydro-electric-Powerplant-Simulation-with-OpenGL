#include "Scene5_IntakeGate.h"
#include "../objects/Materials.h"
#include <cstdio>

static const float RES_Y = 12.0f;         // reservoir level on the left
static const float CONDUIT_END = 46.0f;   // conduit length on the right
static const int NX = 120, NZ = 14;       // resolution of the moving water surface
static const float FZ0 = -8.9f, FZ1 = 6.3f;

void Scene5_IntakeGate::build() {
    gate.build();
    MeshBuilder b;
    // conduit walls: back wall, roof block (dark), upstream floor
    b.box(Vec3(-60, -3, -11), Vec3(60, 0, 7));                       // base slab
    b.box(Vec3(-60, -1, -11), Vec3(60, 21, -9));                     // back wall
    b.box(Vec3(-60, 21, -11), Vec3(-3.5f, 22.4f, -6));               // walkway left
    structure = b.build();
    b.clear();
    b.box(Vec3(1.3f, 11.0f, -9), Vec3(60, 21.0f, 7.0f));             // heavy roof over the conduit
    b.box(Vec3(1.3f, 21.0f, -9), Vec3(60, 22.4f, -3));
    darkBlock = b.build();
    b.clear();
    b.box(Vec3(0.8f, -0.01f, -9), Vec3(60, 0.0f, 7));                 // tiled conduit floor
    floorTiles = b.build();

    // reservoir water (left of the gate): surface + cut face
    b.clear();
    b.setTransform(translate(Vec3(-30.4f, RES_Y, -1.0f)));
    b.grid(59.2f, 16.0f, 120, 32);
    resTop = b.build();
    b.clear();
    for (int i = 0; i <= 120; i++) {
        float x = -60 + 59.6f * i / 120;
        b.addVertex(Vec3(x, -0.5f, 7.0f), Vec3(0, 0, 1), Vec2(x, 0));
        b.addVertex(Vec3(x, RES_Y, 7.0f), Vec3(0, 0, 1), Vec2(x, RES_Y));
    }
    for (int i = 0; i < 120; i++) { unsigned int k = i * 2; b.tri(k, k + 2, k + 3); b.tri(k, k + 3, k + 1); }
    resFront = b.build();

    // moving water in the conduit: CPU updates the heights every frame
    topVerts.resize((NX + 1) * (NZ + 1));
    frontVerts.resize((NX + 1) * 2);
    std::vector<unsigned int> idx;
    for (int j = 0; j < NZ; j++)
        for (int i = 0; i < NX; i++) {
            unsigned int a = j * (NX + 1) + i, c = a + NX + 1;
            idx.insert(idx.end(), {a, c, a + 1, a + 1, c, c + 1});
        }
    rebuildFlowMesh();
    flowTop.upload(topVerts, idx, true);
    std::vector<unsigned int> fidx;
    for (int i = 0; i < NX; i++) { unsigned int k = i * 2; fidx.insert(fidx.end(), {k, k + 2, k + 3, k, k + 3, k + 1}); }
    flowFront.upload(frontVerts, fidx, true);
}

// Height of the water surface in the conduit at position x.
float Scene5_IntakeGate::profile(float x) const {
    if (x > front || lift <= 0.01f) return 0.0f;
    float opening = std::fmin(lift, 4.0f);
    float jet = opening * 0.9f * std::exp(-(x - 2.0f) * (x - 2.0f) / 6.0f);   // bulge just after the gate
    float h = level + jet;
    float nose = clampf((front - x) / 3.0f, 0.0f, 1.0f);                       // rounded front
    h *= std::sqrt(nose);
    h += 0.08f * std::sin(x * 1.3f - t * 6.0f) * (h > 0.1f ? 1.0f : 0.0f);
    return h;
}

void Scene5_IntakeGate::rebuildFlowMesh() {
    for (int j = 0; j <= NZ; j++)
        for (int i = 0; i <= NX; i++) {
            float x = 0.85f + (CONDUIT_END - 0.85f) * i / NX;
            float z = mixf(FZ0, FZ1, (float)j / NZ);
            float h = profile(x);
            float hn = profile(x + 0.3f);
            Vec3 n = normalize(Vec3(-(hn - h) / 0.3f, 1.0f, 0.0f));
            Vertex& v = topVerts[j * (NX + 1) + i];
            v.pos = Vec3(x, std::fmax(h, 0.0f) + 0.02f, z);
            v.normal = n;
            v.uv = Vec2(z - FZ0, x);                 // uv.y runs along the flow
        }
    for (int i = 0; i <= NX; i++) {
        float x = 0.85f + (CONDUIT_END - 0.85f) * i / NX;
        float h = std::fmax(profile(x), 0.0f);
        frontVerts[i * 2].pos = Vec3(x, 0.02f, FZ1);
        frontVerts[i * 2 + 1].pos = Vec3(x, h + 0.02f, FZ1);
        frontVerts[i * 2].normal = frontVerts[i * 2 + 1].normal = Vec3(0, 0, 1);
        frontVerts[i * 2].uv = Vec2(x, 0);
        frontVerts[i * 2 + 1].uv = Vec2(x, h);
    }
}

void Scene5_IntakeGate::enter() {
    t = 0;
    gateTarget = everOpened = next = false;
    lift = 0;
    front = 0;
    level = 0;
    filledTime = 0;
    spray.clear();
    spray.softNoise = false;
}

void Scene5_IntakeGate::update(float dt) {
    t += dt;
    // hydraulic cylinders move the gate at a steady speed
    float target = gateTarget ? gate.maxLift : 0.0f;
    lift += clampf(target - lift, -dt * 1.6f, dt * 1.6f);

    if (lift > 0.05f) {
        // Torricelli: water leaves under the gate with v = sqrt(2 g h)
        float jetSpeed = std::sqrt(2.0f * 9.81f * RES_Y);
        front = std::fmin(front + dt * (2.0f + 0.35f * jetSpeed * std::fmin(lift, 3.0f) / 3.0f), CONDUIT_END + 3.0f);
        float targetLevel = std::fmin(1.0f + lift * 0.42f, 4.2f);
        level += (targetLevel - level) * std::fmin(dt * 0.8f, 1.0f);
        // spray where the jet leaves the gate
        for (int k = 0; k < 10; k++) {
            Particle p;
            p.pos = Vec3(rng.range(0.9f, 2.5f), rng.range(0.2f, std::fmin(lift, 3.5f)), rng.range(-6.5f, 6.0f));
            p.vel = Vec3(rng.range(4.0f, 9.0f), rng.range(0.5f, 3.5f), rng.range(-0.8f, 0.8f));
            p.size = rng.range(0.06f, 0.16f);
            p.maxLife = rng.range(0.4f, 0.9f);
            p.gravity = -9.81f;
            p.color = Vec3(0.88f, 0.94f, 1.0f);
            p.alpha = 0.8f;
            spray.emit(p);
        }
    } else {
        level = std::fmax(level - dt * 0.6f, 0.0f);   // conduit drains when closed
        if (level <= 0.01f) front = 0;
    }
    if (everOpened && front >= CONDUIT_END && lift > 4.0f) filledTime += dt;
    spray.update(dt);
    rebuildFlowMesh();
    flowTop.updateVertices(topVerts);
    flowFront.updateVertices(frontVerts);
}

void Scene5_IntakeGate::camera(Camera& cam) {
    static const CamKey keys[] = {
        {0.0f,  Vec3(15.0f, 14.5f, 29.0f), Vec3(1.0f, 6.0f, -1.0f)},
        {12.0f, Vec3(17.0f, 13.0f, 24.0f), Vec3(2.5f, 5.0f, -1.0f)},
    };
    evalCamPath(keys, 2, t, cam.eye, cam.target);
    cam.fovDeg = 50.0f;
}

void Scene5_IntakeGate::lighting(Lighting& L) {
    L.sunDir = normalize(Vec3(-0.45f, 0.8f, 0.55f));
    L.sunColor = Vec3(2.2f, 2.1f, 1.95f);
    L.skyTop = Vec3(0.45f, 0.52f, 0.6f);
    L.skyHorizon = Vec3(0.68f, 0.71f, 0.74f);
    L.ground = Vec3(0.25f, 0.25f, 0.25f);
    L.ambient = 0.6f;
    L.fogColor = Vec3(0.62f, 0.65f, 0.68f);
    L.fogDensity = 0.005f;
    L.skyMode = 1;
    L.waterLevel = RES_Y;
    L.caustics = 0.45f;
    L.causticRect = Vec4(-60, -9, 0, 7);
    L.exposure = 0.5f;
}

void Scene5_IntakeGate::drawOpaque(Renderer& r) {
    Mat4 I;
    r.drawMesh(structure, I, Mat::darkConcrete(1.4f));
    r.drawMesh(darkBlock, I, Mat::concrete(1.4f, Vec3(0.24f, 0.24f, 0.24f)));
    Material tiles;
    tiles.color = Vec3(0.42f, 0.43f, 0.44f);
    tiles.pattern = PAT_TILES;
    tiles.patternScale = 0.5f;
    tiles.specular = 0.3f;
    r.drawMesh(floorTiles, I, tiles);
    gate.draw(r, lift);
}

void Scene5_IntakeGate::drawTransparent(Renderer& r) {
    WaterStyle res;
    res.mode = 0;
    res.deep = Vec3(0.02f, 0.12f, 0.2f);
    res.shallow = Vec3(0.1f, 0.38f, 0.52f);
    res.alpha = 0.75f;
    res.ripple = 0.45f;
    res.rippleStrength = 1.3f;
    res.waveAmp = 0.12f + 0.15f * clampf(lift / 3.0f, 0, 1);   // more agitated when the gate is open
    res.waveFreq = 0.6f;
    res.surfaceY = RES_Y;
    res.flowSpeed = 0.6f * clampf(lift, 0, 1);
    WaterStyle face = res;
    face.mode = 3;
    face.alpha = 0.74f;
    face.deep = Vec3(0.02f, 0.12f, 0.24f);
    face.shallow = Vec3(0.05f, 0.38f, 0.58f);
    r.drawWater(resFront, Mat4(), face);
    r.drawWater(resTop, Mat4(), res);

    if (front > 0.1f) {
        WaterStyle flow;
        flow.mode = 1;
        flow.deep = Vec3(0.03f, 0.16f, 0.25f);
        flow.shallow = Vec3(0.2f, 0.5f, 0.62f);
        flow.alpha = 0.72f;
        flow.foam = 0.55f;
        flow.flowSpeed = 9.0f;
        flow.ripple = 0.9f;
        flow.front = front;
        flow.sheetWidth = FZ1 - FZ0;
        r.drawWater(flowTop, Mat4(), flow);
        WaterStyle ff = face;
        ff.surfaceY = level;
        ff.waveAmp = 0;
        ff.alpha = 0.66f;
        r.drawWater(flowFront, Mat4(), ff);
    }
    spray.draw(r);
}

void Scene5_IntakeGate::telemetry(std::vector<Telemetry>& out) {
    char buf[64];
    std::snprintf(buf, sizeof(buf), "%.1f M (%d %%)", lift, (int)(lift / gate.maxLift * 100 + 0.5f));
    out.push_back({"GATE OPENING", buf});
    std::snprintf(buf, sizeof(buf), "%.1f M/S", lift > 0.05f ? std::sqrt(2.0f * 9.81f * RES_Y) : 0.0f);
    out.push_back({"JET SPEED  V=SQRT(2GH)", buf});
    std::snprintf(buf, sizeof(buf), "%.0f M3/S", lift > 0.05f ? 13.6f * std::fmin(lift, 4.0f) * 0.6f * std::sqrt(2.0f * 9.81f * RES_Y) : 0.0f);
    out.push_back({"DISCHARGE", buf});
    std::snprintf(buf, sizeof(buf), "%d %%", (int)(clampf(front / CONDUIT_END, 0, 1) * 100));
    out.push_back({"CONDUIT FILLED", buf});
    out.push_back({"HYDRAULIC PRESSURE", lift > 0.01f && lift < gate.maxLift - 0.01f ? "180 BAR" : "HOLD"});
}

void Scene5_IntakeGate::buttons(std::vector<Button>& out) {
    out.push_back({gateTarget ? "CLOSE GATE" : "OPEN GATE", 1, "O", !gateTarget});
    out.push_back({"FOLLOW INTO PENSTOCK >", 2, "ENTER", gateTarget});
}

void Scene5_IntakeGate::onButton(int id) {
    if (id == 1) {
        gateTarget = !gateTarget;
        if (gateTarget) everOpened = true;
    }
    if (id == 2) next = true;
}
