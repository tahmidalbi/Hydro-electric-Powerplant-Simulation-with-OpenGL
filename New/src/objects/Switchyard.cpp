#include "Switchyard.h"
#include "Materials.h"

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
            b.beam(corner(k, y1), corner(n, y1), t, t);          // horizontal
            b.beam(corner(k, y0), corner(n, y1), t * 0.7f, t * 0.7f);   // X bracing
            b.beam(corner(n, y0), corner(k, y1), t * 0.7f, t * 0.7f);
        }
    }
}

void Switchyard::build() {
    MeshBuilder b;
    b.box(Vec3(72, -3, 44), Vec3(116, 5, 96));          // raised platform
    platform = b.build();

    // gantry towers + beams, and two big transmission pylons
    b.clear();
    Vec3 towers[4] = {Vec3(80, 5, 54), Vec3(80, 5, 70), Vec3(100, 5, 54), Vec3(100, 5, 70)};
    for (Vec3 p : towers) latticeTower(b, p, 1.3f, 0.7f, 16.0f, 6);
    b.beam(Vec3(80, 20.5f, 54), Vec3(80, 20.5f, 70), 0.9f, 0.9f);
    b.beam(Vec3(100, 20.5f, 54), Vec3(100, 20.5f, 70), 0.9f, 0.9f);
    Vec3 pylons[2] = {Vec3(110, 5, 85), Vec3(140, 40, 120)};
    for (Vec3 p : pylons) {
        latticeTower(b, p, 3.0f, 0.9f, 34.0f, 9);
        for (int k = 0; k < 3; k++) {
            float y = p.y + 22 + k * 5.0f;
            b.beam(p + Vec3(-7, y - p.y, 0), p + Vec3(7, y - p.y, 0), 0.35f, 0.35f);
        }
    }
    steelwork = b.build();

    // transformers with cooling fins
    b.clear();
    MeshBuilder ins;
    for (int k = 0; k < 4; k++) {
        float x = 84 + k * 4.5f, z = 60;
        b.box(Vec3(x - 1.5f, 5, z - 2), Vec3(x + 1.5f, 8.5f, z + 2));
        for (int f = 0; f < 6; f++) b.box(Vec3(x - 1.8f, 5.3f, z - 1.6f + f * 0.65f), Vec3(x + 1.8f, 8.0f, z - 1.5f + f * 0.65f));
        for (int i = 0; i < 3; i++) {
            float iz = z - 1.2f + i * 1.2f;
            for (int d = 0; d < 6; d++) {                         // stacked insulator discs
                ins.setTransform(translate(Vec3(x, 8.5f + d * 0.38f, iz)));
                ins.cylinder(0.28f, 0.18f, 0.3f, 12, true, true);
            }
        }
    }
    ins.resetTransform();
    transformers = b.build();
    insulators = ins.build();

    // wires: sagging lines made of short segments
    b.clear();
    auto wire = [&](Vec3 a, Vec3 c, float sag) {
        Vec3 prev = a;
        for (int i = 1; i <= 16; i++) {
            float u = i / 16.0f;
            Vec3 p = mix(a, c, u) - Vec3(0, sag * 4 * u * (1 - u), 0);
            b.beam(prev, p, 0.06f, 0.06f);
            prev = p;
        }
    };
    for (int k = 0; k < 3; k++) {
        wire(Vec3(80, 20.0f, 58 + k * 4), Vec3(100, 20.0f, 58 + k * 4), 1.2f);
        wire(Vec3(110 - 7 + k * 7, 5 + 22 + 5, 85), Vec3(140 - 7 + k * 7, 40 + 22 + 5, 120), 3.0f);
    }
    wires = b.build();

    // fence
    b.clear();
    for (float x = 73; x <= 115; x += 2.5f) {
        b.box(Vec3(x, 5, 45), Vec3(x + 0.08f, 7, 45.08f));
        b.box(Vec3(x, 5, 95), Vec3(x + 0.08f, 7, 95.08f));
    }
    b.box(Vec3(73, 6.9f, 45), Vec3(115, 7.0f, 45.1f));
    b.box(Vec3(73, 6.9f, 95), Vec3(115, 7.0f, 95.1f));
    fence = b.build();
}

void Switchyard::draw(Renderer& r) {
    Mat4 I;
    r.drawMesh(platform, I, Mat::concrete(1.0f, Vec3(0.5f, 0.5f, 0.48f)));
    r.drawMesh(steelwork, I, Mat::steel(Vec3(0.6f, 0.6f, 0.6f)));
    r.drawMesh(transformers, I, Mat::paint(Vec3(0.38f, 0.42f, 0.4f)));
    r.drawMesh(insulators, I, Mat::plain(Vec3(0.55f, 0.35f, 0.25f), 0.6f, 60.0f));
    r.drawMesh(wires, I, Mat::plain(Vec3(0.15f, 0.15f, 0.15f)));
    r.drawMesh(fence, I, Mat::steel());
}
