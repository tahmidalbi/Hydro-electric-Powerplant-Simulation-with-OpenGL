#include "Turbine.h"
#include "../core/Material.h"
#include "../core/MeshLibrary.h"
#include "../core/Render.h"
#include <glm/gtc/matrix_transform.hpp>

namespace {

using glm::vec3;
using glm::mat4;

constexpr float TOWER_H = 12.0f;
constexpr float BLADE_LEN = 5.8f;

mat4 T(vec3 t) { return glm::translate(mat4(1.0f), t); }
mat4 S(vec3 s) { return glm::scale(mat4(1.0f), s); }
mat4 R(float deg, vec3 axis) { return glm::rotate(mat4(1.0f), glm::radians(deg), axis); }

}

// Scale is applied only at each part's own drawMesh() call, never baked into
// a frame a child is built from -- so children don't inherit a stretched parent.
void drawTurbine(const Shader& shader, const MeshLibrary& m, const mat4& turbineRoot, float spinAngle) {
    // Level 1: tapered tower (cylinder axis is local Y, already upright).
    drawMesh(shader, m.tower, turbineRoot * T({0.0f, TOWER_H * 0.5f, 0.0f}) * S({1.25f, TOWER_H, 1.25f}), Materials::WHITE_PAINT);

    // Level 2: nacelle on the tower top.
    mat4 nacelle = turbineRoot * T({0.0f, TOWER_H + 0.35f, 0.0f});
    drawMesh(shader, m.cube, nacelle * T({0.0f, 0.0f, -0.4f}) * S({0.95f, 0.95f, 2.8f}), Materials::WHITE_PAINT);

    // Level 3: hub with a pointed spinner at the nacelle's front.
    mat4 hub = nacelle * T({0.0f, 0.0f, 1.2f});
    drawMesh(shader, m.sphere, hub, Materials::WHITE_PAINT);
    drawMesh(shader, m.cone, hub * T({0.0f, 0.0f, 0.55f}) * R(90.0f, {1, 0, 0}) * S({0.9f, 0.7f, 0.9f}), Materials::WHITE_PAINT);

    // Level 4: three tapered blades sharing one spin angle, each slightly pitched.
    for (int i = 0; i < 3; ++i) {
        mat4 blade = hub * glm::rotate(mat4(1.0f), spinAngle + glm::radians(120.0f * i), vec3(0, 0, 1))
                         * T({0.0f, 0.2f + BLADE_LEN * 0.5f, 0.15f});
        drawMesh(shader, m.cone, blade * R(12.0f, {0, 1, 0}) * S({0.85f, BLADE_LEN, 0.16f}), Materials::WHITE_PAINT);
    }
}
