#include "Debris.h"
#include "Materials.h"

// A tapered wooden piece from a to b (radius ra -> rb) with a ball at the joint.
static void limb(MeshBuilder& m, const Vec3& a, const Vec3& b, float ra, float rb) {
    m.setTransform(alignY(a, b - a));
    m.cylinder(ra, rb, length(b - a), 12, false, true);
    m.setTransform(translate(a));
    m.sphere(ra * 1.02f, 10, 6);
    m.resetTransform();
}

void Debris::build() {
    MeshBuilder m;
    // ---- drift-wood branch: bent trunk + forks + root stubs (trunk along local x)
    Vec3 P0(-3.1f, -0.1f, 0.0f), P1(-1.3f, 0.12f, 0.1f), P2(0.7f, 0.0f, -0.1f), P3(3.0f, 0.22f, 0.05f);
    limb(m, P0, P1, 0.36f, 0.3f);
    limb(m, P1, P2, 0.3f, 0.22f);
    limb(m, P2, P3, 0.22f, 0.1f);
    limb(m, P1, Vec3(-0.1f, 1.5f, 0.5f), 0.17f, 0.07f);
    limb(m, Vec3(-0.1f, 1.5f, 0.5f), Vec3(0.2f, 2.3f, 0.6f), 0.07f, 0.03f);
    limb(m, P2, Vec3(1.7f, -1.3f, 0.35f), 0.15f, 0.06f);
    limb(m, P2, Vec3(1.9f, 1.25f, -0.45f), 0.14f, 0.05f);
    limb(m, Vec3(1.9f, 1.25f, -0.45f), Vec3(2.6f, 1.9f, -0.5f), 0.05f, 0.02f);
    limb(m, P0, Vec3(-3.8f, 0.6f, 0.3f), 0.26f, 0.08f);       // root stubs
    limb(m, P0, Vec3(-3.7f, -0.7f, -0.3f), 0.24f, 0.07f);
    limb(m, P0, Vec3(-3.6f, 0.1f, -0.7f), 0.2f, 0.06f);
    branch = m.build();

    // ---- short log (along x)
    m.clear();
    m.setTransform(translate(Vec3(-0.8f, 0, 0)) * rotateZ(-PI * 0.5f));
    m.cylinder(0.2f, 0.18f, 1.6f, 14, true, true);
    m.resetTransform();
    log = m.build();

    // ---- leaf: flat ellipse in the XZ plane, uv.x across (vein at 0.5)
    m.clear();
    {
        unsigned int c = m.addVertex(Vec3(0, 0, 0), Vec3(0, 1, 0), Vec2(0.5f, 0.5f));
        for (int i = 0; i <= 12; i++) {
            float a = i * 2 * PI / 12;
            float x = std::cos(a) * 0.22f, z = std::sin(a) * 0.1f * (1.0f - 0.3f * std::cos(a));
            m.addVertex(Vec3(x, std::fabs(std::sin(a)) * 0.02f, z), Vec3(0, 1, 0), Vec2(0.5f + z / 0.2f, 0.5f + x / 0.44f));
        }
        for (int i = 0; i < 12; i++) m.tri(c, c + 1 + i, c + 2 + i);
    }
    leaf = m.build();

    // ---- crumpled plastic: a few random triangles
    m.clear();
    {
        Rng r(5);
        for (int i = 0; i < 7; i++) {
            Vec3 a(r.range(-0.3f, 0.3f), r.range(-0.08f, 0.08f), r.range(-0.25f, 0.25f));
            Vec3 b = a + Vec3(r.range(-0.25f, 0.25f), r.range(-0.1f, 0.1f), r.range(-0.25f, 0.25f));
            Vec3 c = a + Vec3(r.range(-0.25f, 0.25f), r.range(-0.1f, 0.1f), r.range(-0.25f, 0.25f));
            Vec3 n = normalize(cross(b - a, c - a));
            unsigned int k = m.addVertex(a, n, Vec2(0, 0));
            m.addVertex(b, n, Vec2(1, 0));
            m.addVertex(c, n, Vec2(0, 1));
            m.tri(k, k + 1, k + 2);
        }
    }
    plastic = m.build();

    // ---- straw / grass clump
    m.clear();
    {
        Rng r(9);
        for (int i = 0; i < 16; i++) {
            Vec3 a(r.range(-0.15f, 0.15f), r.range(-0.1f, 0.1f), r.range(-0.15f, 0.15f));
            Vec3 d = normalize(Vec3(r.range(-1, 1), r.range(-0.4f, 0.4f), r.range(-1, 1)));
            m.beam(a - d * 0.35f, a + d * 0.35f, 0.025f, 0.025f);
        }
    }
    straw = m.build();
}

void Debris::reset() {
    pieces.clear();
    passed = 0;
    stuckSlots = 0;
}

// Each wave sends a group of trash down the channel (wave 0 = the big branch).
void Debris::spawnWave(int wave) {
    auto add = [&](DebrisType type, float x, float z) {
        DebrisPiece p;
        p.type = type;
        p.pos = Vec3(x, waterY - 0.3f, z);
        p.vel = Vec3(current, 0, 0);
        p.phase = rng.range(0, 6.28f);
        p.rot = Vec3(rng.range(0, 6.28f), rng.range(0, 6.28f), rng.range(-0.3f, 0.3f));
        p.spin = Vec3(rng.range(-0.6f, 0.6f), rng.range(-0.8f, 0.8f), rng.range(-0.3f, 0.3f));
        switch (type) {
            case DEB_BRANCH:
                p.rot = Vec3(0.15f, PI * 0.5f - 0.35f, 0.12f);
                p.spin = Vec3(0, 0.02f, 0);
                p.floatY = waterY - 0.5f;
                p.extent = 1.9f;
                p.scale = 1.45f;
                p.color = Vec3(0.5f, 0.4f, 0.3f);
                p.stuckPos = Vec3(rackX - 3.4f, 4.9f, z);
                p.stuckRot = Vec3(0.25f, 0.15f, 0.75f);
                break;
            case DEB_LOG:
                p.floatY = waterY - rng.range(0.2f, 1.6f);
                p.extent = 0.55f;
                p.rot.z = 0;
                p.spin.z = 0;
                p.scale = rng.range(0.7f, 1.15f);
                p.color = Vec3(0.42f, 0.3f, 0.2f) * rng.range(0.8f, 1.1f);
                break;
            case DEB_LEAF: {
                float k = rng.next();
                p.color = k < 0.5f ? Vec3(0.25f, 0.42f, 0.1f) : (k < 0.8f ? Vec3(0.6f, 0.55f, 0.12f) : Vec3(0.45f, 0.25f, 0.1f));
                p.floatY = waterY - rng.range(0.05f, 2.5f);
                p.extent = 0.1f;
                p.scale = rng.range(0.8f, 1.6f);
                p.passes = rng.next() < 0.45f;   // some leaves are small enough to slip through
                p.spin = p.spin * 3.0f;
                break;
            }
            case DEB_PLASTIC:
                p.color = Vec3(0.85f, 0.88f, 0.9f);
                p.floatY = waterY - rng.range(0.1f, 2.0f);
                p.extent = 0.25f;
                p.scale = rng.range(0.8f, 1.4f);
                p.spin = p.spin * 2.0f;
                break;
            case DEB_STRAW:
                p.color = Vec3(0.62f, 0.5f, 0.22f);
                p.floatY = waterY - rng.range(0.6f, 3.5f);
                p.extent = 0.3f;
                break;
        }
        pieces.push_back(p);
    };
    float x0 = -16.0f - wave * 1.5f;
    if (wave == 0) {
        add(DEB_BRANCH, x0 + 2.0f, 1.0f);
        add(DEB_STRAW, x0 + 1.0f, 0.5f);
        add(DEB_STRAW, x0 + 4.0f, 2.0f);
    }
    int nLeaves = wave == 0 ? 18 : 14;
    for (int i = 0; i < nLeaves; i++) add(DEB_LEAF, x0 - rng.range(0, 10), rng.range(-4.5f, 4.5f));
    for (int i = 0; i < 4; i++) add(DEB_LOG, x0 - rng.range(1, 9), rng.range(-4.0f, 4.0f));
    for (int i = 0; i < 4; i++) add(DEB_PLASTIC, x0 - rng.range(0, 9), rng.range(-4.0f, 4.0f));
}

int Debris::stuckCount() const {
    int n = 0;
    for (const DebrisPiece& p : pieces) if (p.stuck) n++;
    return n;
}

void Debris::update(float dt, float time, ParticleSystem& splash) {
    for (DebrisPiece& p : pieces) {
        if (!p.stuck) {
            // current + buoyancy + bobbing
            float target = p.floatY + 0.12f * std::sin(time * 1.6f + p.phase);
            p.vel.x += (current - p.vel.x) * std::fmin(dt * 1.5f, 1.0f);
            p.vel.y += ((target - p.pos.y) * 2.5f - p.vel.y * 1.8f) * dt;
            p.vel.z += (-p.vel.z * 0.8f) * dt;
            p.pos += p.vel * dt;
            p.rot += p.spin * dt;

            float contactX = rackX - p.extent;
            if (!p.passes && p.pos.x >= contactX) {
                // hits the bars: stops, splashes, settles against the rack
                p.stuck = true;
                p.stuckTime = 0;
                p.vel = Vec3(0, 0, 0);
                if (p.type != DEB_BRANCH) {
                    int slot = stuckSlots++;
                    p.stuckPos = Vec3(contactX - 0.12f * (slot % 4), p.pos.y - rng.range(0.0f, 0.6f), p.pos.z);
                    p.stuckRot = p.rot;
                }
                int n = p.type == DEB_BRANCH ? 70 : 10;
                for (int k = 0; k < n; k++) {
                    Particle s;
                    s.pos = Vec3(rackX - 0.3f, waterY, p.pos.z + rng.range(-1.5f, 1.5f) * (p.type == DEB_BRANCH ? 1.6f : 0.4f));
                    s.vel = Vec3(rng.range(-2.5f, 0.5f), rng.range(1.5f, 5.0f), rng.range(-1.5f, 1.5f));
                    s.size = rng.range(0.05f, 0.14f);
                    s.maxLife = rng.range(0.6f, 1.3f);
                    s.gravity = -9.81f;
                    s.color = Vec3(0.9f, 0.95f, 1.0f);
                    s.alpha = 0.9f;
                    s.pos = parent.transformPoint(s.pos);       // into world space
                    s.vel = parent.transformDir(s.vel);
                    splash.emit(s);
                }
            }
            if (p.passes && std::fabs(p.pos.x - rackX) < 0.6f) {   // squeezes through the bars
                p.vel.x *= 0.92f;
                p.vel.z += rng.range(-1.0f, 1.0f) * dt * 4.0f;
            }
            if (p.passes && p.pos.x > rackX + 0.7f && p.pos.x - p.vel.x * dt <= rackX + 0.7f) passed++;
            if (p.passes && p.pos.x > rackX + 0.5f && sinkY > -1e8f) p.floatY = sinkY;   // drawn down into the intake
        } else {
            // resting against the rack: ease into the final pose, tremble in the current
            p.stuckTime += dt;
            float rate = p.type == DEB_BRANCH ? 1.8f : 4.0f;
            p.pos = mix(p.pos, p.stuckPos, std::fmin(dt * rate, 1.0f));
            p.rot = mix(p.rot, p.stuckRot, std::fmin(dt * rate * 0.8f, 1.0f));
            p.pos.y += 0.01f * std::sin(time * 3.0f + p.phase);
        }
    }
    // forget pieces that left the picture
    for (size_t i = 0; i < pieces.size();)
        if (pieces[i].pos.x > endX) pieces.erase(pieces.begin() + i); else i++;
}

void Debris::draw(Renderer& r) {
    for (const DebrisPiece& p : pieces) {
        Mat4 m = parent * translate(p.pos) * rotateY(p.rot.y) * rotateZ(p.rot.z) * rotateX(p.rot.x) * scale(Vec3(p.scale));
        Material mat;
        mat.color = p.color;
        mat.specular = 0.15f;
        mat.twoSided = true;
        switch (p.type) {
            case DEB_BRANCH: case DEB_LOG: mat.pattern = PAT_BARK; mat.patternScale = 1.0f; r.drawMesh(p.type == DEB_BRANCH ? branch : log, m, mat); break;
            case DEB_LEAF:   mat.pattern = PAT_LEAF; r.drawMesh(leaf, m, mat); break;
            case DEB_PLASTIC: mat.pattern = PAT_PLASTIC; mat.specular = 0.8f; mat.shininess = 60; r.drawMesh(plastic, m, mat); break;
            case DEB_STRAW:  r.drawMesh(straw, m, mat); break;
        }
    }
}
