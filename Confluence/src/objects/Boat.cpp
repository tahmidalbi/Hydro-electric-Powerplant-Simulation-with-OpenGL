#include "Boat.h"
#include "../core/Material.h"
#include "../core/Render.h"
#include <glm/gtc/matrix_transform.hpp>

namespace {

using glm::vec3;
using glm::mat4;

const vec3 HULL(0.95f, 0.96f, 0.97f);
const vec3 STRIPE(0.15f, 0.30f, 0.60f);
const vec3 GLASS(0.15f, 0.22f, 0.30f);
const vec3 WAKE(0.92f, 0.96f, 1.00f);

mat4 T(vec3 t) { return glm::translate(mat4(1.0f), t); }
mat4 S(vec3 s) { return glm::scale(mat4(1.0f), s); }
mat4 R(float deg, vec3 axis) { return glm::rotate(mat4(1.0f), glm::radians(deg), axis); }

MeshData build() {
    MeshData d;
    MeshData cube = cubeData(), pyramid = coneData(4), puff = sphereData(6, 10);

    d.append(cube, T({0.0f, 0.25f, -0.3f}) * S({1.7f, 0.8f, 3.6f}), HULL);
    d.append(cube, T({0.0f, 0.35f, -0.3f}) * S({1.74f, 0.12f, 3.64f}), STRIPE);
    // Bow: a 4-sided cone turned to point forward, its square base matching the hull section.
    d.append(pyramid, T({0.0f, 0.25f, 2.3f}) * R(90.0f, {1, 0, 0}) * S({1.7f / 0.707f, 1.6f, 0.8f / 0.707f}) * R(45.0f, {0, 1, 0}), HULL);
    d.append(cube, T({0.0f, 0.95f, -0.6f}) * S({1.2f, 0.6f, 1.4f}), HULL);
    d.append(cube, T({0.0f, 1.05f, 0.18f}) * R(-30.0f, {1, 0, 0}) * S({1.1f, 0.45f, 0.06f}), GLASS);

    // Wake: a widening V of foam behind the stern.
    for (int i = 1; i <= 7; ++i) {
        float z = -2.3f - i * 1.4f, spread = 0.5f + i * 0.4f, size = 0.5f + 0.12f * i;
        for (float side : {-1.0f, 1.0f})
            d.append(puff, T({side * spread, 0.02f, z}) * S({size, 0.08f, 1.8f}), WAKE);
        d.append(puff, T({0.0f, 0.02f, z + 0.6f}) * S({0.7f, 0.06f, 1.6f}), WAKE);
    }
    return d;
}

}

Boat::Boat() : mesh(build()) {}

void Boat::draw(const Shader& shader, const glm::mat4& root) const {
    drawMesh(shader, mesh, root, Materials::GLOSSY);
}
