#include "Water.h"
#include "Layout.h"
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

// Both sheets are laid out in dam-local space so they meet the dam wall exactly;
// the terrain hides them wherever the ground is higher.
MeshData buildLake() {
    MeshData d;
    d.append(gridData(24), Layout::damRoot() * T({40.0f, Layout::LAKE_LEVEL, -299.0f}) * S({760.0f, 1.0f, 600.0f}));
    return d;
}

MeshData buildRiver() {
    MeshData d;
    d.append(gridData(64), Layout::damRoot() * T({40.0f, Layout::RIVER_LEVEL, 155.0f}) * S({500.0f, 1.0f, 300.0f}));
    // Pale turquoise where the spillways churn it, deepening to teal downstream.
    for (Vertex& v : d.vertices) {
        float along = Layout::damAlong(v.position.x, v.position.z);
        float side = Layout::damSide(v.position.x, v.position.z);
        float dist = glm::length(glm::vec2((along - 45.0f) * 0.6f, side - 12.0f));
        v.color = glm::mix(vec3(0.14f, 0.47f, 0.54f), vec3(0.05f, 0.28f, 0.40f), smooth01((dist - 8.0f) / 55.0f));
    }
    return d;
}

MeshData buildFoam() {
    MeshData d;
    MeshData puff = sphereData(6, 10);
    Noise::Rng rng(99);
    const mat4 dam = Layout::damRoot();
    auto blob = [&](const mat4& frame, float x, float z, float r, float flat, float stretch, vec3 col) {
        d.append(puff, frame * T({x, Layout::RIVER_LEVEL + 0.1f, z}) * S({r * 2.0f, r * flat, r * 2.0f * stretch}), col);
    };

    // Churning white water at the foot of the two spillway chutes: dense,
    // overlapping lumps that read as one frothing mass, thinning downstream.
    for (int i = 0; i < 380; ++i) {
        float t = std::pow(rng.next(), 1.7f);
        float z = 11.0f + t * 30.0f;
        float spread = (z - 11.0f) * 0.35f;
        float x = rng.range(30.0f, 50.0f) + rng.range(-spread, spread);
        float r = rng.range(1.0f, 2.6f) * (1.0f - 0.7f * t);
        float w = rng.range(0.90f, 1.0f);
        blob(dam, x, z, r, rng.range(0.35f, 0.7f), 1.0f, glm::mix(vec3(w, w, 1.0f), vec3(0.62f, 0.86f, 0.90f), t * 0.6f));
    }
    // Outflow below the water wheel.
    for (int i = 0; i < 60; ++i) {
        float z = 13.0f + std::pow(rng.next(), 1.5f) * 12.0f;
        float x = rng.range(64.0f, 71.0f) + rng.range(-1.0f, 1.0f) * (z - 13.0f) * 0.3f;
        float r = rng.range(0.5f, 1.3f) * (1.0f - (z - 13.0f) / 16.0f);
        float w = rng.range(0.9f, 1.0f);
        blob(dam, x, z, r, rng.range(0.5f, 0.9f), 1.0f, vec3(w, w, 1.0f));
    }
    // Faint streaks of broken water running down the river towards the viewer.
    for (int i = 0; i < 18; ++i) {
        float z = rng.range(-58.0f, -20.0f);
        float left = Layout::leftBankX(z) + 3.0f, right = Layout::rightBankX(z) - 3.0f;
        float x = glm::mix(left, right, rng.range(0.1f, 0.9f));
        blob(mat4(1.0f), x, z, rng.range(0.2f, 0.45f), 0.05f, rng.range(5.0f, 9.0f), vec3(0.30f, 0.56f, 0.62f));
    }
    // White-water tongue carried downstream from the spillway, fading into the current.
    for (int i = 0; i < 110; ++i) {
        float t = std::pow(rng.next(), 1.4f);
        float z = glm::mix(-66.0f, -40.0f, t);
        float centre = glm::mix(1.0f, 8.0f, t) + rng.range(-7.0f, 7.0f) * (0.6f + 0.5f * t);
        vec3 col = glm::mix(vec3(0.93f, 0.97f, 0.98f), vec3(0.40f, 0.66f, 0.70f), t);
        blob(mat4(1.0f), centre, z, rng.range(0.5f, 1.2f) * (1.0f - 0.5f * t), 0.08f, rng.range(2.0f, 4.0f), col);
    }
    return d;
}

}

Water::Water() : lake(buildLake()), river(buildRiver()), foam(buildFoam()) {}

void Water::draw(const Shader& shader) const {
    drawMesh(shader, lake, mat4(1.0f), Materials::LAKE_WATER);
    drawMesh(shader, river, mat4(1.0f), Materials::WATER);
    drawMesh(shader, foam, mat4(1.0f), Materials::FOAM);
}
