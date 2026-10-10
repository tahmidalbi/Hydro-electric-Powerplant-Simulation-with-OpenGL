#include "SpillwayWater.h"
#include "Dam.h"

using namespace DamDims;

void SpillwayWater::build() {
    // 1) chute floor from the gates downwards, resampled every 0.6 m of length
    std::vector<Vec2> prof = chuteProfile();
    std::vector<Vec2> pts;
    for (Vec2 v : prof) if (v.x >= GATE_Z) pts.push_back(v);
    pts.insert(pts.begin(), Vec2(GATE_Z, chuteY(GATE_Z)));
    std::vector<Vec2> even;
    std::vector<float> evenS;
    even.push_back(pts[0]);
    evenS.push_back(0);
    float s = 0, next = 0.6f;
    for (size_t i = 1; i < pts.size(); i++) {
        Vec2 a = pts[i - 1], c = pts[i];
        float seg = std::sqrt((c.x - a.x) * (c.x - a.x) + (c.y - a.y) * (c.y - a.y));
        while (s + seg >= next) {
            float u = (next - s) / seg;
            even.push_back(Vec2(mixf(a.x, c.x, u), mixf(a.y, c.y, u)));
            evenS.push_back(next);
            next += 0.6f;
        }
        s += seg;
    }
    length = evenS.back();

    // 2) one sheet per bay: floor + normal * thickness (thick at the crest, thin when fast)
    for (int bay = 0; bay < BAYS; bay++) {
        float x0 = bayX0(bay) + 0.02f, x1 = bayX0(bay) + BAY_W - 0.02f;
        const int NX = 12;
        MeshBuilder b;
        for (size_t i = 0; i < even.size(); i++) {
            size_t i0 = i > 0 ? i - 1 : i, i1 = i + 1 < even.size() ? i + 1 : i;
            float dz = even[i1].x - even[i0].x, dy = even[i1].y - even[i0].y;
            Vec3 n = normalize(Vec3(0, dz, -dy));
            float th = 0.75f + 2.4f * std::exp(-evenS[i] / 4.0f);
            Vec3 floorP(0, even[i].y, even[i].x);
            for (int k = 0; k <= NX; k++) {
                float x = mixf(x0, x1, (float)k / NX);
                Vec3 p = floorP + n * th;
                p.x = x;
                b.addVertex(p, n, Vec2(x - x0, evenS[i]));
            }
            if (bay == 0) { profS.push_back(evenS[i]); profY.push_back(even[i].y + th); }
        }
        for (size_t i = 0; i + 1 < even.size(); i++)
            for (int k = 0; k < NX; k++) {
                unsigned int a = (unsigned int)(i * (NX + 1) + k), c = a + NX + 1;
                b.tri(a, c, a + 1);
                b.tri(a + 1, c, c + 1);
            }
        sheet[bay] = b.build();
    }
}

// free fall:  v = sqrt(v0^2 + 2 g * drop)
float SpillwayWater::speedAt(float s) const {
    float y = profY.back();
    for (size_t i = 1; i < profS.size(); i++)
        if (profS[i] >= s) { y = profY[i]; break; }
    float drop = std::fmax(profY[0] - y, 0.0f);
    return std::sqrt(4.0f + 2.0f * 9.81f * drop);
}

void SpillwayWater::open() {
    for (const Slug& s : slugs) if (s.feeding) return;
    slugs.push_back({0.0f, 0.0f, true});
}
void SpillwayWater::close() {
    for (Slug& s : slugs) s.feeding = false;
}
void SpillwayWater::fillInstantly() {
    slugs.clear();
    slugs.push_back({0.0f, length + 5.0f, true});
}
bool SpillwayWater::reachesBottom() const {
    for (const Slug& s : slugs) if (s.front >= length - 0.5f && s.tail < length - 0.5f) return true;
    return false;
}

void SpillwayWater::update(float dt, ParticleSystem& spray, ParticleSystem& mist) {
    for (Slug& s : slugs) {
        s.front = std::fmin(s.front + speedAt(s.front) * dt * timeScale, length + 5.0f);
        if (!s.feeding) s.tail = std::fmin(s.tail + speedAt(s.tail) * dt * timeScale, length + 5.0f);
    }
    for (size_t i = 0; i < slugs.size();)
        if (slugs[i].tail >= length) slugs.erase(slugs.begin() + i); else i++;

    // spray and mist where the water shoots into the river
    if (reachesBottom()) {
        float z = 54.0f, y = profY.back();
        for (int bay = 0; bay < BAYS; bay++) {
            for (int k = 0; k < 7; k++) {
                Particle p;
                p.pos = Vec3(bayX0(bay) + rng.range(0.5f, BAY_W - 0.5f), y, z + rng.range(-0.5f, 1.0f));
                p.vel = Vec3(rng.range(-2.5f, 2.5f), rng.range(2.0f, 7.5f), rng.range(6.0f, 15.0f));
                p.size = rng.range(0.25f, 0.6f);
                p.grow = 1.2f;
                p.maxLife = rng.range(0.8f, 1.6f);
                p.gravity = -9.81f;
                p.drag = 0.6f;
                p.color = Vec3(0.85f, 0.9f, 0.95f);
                p.alpha = 0.75f;
                spray.emit(p);
            }
            for (int k = 0; k < 2; k++) {
                Particle m;
                m.pos = Vec3(bayX0(bay) + rng.range(0.0f, BAY_W), y + rng.range(0.0f, 2.0f), z + rng.range(2.0f, 10.0f));
                m.vel = Vec3(rng.range(-1.0f, 1.0f), rng.range(1.0f, 3.0f), rng.range(2.0f, 6.0f));
                m.size = rng.range(1.5f, 3.0f);
                m.grow = 2.2f;
                m.maxLife = rng.range(2.5f, 4.5f);
                m.gravity = 0.25f;
                m.drag = 0.5f;
                m.color = Vec3(0.8f, 0.85f, 0.9f);
                m.alpha = 0.32f;
                mist.emit(m);
            }
        }
    }
}

void SpillwayWater::draw(Renderer& r, float time) {
    (void)time;
    WaterStyle w;
    w.mode = 1;
    w.deep = Vec3(0.02f, 0.12f, 0.2f);
    w.shallow = Vec3(0.12f, 0.45f, 0.62f);
    w.alpha = 0.93f;
    w.foam = 0.75f;
    w.flowSpeed = 14.0f;
    w.ripple = 0.9f;
    w.rippleStrength = 1.2f;
    w.sheetWidth = BAY_W - 0.04f;
    for (const Slug& s : slugs) {
        w.front = s.front;
        w.tail = s.feeding ? -1e3f : s.tail;
        for (int bay = 0; bay < BAYS; bay++) r.drawWater(sheet[bay], Mat4(), w);
    }
}
