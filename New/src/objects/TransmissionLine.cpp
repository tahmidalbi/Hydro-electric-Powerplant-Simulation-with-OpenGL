#include "TransmissionLine.h"
#include "Switchyard.h"
#include "Terrain.h"
#include "Materials.h"

static const float LINE_X = 116.0f, FIRST_Z = 140.0f, SPAN = 62.0f;
static const float ARM_H[3] = {26.0f, 31.0f, 36.0f};       // cross-arm heights
static const float ARM_W[3] = {7.0f, 6.0f, 5.0f};          // half width of each arm

// sagging wire between a and c (parabola ~ catenary), as a list of points
static void catenary(std::vector<Vec3>& out, const Vec3& a, const Vec3& c, float sag, int n, bool skipFirst) {
    for (int k = skipFirst ? 1 : 0; k <= n; k++) {
        float u = (float)k / n;
        out.push_back(mix(a, c, u) - Vec3(0, sag * 4 * u * (1 - u), 0));
    }
}

void TransmissionLine::build() {
    MeshBuilder t, ins;
    for (int i = 0; i < TOWERS; i++) {
        float z = FIRST_Z + i * SPAN;
        Vec3 base(LINE_X, Terrain::height(LINE_X, z), z);
        bases[i] = base;
        Switchyard::latticeTower(t, base, 4.0f, 1.3f, ARM_H[2] + 1.0f, 9);
        // cross arms (along x, across the line) and the earth-wire peak
        for (int a = 0; a < 3; a++) {
            Vec3 c = base + Vec3(0, ARM_H[a], 0);
            t.beam(c - Vec3(ARM_W[a], 0, 0.5f), c + Vec3(ARM_W[a], 0, -0.5f), 0.25f, 0.25f);
            t.beam(c - Vec3(ARM_W[a], 0, -0.5f), c + Vec3(ARM_W[a], 0, 0.5f), 0.25f, 0.25f);
            t.beam(c - Vec3(ARM_W[a], 0, 0), c + Vec3(-1.3f, 2.0f, 0), 0.15f, 0.15f);
            t.beam(c + Vec3(ARM_W[a], 0, 0), c + Vec3(1.3f, 2.0f, 0), 0.15f, 0.15f);
            for (int s = -1; s <= 1; s += 2)        // insulator strings at both arm tips
                for (int d = 0; d < 9; d++) {
                    ins.setTransform(translate(c + Vec3(s * ARM_W[a], -0.25f - d * 0.24f, 0)));
                    ins.cylinder(0.2f, 0.12f, 0.12f, 10, true, true);
                }
        }
        Vec3 top = base + Vec3(0, ARM_H[2] + 1.0f, 0);
        t.beam(top + Vec3(-1.3f, 0, 0), top + Vec3(0, 4.0f, 0), 0.18f, 0.18f);
        t.beam(top + Vec3(1.3f, 0, 0), top + Vec3(0, 4.0f, 0), 0.18f, 0.18f);
    }
    ins.resetTransform();
    towers = t.build();
    insulators = ins.build();

    // conductors: circuit A (left, from the main bay) and circuit B (right) + earth wire
    MeshBuilder w;
    glow.begin();
    for (int c = 0; c < 7; c++) {
        std::vector<Vec3> pts;
        for (int i = 0; i < TOWERS; i++) {
            Vec3 attach;
            if (c < 6) {
                int a = c % 3, side = c < 3 ? -1 : 1;
                attach = bases[i] + Vec3(side * ARM_W[a], ARM_H[a] - 2.4f, 0);
            } else {
                attach = bases[i] + Vec3(0, ARM_H[2] + 5.0f, 0);
            }
            if (i == 0) {
                Vec3 start = c < 3 ? YardDims::lineExit(c) : (c < 6 ? YardDims::lineExit(c - 3) + Vec3(0, 0, 16.0f) : attach + Vec3(0, -2, -30));
                catenary(pts, start, attach, 2.0f, 24, false);
            } else {
                Vec3 prev = pts.back();
                catenary(pts, prev, attach, 3.5f, 20, true);
            }
        }
        for (size_t k = 1; k < pts.size(); k++) w.beam(pts[k - 1], pts[k], c < 6 ? 0.07f : 0.05f, c < 6 ? 0.07f : 0.05f);
        if (c < 6) glow.add(pts, 0.11f);
        if (c == 0) {
            guide = pts;
            lineLength = 0;
            for (size_t k = 1; k < pts.size(); k++) lineLength += length(pts[k] - pts[k - 1]);
        }
    }
    glow.finish();
    wires = w.build();
}

Vec3 TransmissionLine::pointAlong(float s) const {
    float acc = 0;
    for (size_t k = 1; k < guide.size(); k++) {
        float seg = length(guide[k] - guide[k - 1]);
        if (acc + seg >= s) return mix(guide[k - 1], guide[k], (s - acc) / seg);
        acc += seg;
    }
    return guide.back();
}

void TransmissionLine::draw(Renderer& r) {
    Mat4 I;
    r.drawMesh(towers, I, Mat::steel(Vec3(0.6f, 0.61f, 0.62f)));
    r.drawMesh(insulators, I, Mat::plain(Vec3(0.4f, 0.19f, 0.11f), 0.8f, 80.0f));
    r.drawMesh(wires, I, Mat::plain(Vec3(0.25f, 0.25f, 0.26f), 0.5f, 40.0f));
}

void TransmissionLine::drawGlow(Renderer& r, float front, float strength, float speed) {
    glow.draw(r, Mat4(), Vec3(0.6f, 0.9f, 2.4f), strength, speed, 6.0f, front);
}
