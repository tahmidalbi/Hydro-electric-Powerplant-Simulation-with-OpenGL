#include "Turbine.h"
#include "../core/Material.h"
#include "../core/MeshLibrary.h"
#include "../core/Render.h"
#include "../core/Transform.h"

namespace {

using glm::vec3;
using glm::mat4;
using Transform::translate;
using Transform::scale;
using Transform::rotateX;
using Transform::rotateY;
using Transform::rotateZ;
using Transform::degreesToRadians;

constexpr float TOWER_H = 12.0f;
constexpr float BLADE_LEN = 5.8f;

}

// STEP 2 - Hierarchy.
//
//   turbineRoot                       (where the turbine stands, which way it faces)
//     -> tower                        (a child: root * its own offset)
//     -> nacelle = root * T(up)       (sits on top of the tower)
//          -> hub = nacelle * T(forward)
//               -> blade i = hub * Rz(spin + 120 deg * i) * T(out along the blade)
//
// Each child frame is "parent frame * local offset", so moving or turning the
// root carries the tower, nacelle, hub and all blades with it.
//
// The hub frame's origin IS the rotor pivot: Rz(spin) turns the blades about
// the hub centre, so the rotor spins in place.
//
// Scale (size) is applied only in each part's own drawMesh() call, at the
// right end of the product, and never baked into a frame a child is built
// from -- so a tall, thin tower does not stretch the nacelle above it.
void drawTurbine(const Shader& shader, const MeshLibrary& m, const mat4& turbineRoot, float spinAngle) {
    // Level 1: tapered tower (cylinder axis is local Y, already upright).
    drawMesh(shader, m.tower, turbineRoot * translate({0.0f, TOWER_H * 0.5f, 0.0f}) * scale({1.25f, TOWER_H, 1.25f}), Materials::WHITE_PAINT);

    // Level 2: nacelle on the tower top.
    mat4 nacelle = turbineRoot * translate({0.0f, TOWER_H + 0.35f, 0.0f});
    drawMesh(shader, m.cube, nacelle * translate({0.0f, 0.0f, -0.4f}) * scale({0.95f, 0.95f, 2.8f}), Materials::WHITE_PAINT);

    // Level 3: hub with a pointed spinner at the nacelle's front.
    mat4 hub = nacelle * translate({0.0f, 0.0f, 1.2f});
    drawMesh(shader, m.sphere, hub, Materials::WHITE_PAINT);
    drawMesh(shader, m.cone, hub * translate({0.0f, 0.0f, 0.55f}) * rotateX(degreesToRadians(90.0f)) * scale({0.9f, 0.7f, 0.9f}), Materials::WHITE_PAINT);

    // Level 4: three tapered blades sharing one spin angle, each slightly pitched.
    for (int i = 0; i < 3; ++i) {
        mat4 blade = hub * rotateZ(spinAngle + degreesToRadians(120.0f * i))
                         * translate({0.0f, 0.2f + BLADE_LEN * 0.5f, 0.15f});
        drawMesh(shader, m.cone, blade * rotateY(degreesToRadians(12.0f)) * scale({0.85f, BLADE_LEN, 0.16f}), Materials::WHITE_PAINT);
    }
}
