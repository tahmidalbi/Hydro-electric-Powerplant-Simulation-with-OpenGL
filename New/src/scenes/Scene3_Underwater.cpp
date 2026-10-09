#include "Scene3_Underwater.h"
#include "../objects/Materials.h"
#include "../objects/Rocks.h"
#include <cstdio>

// Opening of the intake in the dam face
static const float OX = 5.5f, OY0 = 18.0f, OY1 = 28.0f;

void Scene3_Underwater::build() {
    MeshBuilder b;
    // dam face (z = 0 .. 6) with a hole for the intake tunnel
    b.box(Vec3(-70, -12, 0), Vec3(-OX, 46, 6));
    b.box(Vec3(OX, -12, 0), Vec3(70, 46, 6));
    b.box(Vec3(-OX, -12, 0), Vec3(OX, OY0, 6));
    b.box(Vec3(-OX, OY1, 0), Vec3(OX, 46, 6));
    wall = b.build();

    // the lighter, slightly protruding intake column around the opening
    b.clear();
    b.box(Vec3(-9.5f, -12, -1.6f), Vec3(-OX, 46, 0));
    b.box(Vec3(OX, -12, -1.6f), Vec3(9.5f, 46, 0));
    b.box(Vec3(-OX, -12, -1.6f), Vec3(OX, OY0 - 0.6f, 0));
    b.box(Vec3(-OX, OY1 + 2.2f, -1.6f), Vec3(OX, 46, 0));
    column = b.build();

    // sloped hood over the opening (the dark "visor" seen in the video)
    b.clear();
    b.quad(Vec3(-OX, OY1 + 2.2f, -1.6f), Vec3(OX, OY1 + 2.2f, -1.6f), Vec3(OX, OY1, 1.5f), Vec3(-OX, OY1, 1.5f));
    b.box(Vec3(-OX, OY0 - 0.6f, -1.6f), Vec3(OX, OY0, 0.0f));          // sill
    hood = b.build();

    // tunnel behind the opening
    b.clear();
    b.box(Vec3(-OX - 1, OY0 - 1, 0), Vec3(OX + 1, OY0, 34));            // floor
    b.box(Vec3(-OX - 1, OY1, 0), Vec3(OX + 1, OY1 + 1, 34));            // roof
    b.box(Vec3(-OX - 1, OY0, 0), Vec3(-OX, OY1, 34));                   // walls
    b.box(Vec3(OX, OY0, 0), Vec3(OX + 1, OY1, 34));
    tunnel = b.build();
    b.clear();
    b.box(Vec3(-OX, OY0, 33.5f), Vec3(OX, OY1, 34));
    glowEnd = b.build();

    // the steel trash rack in front of the opening
    RackSpec spec;
    spec.width = 13.0f;
    spec.y0 = OY0 - 1.2f;
    spec.y1 = OY1 + 1.4f;
    spec.spacing = 0.9f;
    spec.barW = 0.14f;
    spec.barD = 0.55f;
    rack.build(spec, translate(Vec3(0, 0, -2.4f)));
    rack.finish();

    // boulders piled at the foot of the wall
    b.clear();
    Rng r(11);
    for (int i = 0; i < 26; i++) {
        float side = i % 2 == 0 ? -1.0f : 1.0f;
        float x = side * r.range(10.0f, 30.0f);
        float z = r.range(-12.0f, -1.0f);
        float y = r.range(-1.0f, 3.0f) + std::fmax(0.0f, 8.0f - std::fabs(x) * 0.2f) * 0.0f;
        addRock(b, Vec3(x, y + std::fmax(0.0f, (std::fabs(x) - 10.0f)) * 0.25f, z), r.range(1.2f, 3.6f), 100 + i);
    }
    for (int i = 0; i < 10; i++) addRock(b, Vec3(r.range(-9, 9), r.range(-1.5f, 0.0f), r.range(-14, -3)), r.range(0.6f, 1.5f), 300 + i);
    rocks = b.build();

    b.clear();
    b.setTransform(translate(Vec3(0, -2.0f, -30)));
    b.grid(160, 80, 40, 20);
    seabed = b.build();

    // water surface seen from below
    b.clear();
    b.setTransform(translate(Vec3(0, SURFACE_Y, -40)));
    b.grid(240, 120, 60, 30);
    surface = b.build();

    // light shafts: tall thin quads hanging from the surface, tilted with the sun
    b.clear();
    Rng rs(3);
    for (int i = 0; i < 16; i++) {
        float x = rs.range(9, 34) * (i % 2 == 0 ? -1.0f : 1.0f), z = rs.range(-9, -2), w = rs.range(1.0f, 3.5f), len = rs.range(26, 42);
        Vec3 top(x, SURFACE_Y, z);
        Vec3 dir = normalize(Vec3(-0.22f, -1.0f, 0.3f));
        Vec3 side = normalize(Vec3(1, 0, rs.range(-0.5f, 0.5f)));
        Vec3 a = top - side * w * 0.5f, c = top + side * w * 0.5f;
        unsigned int k = (unsigned int)b.verts.size();
        Vec3 n(0, 0, -1);
        b.addVertex(a, n, Vec2(0, 0));
        b.addVertex(c, n, Vec2(1, 0));
        b.addVertex(c + dir * len, n, Vec2(1, 1));
        b.addVertex(a + dir * len, n, Vec2(0, 1));
        b.tri(k, k + 1, k + 2);
        b.tri(k, k + 2, k + 3);
    }
    shafts = b.build();
}

void Scene3_Underwater::enter() {
    t = 0;
    skip = false;
    dust.clear();
    dust.softNoise = false;
    // fill the water with floating specks
    for (int i = 0; i < 900; i++) {
        Particle p;
        p.pos = Vec3(rng.range(-25, 25), rng.range(2, 38), rng.range(-40, 20));
        p.vel = Vec3(rng.range(-0.1f, 0.1f), rng.range(-0.05f, 0.08f), rng.range(0.05f, 0.25f));
        p.size = rng.range(0.025f, 0.07f);
        p.maxLife = rng.range(20.0f, 40.0f);
        p.life = rng.range(0.5f, 10.0f);
        p.gravity = 0;
        p.color = Vec3(0.75f, 0.9f, 0.95f);
        p.alpha = rng.range(0.4f, 0.9f);
        dust.emit(p);
    }
}

void Scene3_Underwater::update(float dt) {
    t += dt;
    // rising bubbles now and then
    if (rng.next() < dt * 6.0f) {
        Particle p;
        p.pos = Vec3(rng.range(-14, 14), rng.range(0, 6), rng.range(-12, -3));
        p.vel = Vec3(0, rng.range(1.0f, 2.0f), 0);
        p.size = rng.range(0.05f, 0.14f);
        p.maxLife = 14;
        p.gravity = 0.4f;
        p.drag = 0.5f;
        p.color = Vec3(0.85f, 0.95f, 1.0f);
        p.alpha = 0.8f;
        dust.emit(p);
    }
    // the water drifts slowly into the intake
    for (Particle& p : dust.particles)
        if (std::fabs(p.pos.x) < 7 && p.pos.y > OY0 && p.pos.y < OY1 && p.pos.z > -10) p.vel.z += dt * 0.15f;
    dust.update(dt);
}

// Swim straight at the rack, slow down, then glide between two bars.
void Scene3_Underwater::camera(Camera& cam) {
    static const CamKey keys[] = {
        {0.0f,  Vec3(0, 24.5f, -36), Vec3(0, 28.5f, 0)},
        {5.5f,  Vec3(0, 23.6f, -15), Vec3(0, 23.6f, 0)},
        {8.0f,  Vec3(0, 23.2f, -5),  Vec3(0, 23.2f, 10)},
        {10.5f, Vec3(0, 23.0f, 9),   Vec3(0, 23.0f, 32)},
    };
    evalCamPath(keys, 4, t, cam.eye, cam.target);
    cam.fovDeg = 52.0f;
    cam.nearZ = 0.05f;
    cam.farZ = 400.0f;
    camPos = cam.eye;
}

void Scene3_Underwater::lighting(Lighting& L) {
    L.sunDir = normalize(Vec3(0.22f, 1.0f, -0.3f));
    L.sunColor = Vec3(1.3f, 1.75f, 1.85f);
    L.skyTop = Vec3(0.12f, 0.35f, 0.45f);
    L.skyHorizon = Vec3(0.06f, 0.2f, 0.27f);
    L.ground = Vec3(0.02f, 0.06f, 0.08f);
    L.ambient = 0.85f;
    L.fogColor = Vec3(0.03f, 0.16f, 0.23f);
    L.fogDensity = 0.017f;
    L.skyMode = 2;
    L.underwater = true;
    L.waterLevel = SURFACE_Y;
    L.caustics = 1.6f;
    L.exposure = 0.95f;
    L.numPoint = 1;                                   // light inside the tunnel
    L.pointPos[0] = Vec3(0, 23, 30);
    L.pointColor[0] = Vec3(6, 12, 16);
}

void Scene3_Underwater::drawOpaque(Renderer& r) {
    Mat4 I;
    r.drawMesh(wall, I, Mat::concrete(1.0f, Vec3(0.42f, 0.46f, 0.47f)));
    r.drawMesh(column, I, Mat::concrete(1.0f, Vec3(0.55f, 0.6f, 0.6f)));
    r.drawMesh(hood, I, Mat::concrete(1.0f, Vec3(0.16f, 0.18f, 0.2f)));
    r.drawMesh(tunnel, I, Mat::concrete(1.0f, Vec3(0.35f, 0.38f, 0.4f)));
    r.drawMesh(glowEnd, I, Mat::glow(Vec3(0.4f, 1.4f, 2.0f)));
    rack.draw(r, I);
    Material rock;
    rock.color = Vec3(0.3f, 0.32f, 0.3f);
    rock.pattern = PAT_ROCK;
    rock.patternScale = 0.6f;
    rock.specular = 0.1f;
    r.drawMesh(rocks, I, rock);
    Material sand = rock;
    sand.color = Vec3(0.32f, 0.33f, 0.3f);
    sand.pattern = PAT_SOIL;
    sand.gouraud = true;
    r.drawMesh(seabed, I, sand);
}

void Scene3_Underwater::drawTransparent(Renderer& r) {
    WaterStyle w;
    w.mode = 0;
    w.ripple = 0.25f;
    w.rippleStrength = 2.0f;
    r.drawWater(surface, Mat4(), w);

    Material beam;
    beam.pattern = PAT_BEAM;
    beam.emissive = Vec3(0.5f, 0.85f, 0.9f);
    beam.alpha = 0.22f;
    beam.additive = true;
    beam.castShadow = false;
    beam.twoSided = true;
    r.drawMesh(shafts, Mat4(), beam);
    dust.draw(r);
}

void Scene3_Underwater::telemetry(std::vector<Telemetry>& out) {
    char buf[64];
    float depth = SURFACE_Y - camPos.y;
    std::snprintf(buf, sizeof(buf), "%.1f M", depth);
    out.push_back({"DEPTH", buf});
    std::snprintf(buf, sizeof(buf), "%.2f BAR", 1.0f + depth / 10.2f);
    out.push_back({"PRESSURE", buf});
    std::snprintf(buf, sizeof(buf), "%.1f M", std::fmax(-camPos.z - 2.4f, 0.0f));
    out.push_back({"DISTANCE TO RACK", buf});
    out.push_back({"CLEAR GAP BETWEEN BARS", "0.76 M"});
    out.push_back({"APPROACH SPEED", "0.9 M/S"});
}

void Scene3_Underwater::buttons(std::vector<Button>& out) {
    out.push_back({"SWIM THROUGH THE RACK >", 1, "ENTER", true});
}
