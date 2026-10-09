#include "Water.h"
#include "Cascade.h"
#include "Dam.h"
#include "Layout.h"
#include "../core/Material.h"
#include "../core/Noise.h"
#include "../core/Render.h"
#include "../core/Shader.h"
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <cmath>
#include <vector>

namespace {

using glm::vec3;
using glm::mat4;

float smooth01(float t) { t = std::clamp(t, 0.0f, 1.0f); return t * t * (3.0f - 2.0f * t); }
mat4 T(vec3 t) { return glm::translate(mat4(1.0f), t); }
mat4 S(vec3 s) { return glm::scale(mat4(1.0f), s); }

// Everything below is laid out in dam-local space: a = along the crest,
// s = downstream distance from the upstream face. The spillway chutes land
// across a in [30.4, 49.6] at s ~ 11-14; the wheel channel spills at s ~ 17.
constexpr float SPILL_A = 40.0f, WHEEL_A = 67.5f;
constexpr float CHANNEL_LEVEL = 2.8f;   // water in the wheel channel, above its sill

const vec3 DEEP(0.04f, 0.23f, 0.35f);
const vec3 TURQUOISE(0.05f, 0.32f, 0.43f);
const vec3 AERATED(0.22f, 0.54f, 0.64f);
const vec3 WHITE(0.93f, 0.97f, 0.98f);

// How violently the water is churned at (a, s): 1 where the falls land,
// falling off quickly downstream.
float churn(float a, float s) {
    float halfW = 10.0f + std::max(s - 12.0f, 0.0f) * 0.35f;
    float across = 1.0f - smooth01((std::abs(a - SPILL_A) - halfW) / 5.0f);
    float along = smooth01((s - 9.0f) / 2.0f) * (1.0f - smooth01((s - 13.0f) / 26.0f));

    float wHalf = 3.0f + std::max(s - 17.0f, 0.0f) * 0.3f;
    float wAcross = 1.0f - smooth01((std::abs(a - WHEEL_A) - wHalf) / 3.0f);
    float wAlong = smooth01((s - 16.5f) / 1.5f) * (1.0f - smooth01((s - 18.5f) / 11.0f));
    return std::max(across * along, 0.8f * wAcross * wAlong);
}

// Foam coverage: solid white only right under the falls; elsewhere churn
// broken into streaks stretched along the flow, plus scattered whitecaps
// on the rapids further downstream -- blue water shows between them.
float foamAmount(float a, float s, float c) {
    float n1 = Noise::fbm(a * 0.45f, s * 0.09f, 4, 61);
    float n2 = Noise::fbm(a * 1.6f, s * 0.35f, 3, 67);
    float foam = smooth01((c * 0.95f + (n1 - 0.5f) * 1.4f + (n2 - 0.5f) * 0.8f - 0.55f) / 0.22f);
    foam = std::max(foam, smooth01((c - 0.9f) / 0.08f));
    // Rapids: long white streaks stretched along the current all the way down.
    float r1 = smooth01((Noise::fbm(a * 0.35f, s * 0.05f, 3, 71) - 0.58f) / 0.07f);
    float r2 = smooth01((Noise::fbm(a * 0.9f, s * 0.12f, 3, 89) - 0.62f) / 0.08f);
    float rapids = std::max(r1, 0.8f * r2) * smooth01((s - 24.0f) / 12.0f);
    return std::max(foam, 0.7f * rapids);
}

// Grid coordinates, finer inside the given bands (lo, hi, step) where the
// camera sees detail.
struct Band { float lo, hi, step; };
std::vector<float> samples(float lo, float hi, const std::vector<Band>& bands, float coarse) {
    std::vector<float> v;
    float x = lo;
    while (x < hi) {
        v.push_back(x);
        float step = coarse;
        for (const Band& b : bands)
            if (x >= b.lo && x < b.hi) step = std::min(step, b.step);
        x += step;
    }
    v.push_back(hi);
    return v;
}

// A displaced water grid in dam-local space; `surface(a, s)` returns height
// and colour for each vertex.
template <typename F>
void addWaterGrid(MeshData& d, const std::vector<float>& as, const std::vector<float>& ss, F surface) {
    const mat4 dam = Layout::damRoot();
    unsigned int base = (unsigned int)d.vertices.size();
    for (float s : ss) {
        for (float a : as) {
            vec3 col;
            float y = surface(a, s, col);
            d.vertices.push_back({vec3(dam * glm::vec4(a, y, s, 1.0f)), vec3(0.0f, 1.0f, 0.0f), col});
        }
    }
    const unsigned int cols = (unsigned int)as.size();
    std::vector<unsigned int> idx;
    for (unsigned int j = 0; j + 1 < ss.size(); ++j) {
        for (unsigned int i = 0; i + 1 < cols; ++i) {
            unsigned int p = base + j * cols + i, q = p + cols;
            idx.insert(idx.end(), {p, q, p + 1, p + 1, q, q + 1});
        }
    }
    d.indices.insert(d.indices.end(), idx.begin(), idx.end());
}

// Step 7: the lake is now a fine grid, because the vertex shader moves every
// vertex up and down with the waves -- a few huge triangles could not show a
// 6 m wave. Fine (1.2 m) where the camera sees the water, coarse far away and
// under the hills. It stops at the dam's upstream face (s = 1).
MeshData buildLake() {
    MeshData d;
    std::vector<float> as = samples(-340.0f, 420.0f, {{-20.0f, 150.0f, 1.2f}, {-80.0f, 260.0f, 2.5f}}, 12.0f);
    std::vector<float> ss = samples(-600.0f, 1.0f, {{-230.0f, 1.0f, 1.2f}, {-330.0f, -230.0f, 2.5f}}, 12.0f);
    addWaterGrid(d, as, ss, [](float, float, vec3& col) {
        col = vec3(1.0f);                       // colour comes from the material
        return Layout::LAKE_LEVEL;              // flat; the shader adds the waves
    });
    return d;
}

MeshData buildRiver() {
    MeshData d;

    // The river below the dam, covering the whole visible valley.
    std::vector<float> as = samples(-15.0f, 140.0f, {{22.0f, 78.0f, 0.45f}}, 1.3f);
    std::vector<float> ss = samples(2.0f, 130.0f, {{5.0f, 45.0f, 0.45f}, {45.0f, 80.0f, 0.8f}}, 1.5f);
    addWaterGrid(d, as, ss, [](float a, float s, vec3& col) {
        float c = churn(a, s);
        float foam = foamAmount(a, s, c);
        vec3 water = glm::mix(TURQUOISE, DEEP, smooth01((s - 25.0f) / 50.0f));
        water = glm::mix(water, AERATED, smooth01(c * 1.5f) * 0.6f);
        col = glm::mix(water, WHITE, foam);
        float chop = (Noise::fbm(a * 0.35f, s * 0.22f, 3, 73) - 0.5f) * 2.0f;
        float ripple = (Noise::fbm(a * 1.1f, s * 0.9f, 2, 79) - 0.5f) * 0.12f;
        return Layout::RIVER_LEVEL + 0.05f + chop * (0.06f + 0.45f * c + 0.12f * foam) + ripple + 0.12f * foam;
    });

    // Water in the wheel channel, frothing around the buckets.
    const glm::vec3 wheel = Dam::wheelMount();
    std::vector<float> ca, cs;
    for (float a = 63.6f; a <= 71.4f; a += 0.35f) ca.push_back(a);
    for (float s = 6.2f; s <= 16.0f; s += 0.35f) cs.push_back(s);
    addWaterGrid(d, ca, cs, [wheel](float a, float s, vec3& col) {
        // Froth only where the buckets dip in and at the lip it spills over.
        float nearWheel = 1.0f - smooth01((glm::length(glm::vec2((a - wheel.x) * 0.8f, s - wheel.z)) - 0.8f) / 2.5f);
        float lip = 0.6f * smooth01((s - 14.5f) / 1.5f) * (1.0f - smooth01((std::abs(a - WHEEL_A) - 2.5f) / 1.5f));
        float n = Noise::fbm(a * 1.4f, s * 0.8f, 3, 83);
        float foam = smooth01((std::max(nearWheel, lip) + (n - 0.5f) * 1.0f - 0.45f) / 0.25f);
        col = glm::mix(AERATED, WHITE, foam);
        return CHANNEL_LEVEL + (n - 0.5f) * 0.2f + 0.1f * foam;
    });

    computeSmoothNormals(d.vertices, d.indices);
    for (Vertex& v : d.vertices)
        if (v.normal.y < 0.0f) v.normal = -v.normal;
    return d;
}

// Water spilling off the end of the wheel channel into the river.
MeshData buildWheelFall() {
    const std::vector<glm::vec2> profile = {{14.6f, 2.88f}, {15.9f, 2.84f}, {16.6f, 2.3f}, {17.1f, 1.5f},
                                            {17.7f, 0.85f}, {18.8f, 0.62f}};
    MeshData d;
    d.append(buildWaterSheet(profile, 5.6f, 7.0f, 0.12f, 57u), Layout::damRoot() * T({WHEEL_A, 0.0f, 0.0f}));
    return d;
}

}

// (The old static spray mesh is gone: since Step 6 the spray is made of
// moving particles whose amount follows the gate -- systems/Effects.cpp.)

Water::Water() : lake(buildLake()), river(buildRiver()), wheelFall(buildWheelFall()) {}

void Water::drawSurfaces(const Shader& shader, float flowStrength) const {
    // Lake: the vertex shader lifts this flat grid onto the waves (Step 7).
    shader.setInt("uWaveMode", 1);
    drawMesh(shader, lake, mat4(1.0f), Materials::LAKE_WATER);
    shader.setInt("uWaveMode", 0);

    // River: foam streaks drift downstream (Step 6). The white foam baked into
    // the mesh was made for the default 45% gate, so the shader scales it:
    // gate shut -> calm blue water, gate wide open -> full white water.
    setFlowUniforms(shader, 2, 1.5f + 3.5f * flowStrength);
    shader.setFloat("uFoam", std::min(1.0f, 0.12f + flowStrength / 0.45f));
    drawMesh(shader, river, mat4(1.0f), Materials::WATER);
    setFlowUniforms(shader, 0, 0.0f);
}

void Water::drawFalls(const Shader& shader, float flowStrength) const {
    // The wheel channel is fed by the same gate: no flow -> no fall.
    if (flowStrength < 0.02f) return;
    Material sheet = Materials::WATERFALL;
    sheet.alpha *= std::min(1.0f, flowStrength * 2.5f);   // thin and glassy when barely open
    setFlowUniforms(shader, 1, 2.2f + 2.0f * flowStrength);
    drawMesh(shader, wheelFall, mat4(1.0f), sheet);
    setFlowUniforms(shader, 0, 0.0f);
}
