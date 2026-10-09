#include "Switchyard.h"
#include "Materials.h"

using namespace YardDims;

void Switchyard::latticeTower(MeshBuilder& b, const Vec3& base, float bh, float th, float h, int levels) {
    auto corner = [&](int k, float y) {
        float half = mixf(bh, th, y / h);
        float sx = (k == 0 || k == 3) ? -1.0f : 1.0f, sz = (k < 2) ? -1.0f : 1.0f;
        return base + Vec3(sx * half, y, sz * half);
    };
    const float t = 0.16f;
    for (int k = 0; k < 4; k++) b.beam(corner(k, 0), corner(k, h), t * 1.6f, t * 1.6f);   // legs
    for (int l = 0; l < levels; l++) {
        float y0 = h * l / levels, y1 = h * (l + 1) / levels;
        for (int k = 0; k < 4; k++) {
            int n = (k + 1) % 4;
            b.beam(corner(k, y1), corner(n, y1), t, t);                // horizontal
            b.beam(corner(k, y0), corner(n, y1), t * 0.7f, t * 0.7f);  // X bracing
            b.beam(corner(n, y0), corner(k, y1), t * 0.7f, t * 0.7f);
        }
    }
}

// a stack of porcelain sheds (brown insulator) from y0 upwards
static void insulator(MeshBuilder& b, const Vec3& p, float height, float radius) {
    int n = (int)(height / 0.22f);
    for (int d = 0; d < n; d++) {
        b.setTransform(translate(p + Vec3(0, d * 0.22f, 0)));
        b.cylinder(radius, radius * 0.6f, 0.2f, 14, true, true);
    }
    b.resetTransform();
}

// one complete line bay at offset dz (the main bay has dz = 0)
static void bay(MeshBuilder& steel, MeshBuilder& por, MeshBuilder& brk, MeshBuilder& tnk, MeshBuilder& cond,
                MeshBuilder* green, MeshBuilder* red, float dz) {
    for (int p = 0; p < 3; p++) {
        float z = BAY_Z[p] + dz;
        // disconnector: two post insulators on a stand + the switch blade
        for (int s = 0; s < 2; s++) {
            float x = 96.0f + s * 2.2f;
            steel.box(Vec3(x - 0.15f, 5.0f, z - 0.15f), Vec3(x + 0.15f, 8.5f, z + 0.15f));
            insulator(por, Vec3(x, 8.5f, z), 2.4f, 0.22f);
        }
        steel.beam(Vec3(96.0f, 11.0f, z), Vec3(98.2f, 11.0f, z), 0.08f, 0.08f);
        // circuit breaker: frame + interrupter column + mechanism box
        steel.box(Vec3(BREAKER_X - 0.6f, 5.0f, z - 0.6f), Vec3(BREAKER_X + 0.6f, 5.3f, z + 0.6f));
        for (int l = 0; l < 4; l++)
            steel.box(Vec3(BREAKER_X - 0.55f + (l % 2) * 1.0f, 5.3f, z - 0.55f + (l / 2) * 1.0f),
                      Vec3(BREAKER_X - 0.45f + (l % 2) * 1.0f, 8.6f, z - 0.45f + (l / 2) * 1.0f));
        brk.box(Vec3(BREAKER_X - 0.5f, 8.6f, z - 0.5f), Vec3(BREAKER_X + 0.5f, 9.6f, z + 0.5f));
        brk.setTransform(translate(Vec3(BREAKER_X, 9.6f, z)));
        brk.cylinder(0.3f, 0.3f, 3.2f, 16, true, true);
        brk.resetTransform();
        if (green) green->box(Vec3(BREAKER_X - 0.52f, 9.15f, z - 0.15f), Vec3(BREAKER_X - 0.5f, 9.35f, z + 0.05f));
        if (red) red->box(Vec3(BREAKER_X - 0.52f, 8.85f, z - 0.15f), Vec3(BREAKER_X - 0.5f, 9.05f, z + 0.05f));
        // current transformer: insulator column + head tank
        steel.box(Vec3(106.85f, 5.0f, z - 0.15f), Vec3(107.15f, 8.0f, z + 0.15f));
        insulator(por, Vec3(107.0f, 8.0f, z), 3.2f, 0.3f);
        tnk.setTransform(translate(Vec3(107.0f, 11.3f, z)));
        tnk.cylinder(0.45f, 0.45f, 1.0f, 16, true, true);
        tnk.resetTransform();
        // surge arrester
        steel.box(Vec3(110.85f, 5.0f, z - 0.15f), Vec3(111.15f, 8.0f, z + 0.15f));
        insulator(por, Vec3(111.0f, 8.0f, z), 3.0f, 0.18f);
        // conductors joining the equipment (sagging tubes)
        Vec3 pts[6] = {Vec3(GANTRY_X, 16.0f, z), Vec3(96.0f, 11.0f, z), Vec3(BREAKER_X, 12.8f, z),
                       Vec3(107.0f, 12.3f, z), Vec3(111.0f, 11.1f, z), Vec3(LINE_GANTRY_X, 20.0f, z)};
        for (int i = 0; i < 5; i++) {
            Vec3 prev = pts[i];
            for (int k = 1; k <= 8; k++) {
                float u = k / 8.0f;
                Vec3 q = mix(pts[i], pts[i + 1], u) - Vec3(0, 0.6f * 4 * u * (1 - u), 0);
                cond.beam(prev, q, 0.06f, 0.06f);
                prev = q;
            }
        }
    }
    // gantries at both ends of the bay
    for (int g = 0; g < 2; g++) {
        float x = g == 0 ? GANTRY_X : LINE_GANTRY_X, h = g == 0 ? 11.4f : 15.4f;
        Switchyard::latticeTower(steel, Vec3(x, 5.0f, 56.0f + dz), 0.8f, 0.5f, h, 5);
        Switchyard::latticeTower(steel, Vec3(x, 5.0f, 68.0f + dz), 0.8f, 0.5f, h, 5);
        steel.beam(Vec3(x, 5.0f + h, 56.0f + dz), Vec3(x, 5.0f + h, 68.0f + dz), 0.7f, 0.7f);
    }
}

void Switchyard::build() {
    MeshBuilder b;
    b.box(Vec3(75, -3, 44), Vec3(124, 5, 124));            // gravel platform
    platform = b.build();

    MeshBuilder steel, por, brk, tnk, cond, green, red;
    bay(steel, por, brk, tnk, cond, &green, &red, 0.0f);
    bay(steel, por, brk, tnk, cond, nullptr, nullptr, 16.0f);   // two more bays: a busy yard
    bay(steel, por, brk, tnk, cond, nullptr, nullptr, 32.0f);
    steelwork = steel.build();
    porcelain = por.build();
    breakers = brk.build();
    tanks = tnk.build();
    conductors = cond.build();
    lampsGreen = green.build();
    lampsRed = red.build();

    // control building + fence
    b.clear();
    b.box(Vec3(80, 5, 104), Vec3(94, 10, 116));
    b.box(Vec3(79.8f, 10, 103.8f), Vec3(94.2f, 10.4f, 116.2f));
    building = b.build();
    b.clear();
    for (int k = 0; k < 4; k++) b.box(Vec3(79.95f, 7, 105.5f + k * 2.6f), Vec3(80.0f, 8.6f, 107.0f + k * 2.6f));
    windowsMesh = b.build();
    b.clear();
    for (float x = 76; x <= 123; x += 2.5f) {
        b.box(Vec3(x, 5, 45), Vec3(x + 0.08f, 7, 45.08f));
        b.box(Vec3(x, 5, 123), Vec3(x + 0.08f, 7, 123.08f));
    }
    b.box(Vec3(76, 6.9f, 45), Vec3(123, 7.0f, 45.1f));
    b.box(Vec3(76, 6.9f, 123), Vec3(123, 7.0f, 123.1f));
    fence = b.build();

    // glowing power along the main bay: before and after the breaker
    glowBefore.begin();
    glowAfter.begin();
    for (int p = 0; p < 3; p++) {
        float z = BAY_Z[p];
        Vec3 pts[6] = {Vec3(GANTRY_X, 16.0f, z), Vec3(96.0f, 11.0f, z), Vec3(BREAKER_X, 12.8f, z),
                       Vec3(107.0f, 12.3f, z), Vec3(111.0f, 11.1f, z), Vec3(LINE_GANTRY_X, 20.0f, z)};
        std::vector<Vec3> before, after;
        for (int i = 0; i < 5; i++)
            for (int k = 0; k <= 8; k++) {
                float u = k / 8.0f;
                Vec3 q = mix(pts[i], pts[i + 1], u) - Vec3(0, 0.6f * 4 * u * (1 - u), 0);
                if (i < 2) before.push_back(q); else after.push_back(q);
            }
        glowBefore.add(before, 0.07f);
        glowAfter.add(after, 0.07f);
    }
    glowBefore.finish();
    glowAfter.finish();
}

void Switchyard::draw(Renderer& r, float breakerClosed01) {
    Mat4 I;
    Material gravel;
    gravel.color = Vec3(0.42f, 0.41f, 0.4f);
    gravel.pattern = PAT_SOIL;
    gravel.patternScale = 2.0f;
    r.drawMesh(platform, I, gravel);
    r.drawMesh(steelwork, I, Mat::steel(Vec3(0.62f, 0.63f, 0.64f)));
    r.drawMesh(porcelain, I, Mat::plain(Vec3(0.4f, 0.19f, 0.11f), 0.8f, 80.0f));
    r.drawMesh(breakers, I, Mat::paint(Vec3(0.7f, 0.72f, 0.74f), 0.6f));
    r.drawMesh(tanks, I, Mat::paint(Vec3(0.75f, 0.76f, 0.78f), 0.6f));
    r.drawMesh(conductors, I, Mat::steel(Vec3(0.7f, 0.7f, 0.68f)));
    r.drawMesh(building, I, Mat::concrete(1.0f, Vec3(0.62f, 0.6f, 0.56f)));
    r.drawMesh(windowsMesh, I, Mat::glow(Vec3(1.6f, 1.7f, 1.8f)));
    r.drawMesh(fence, I, Mat::steel());
    // breaker status lamps: green = closed (connected), red = open
    r.drawMesh(lampsGreen, I, Mat::glow(Vec3(0.2f, 4.0f, 0.6f) * (0.08f + breakerClosed01)));
    r.drawMesh(lampsRed, I, Mat::glow(Vec3(4.0f, 0.3f, 0.2f) * (1.08f - breakerClosed01)));
}

void Switchyard::drawGlow(Renderer& r, float before, float after, float speed) {
    glowBefore.draw(r, Mat4(), Vec3(0.6f, 0.9f, 2.4f), before, speed, 1.6f);
    glowAfter.draw(r, Mat4(), Vec3(0.6f, 0.9f, 2.4f), after, speed, 1.6f);
}
