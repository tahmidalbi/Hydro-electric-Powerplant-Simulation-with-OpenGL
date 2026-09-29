#include "Cascade.h"
#include "../core/Noise.h"
#include <algorithm>
#include <cmath>

namespace {

using glm::vec2;
using glm::vec3;

float smooth01(float t) { t = std::clamp(t, 0.0f, 1.0f); return t * t * (3.0f - 2.0f * t); }

// Catmull-Rom through the profile points; t runs 0 .. n-1.
vec2 profileAt(const std::vector<vec2>& p, float t) {
    int n = (int)p.size();
    int i = std::clamp((int)t, 0, n - 2);
    float u = t - i;
    vec2 p0 = p[std::max(i - 1, 0)], p1 = p[i], p2 = p[i + 1], p3 = p[std::min(i + 2, n - 1)];
    return 0.5f * (2.0f * p1 + (p2 - p0) * u + (2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3) * u * u
                   + (3.0f * p1 - p0 - 3.0f * p2 + p3) * u * u * u);
}

}

MeshData buildWaterSheet(const std::vector<vec2>& profile, float widthTop, float widthBottom, float lift, unsigned int seed) {
    const int NV = 56, NU = 24;   // samples along / across the flow
    const float tMax = float(profile.size() - 1);
    const float sd = float(seed % 97);
    const vec3 glass(0.50f, 0.74f, 0.86f), foam(0.96f, 0.98f, 1.0f);

    MeshData d;
    std::vector<vec3> outward;
    for (int j = 0; j <= NV; ++j) {
        float v = float(j) / NV;
        vec2 p = profileAt(profile, v * tMax);
        vec2 tan = glm::normalize(profileAt(profile, std::min(v * tMax + 0.03f, tMax)) -
                                  profileAt(profile, std::max(v * tMax - 0.03f, 0.0f)));
        vec2 nrm(-tan.y, tan.x);   // (z, y): away from the concrete
        float width = glm::mix(widthTop, widthBottom, std::pow(v, 1.5f));

        for (int i = 0; i <= NU; ++i) {
            float u = float(i) / NU;
            float x = (u - 0.5f) * width;
            float edge = std::abs(u - 0.5f) * 2.0f;

            // Streaks run along the flow: noise changes fast across, slowly along.
            float streak = Noise::fbm(x * 1.7f + sd, v * 2.5f, 3, (int)seed);
            float ripple = (Noise::fbm(x * 0.8f + sd * 3.0f, v * 9.0f, 2, (int)seed + 5) - 0.5f) * 0.3f;
            float off = (lift + ripple + 0.25f * v) * (1.0f - 0.6f * std::pow(edge, 4.0f));

            vec3 pos(x, p.y + nrm.y * off, p.x + nrm.x * off);
            float white = 0.2f + 0.8f * std::pow(v, 0.8f) + (streak - 0.5f) * 1.8f + 0.5f * smooth01((edge - 0.7f) / 0.3f);
            d.vertices.push_back({pos, vec3(0.0f), glm::mix(glass, foam, std::clamp(white, 0.0f, 1.0f))});
            outward.push_back(vec3(0.0f, nrm.y, nrm.x));
        }
    }
    for (int j = 0; j < NV; ++j) {
        for (int i = 0; i < NU; ++i) {
            unsigned int a = (unsigned int)(j * (NU + 1) + i), c = a + NU + 1;
            d.indices.insert(d.indices.end(), {a, c, a + 1, a + 1, c, c + 1});
        }
    }
    computeSmoothNormals(d.vertices, d.indices);
    for (size_t k = 0; k < d.vertices.size(); ++k)
        if (glm::dot(d.vertices[k].normal, outward[k]) < 0.0f) d.vertices[k].normal = -d.vertices[k].normal;
    return d;
}
