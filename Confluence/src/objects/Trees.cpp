#include "Trees.h"
#include "Layout.h"
#include "Terrain.h"
#include "../core/Material.h"
#include "../core/Noise.h"
#include "../core/Render.h"
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <cmath>
#include <vector>

namespace {

using glm::vec3;
using glm::mat4;

constexpr float PI = 3.14159265358979323846f;

float smooth01(float t) { t = std::clamp(t, 0.0f, 1.0f); return t * t * (3.0f - 2.0f * t); }
mat4 T(vec3 t) { return glm::translate(mat4(1.0f), t); }
mat4 S(vec3 s) { return glm::scale(mat4(1.0f), s); }

// Spruce of unit height: a trunk and `tiers` drooping skirts of branches.
// Each skirt's rim alternates branch tips and notches, so the silhouette is
// ragged rather than a smooth cone; colour runs from dark near the trunk to
// lighter needles at the tips.
MeshData spruce(Noise::Rng& rng, int tiers, int branches, float slenderness) {
    MeshData d;
    d.append(cylinderData(6), T({0.0f, 0.13f, 0.0f}) * S({0.045f, 0.26f, 0.045f}), vec3(0.27f, 0.19f, 0.12f));

    const vec3 inner(0.030f, 0.10f, 0.045f);
    for (int t = 0; t < tiers; ++t) {
        float f = float(t) / (tiers - 1);                          // 0 = bottom tier, 1 = top
        float yTip = 0.10f + f * 0.76f;                            // height of the branch tips
        float r = (0.24f * std::pow(1.0f - f, 0.9f) + 0.03f) * slenderness;
        float yTrunk = yTip + 0.12f + 0.08f * (1.0f - f);          // where the skirt meets the trunk
        vec3 tipColor = vec3(0.10f, 0.25f, 0.09f) * rng.range(0.85f, 1.15f) * (0.8f + 0.3f * f);

        unsigned int apex = (unsigned int)d.vertices.size();
        d.vertices.push_back({{0.0f, yTrunk, 0.0f}, {0.0f, 1.0f, 0.0f}, inner});
        const int rim = branches * 2;
        float phase = rng.range(0.0f, 2.0f * PI);
        for (int k = 0; k < rim; ++k) {
            float ang = phase + 2.0f * PI * k / rim + rng.range(-0.12f, 0.12f);
            bool tip = (k % 2) == 0;
            float rad = tip ? r * rng.range(0.8f, 1.2f) : r * 0.5f;
            float y = tip ? yTip - r * rng.range(0.05f, 0.25f) : yTip + r * 0.25f;   // tips droop
            vec3 n = glm::normalize(vec3(std::cos(ang) * 0.85f, 0.6f, std::sin(ang) * 0.85f));
            d.vertices.push_back({{std::cos(ang) * rad, y, std::sin(ang) * rad}, n, tip ? tipColor : glm::mix(inner, tipColor, 0.4f)});
        }
        for (int k = 0; k < rim; ++k)
            d.indices.insert(d.indices.end(), {apex, apex + 1 + k, apex + 1 + (k + 1) % rim});
    }
    // Leader shoot.
    d.append(coneData(6), T({0.0f, 0.93f, 0.0f}) * S({0.05f, 0.16f, 0.05f}), vec3(0.09f, 0.22f, 0.08f));
    return d;
}

// Rounded broadleaf bush/tree of unit height: a few lumpy foliage masses.
MeshData broadleaf(Noise::Rng& rng) {
    MeshData d;
    int masses = 5 + (int)(rng.next() * 3.0f);
    for (int i = 0; i < masses; ++i) {
        MeshData blob = sphereData(6, 9);
        float seed = rng.range(0.0f, 50.0f);
        for (Vertex& v : blob.vertices) {
            vec3 q = v.position * 2.0f;
            v.position = q * (0.8f + 0.35f * Noise::fbm(q.x * 1.6f + seed, q.z * 1.6f + q.y, 2, 13));
        }
        computeSmoothNormals(blob.vertices, blob.indices);
        vec3 c(rng.range(-0.22f, 0.22f), rng.range(0.45f, 0.72f), rng.range(-0.22f, 0.22f));
        float r = rng.range(0.20f, 0.30f);
        d.append(blob, T(c) * S(vec3(r)));
    }
    // Shade the foliage: darker underneath, sunlit yellow-green on top.
    for (Vertex& v : d.vertices) {
        float up = smooth01((v.position.y - 0.2f) / 0.75f);
        v.color = glm::mix(vec3(0.09f, 0.21f, 0.06f), vec3(0.32f, 0.50f, 0.13f), up);
    }
    d.append(cylinderData(6), T({0.0f, 0.18f, 0.0f}) * S({0.06f, 0.36f, 0.06f}), vec3(0.30f, 0.22f, 0.14f));
    return d;
}

float distToSegment(glm::vec2 p, glm::vec2 a, glm::vec2 b) {
    glm::vec2 ab = b - a;
    float t = glm::clamp(glm::dot(p - a, ab) / glm::dot(ab, ab), 0.0f, 1.0f);
    return glm::length(p - (a + ab * t));
}

// Places that must stay clear: the dam and powerhouse, the viewer's
// immediate foreground, the solar array, the turbine pads, the fence line,
// and the hillside between the viewer and the dam (trees there would hide it).
bool keepClear(float x, float z, float ground) {
    glm::vec2 p(x, z);
    float along = Layout::damAlong(x, z), side = Layout::damSide(x, z);
    if (along > -4.0f && along < Layout::DAM_LENGTH + 4.0f && side > -6.0f && side < 26.0f) return true;
    if (glm::length(p) < 14.0f) return true;
    if (glm::length(p - Layout::SOLAR_XZ) < 10.0f) return true;
    if (glm::length(p - Layout::TURBINE1_XZ) < 6.0f || glm::length(p - Layout::TURBINE2_XZ) < 6.0f) return true;
    if (distToSegment(p, {-6.0f, -25.0f}, {7.0f, -31.0f}) < 2.5f) return true;
    // In the dam's direction, only trees that stay below the line of sight to
    // the dam's foot (the eye is 23 up; that line drops ~0.28 per unit).
    float bearing = glm::degrees(std::atan2(x, -z));   // 0 = straight ahead, + = right
    bool towardsDam = z < -20.0f && z > -95.0f && bearing > -9.0f && bearing < 24.0f;
    if (towardsDam && ground + 7.0f > 23.0f - glm::length(p) * 0.28f) return true;
    return false;
}

MeshData build() {
    Noise::Rng rng(1234);

    // Prototypes: detailed spruces for the near slopes, lighter ones for the
    // distant shores (where fog hides the detail), and broadleaf bushes.
    std::vector<MeshData> nearSpruce, farSpruce, bushes;
    for (int i = 0; i < 6; ++i) nearSpruce.push_back(spruce(rng, 9, 7, rng.range(0.85f, 1.1f)));
    for (int i = 0; i < 4; ++i) farSpruce.push_back(spruce(rng, 5, 5, rng.range(0.9f, 1.1f)));
    for (int i = 0; i < 5; ++i) bushes.push_back(broadleaf(rng));

    MeshData d;
    auto place = [&](const MeshData& proto, float x, float z, float ground, float height, float width) {
        vec3 tint = vec3(rng.range(0.85f, 1.15f), rng.range(0.9f, 1.12f), rng.range(0.85f, 1.1f));
        d.append(proto, T({x, ground - 0.25f, z}) * glm::rotate(mat4(1.0f), rng.range(0.0f, 2.0f * PI), vec3(0, 1, 0))
                        * S({width, height, width}), tint);
    };

    struct Region { float x0, x1, z0, z1, spacing, hMin, hMax, density, minNormalY, bushShare; bool detailed; };
    const Region regions[] = {
        {-80.0f, 14.0f, -170.0f, 12.0f, 2.8f, 5.0f, 9.5f, 0.95f, 0.55f, 0.22f, true},    // left hillside
        {20.0f, 75.0f, -90.0f, 10.0f, 3.2f, 5.0f, 9.0f, 0.90f, 0.78f, 0.25f, true},      // right cliff top
        {30.0f, 240.0f, -330.0f, -95.0f, 5.0f, 7.0f, 11.0f, 0.97f, 0.70f, 0.0f, false},   // far right shore
        {-220.0f, -40.0f, -330.0f, -150.0f, 5.5f, 7.0f, 11.0f, 0.90f, 0.70f, 0.0f, false}, // far left shore
    };
    for (const Region& g : regions) {
        for (float z = g.z0; z < g.z1; z += g.spacing) {
            for (float x = g.x0; x < g.x1; x += g.spacing) {
                float px = x + rng.range(-0.45f, 0.45f) * g.spacing;
                float pz = z + rng.range(-0.45f, 0.45f) * g.spacing;
                float clump = Noise::fbm(px * 0.03f, pz * 0.03f, 3, 77);
                if (rng.next() > g.density * smooth01((clump - 0.35f) * 4.0f)) continue;
                float h = terrainHeight(px, pz);
                if (keepClear(px, pz, h)) continue;
                float water = Layout::damSide(px, pz) < 0.0f ? Layout::LAKE_LEVEL : Layout::RIVER_LEVEL;
                if (h < water + 1.0f || terrainNormal(px, pz).y < g.minNormalY) continue;

                if (rng.next() < g.bushShare) {
                    float size = rng.range(2.0f, 3.8f);
                    place(bushes[(size_t)(rng.next() * bushes.size()) % bushes.size()], px, pz, h, size, size * rng.range(0.9f, 1.3f));
                } else {
                    float height = rng.range(g.hMin, g.hMax);
                    const auto& protos = g.detailed ? nearSpruce : farSpruce;
                    place(protos[(size_t)(rng.next() * protos.size()) % protos.size()], px, pz, h, height, height * rng.range(0.9f, 1.1f));
                }
            }
        }
    }
    return d;
}

}

Trees::Trees() : mesh(build()) {}

void Trees::draw(const Shader& shader) const {
    drawMesh(shader, mesh, mat4(1.0f), Materials::FOLIAGE);
}
