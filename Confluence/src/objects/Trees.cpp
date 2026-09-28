#include "Trees.h"
#include "Layout.h"
#include "Terrain.h"
#include "../core/Material.h"
#include "../core/Noise.h"
#include "../core/Render.h"
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <cmath>

namespace {

using glm::vec3;
using glm::mat4;

float smooth01(float t) { t = std::clamp(t, 0.0f, 1.0f); return t * t * (3.0f - 2.0f * t); }
mat4 T(vec3 t) { return glm::translate(mat4(1.0f), t); }
mat4 S(vec3 s) { return glm::scale(mat4(1.0f), s); }

// Conifer in tree-local space: trunk plus three stacked cones.
void addConifer(MeshData& d, const MeshData& cone, const MeshData& trunk, vec3 base, float h, float yaw, vec3 leaf, bool withTrunk) {
    mat4 root = glm::rotate(T(base), yaw, vec3(0, 1, 0));
    if (withTrunk)
        d.append(trunk, root * T({0.0f, 0.15f * h, 0.0f}) * S({0.09f * h, 0.3f * h, 0.09f * h}), vec3(0.30f, 0.20f, 0.12f));
    const float tiers[3][3] = {{0.18f, 0.66f, 0.30f}, {0.40f, 0.86f, 0.23f}, {0.62f, 1.00f, 0.15f}};   // y0, y1, radius (x h)
    for (int i = 0; i < 3; ++i) {
        float y0 = tiers[i][0] * h, y1 = tiers[i][1] * h, r = tiers[i][2] * h;
        d.append(cone, root * T({0.0f, (y0 + y1) * 0.5f, 0.0f}) * S({2.0f * r, y1 - y0, 2.0f * r}), leaf * (0.92f + 0.08f * i));
    }
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
    MeshData d;
    MeshData cone = coneData(10), trunk = cylinderData(6);
    Noise::Rng rng(1234);

    auto scatter = [&](float x0, float x1, float z0, float z1, float spacing, float hMin, float hMax, float density, bool trunks, float minNormalY) {
        for (float z = z0; z < z1; z += spacing) {
            for (float x = x0; x < x1; x += spacing) {
                float px = x + rng.range(-0.45f, 0.45f) * spacing;
                float pz = z + rng.range(-0.45f, 0.45f) * spacing;
                float clump = Noise::fbm(px * 0.03f, pz * 0.03f, 3, 77);
                if (rng.next() > density * smooth01((clump - 0.35f) * 4.0f)) continue;
                float h = terrainHeight(px, pz);
                if (keepClear(px, pz, h)) continue;
                float water = Layout::damSide(px, pz) < 0.0f ? Layout::LAKE_LEVEL : Layout::RIVER_LEVEL;
                if (h < water + 1.0f || terrainNormal(px, pz).y < minNormalY) continue;
                vec3 leaf = vec3(0.07f, 0.22f, 0.09f) * rng.range(0.8f, 1.25f) + vec3(0.0f, rng.range(-0.02f, 0.03f), 0.0f);
                addConifer(d, cone, trunk, {px, h - 0.2f, pz}, rng.range(hMin, hMax), rng.range(0.0f, 6.28f), leaf, trunks);
            }
        }
    };
    scatter(-80.0f, 14.0f, -170.0f, 12.0f, 3.2f, 3.5f, 7.0f, 0.95f, true, 0.55f);       // left hillside, steep flanks too
    scatter(20.0f, 75.0f, -90.0f, 10.0f, 4.0f, 4.0f, 7.5f, 0.90f, true, 0.78f);         // right cliff top
    scatter(30.0f, 240.0f, -330.0f, -95.0f, 8.0f, 6.0f, 10.0f, 0.90f, false, 0.78f);    // far right shore
    scatter(-220.0f, -40.0f, -330.0f, -150.0f, 8.0f, 6.0f, 10.0f, 0.80f, false, 0.78f); // far left shore
    return d;
}

}

Trees::Trees() : mesh(build()) {}

void Trees::draw(const Shader& shader) const {
    drawMesh(shader, mesh, mat4(1.0f), Materials::MATTE);
}
