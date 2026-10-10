#include "Rocks.h"
#include <algorithm>

void addRock(MeshBuilder& b, const Vec3& c, float r, unsigned int seed) {
    const int SEG = 14, RINGS = 9;
    Rng rng(seed);
    // a few random "bumps" give each rock its own shape
    Vec3 bumpDir[5];
    float bumpAmp[5];
    for (int i = 0; i < 5; i++) {
        bumpDir[i] = normalize(Vec3(rng.range(-1, 1), rng.range(-0.3f, 1), rng.range(-1, 1)));
        bumpAmp[i] = rng.range(-0.25f, 0.3f);
    }
    Vec3 stretch(rng.range(0.8f, 1.3f), rng.range(0.55f, 0.8f), rng.range(0.8f, 1.3f));
    unsigned int start = (unsigned int)b.verts.size();
    std::vector<Vec3> pos;
    for (int j = 0; j <= RINGS; j++) {
        float phi = PI * j / RINGS;
        for (int i = 0; i <= SEG; i++) {
            float th = 2 * PI * (i % SEG) / SEG;
            Vec3 d(std::sin(phi) * std::cos(th), std::cos(phi), std::sin(phi) * std::sin(th));
            float k = 1.0f;
            for (int q = 0; q < 5; q++) k += bumpAmp[q] * std::pow(std::fmax(dot(d, bumpDir[q]), 0.0f), 3.0f);
            k += (hash1((float)(seed % 97) + i * 7.1f + j * 13.7f) - 0.5f) * 0.12f;
            Vec3 p = d * k * r * stretch;
            if (p.y < -0.35f * r) p.y = -0.35f * r;     // flat bottom
            pos.push_back(p);
        }
    }
    // normals from neighbouring vertices (good enough for lumpy rocks)
    for (int j = 0; j <= RINGS; j++)
        for (int i = 0; i <= SEG; i++) {
            Vec3 p = pos[j * (SEG + 1) + i];
            Vec3 a = pos[j * (SEG + 1) + (i + 1) % (SEG + 1)] - pos[j * (SEG + 1) + (i + SEG - 1) % (SEG + 1)];
            Vec3 bb = pos[std::min(j + 1, RINGS) * (SEG + 1) + i] - pos[std::max(j - 1, 0) * (SEG + 1) + i];
            Vec3 n = normalize(cross(bb, a));
            if (dot(n, p) < 0) n = -n;
            if (length(n) < 0.5f) n = normalize(p);
            b.addVertex(c + p, n, Vec2((float)i / SEG * r * 4, (float)j / RINGS * r * 2));
        }
    for (int j = 0; j < RINGS; j++)
        for (int i = 0; i < SEG; i++) {
            unsigned int a = start + j * (SEG + 1) + i, d = a + SEG + 1;
            b.tri(a, d, a + 1);
            b.tri(a + 1, d, d + 1);
        }
}
