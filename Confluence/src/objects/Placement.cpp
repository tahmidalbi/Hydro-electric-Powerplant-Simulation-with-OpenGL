#include "Placement.h"
#include "Dam.h"
#include "Layout.h"
#include "Terrain.h"
#include "../core/Transform.h"
#include <cmath>

namespace {

using Transform::translate;
using Transform::rotateY;
using Transform::degreesToRadians;

// A turbine standing on the ground at (x, z), rotor turned to face the viewer.
// Root = Translate(ground point) * RotateY(yaw): turn first, then move.
glm::mat4 turbineOnGround(glm::vec2 xz) {
    float yaw = std::atan2(Layout::CAMERA_EYE.x - xz.x, Layout::CAMERA_EYE.z - xz.y);
    return translate({xz.x, terrainHeight(xz.x, xz.y) - 0.3f, xz.y}) * rotateY(yaw);
}

}

glm::mat4 Placement::turbineRoot(int index) {
    // Computed once (static): the terrain does not change.
    static const glm::mat4 roots[2] = {turbineOnGround(Layout::TURBINE1_XZ), turbineOnGround(Layout::TURBINE2_XZ)};
    return roots[index == 0 ? 0 : 1];
}

glm::vec3 Placement::turbineHub(int index) {
    // Same offsets as Turbine.cpp: nacelle 12.35 up, hub 1.2 forward.
    return glm::vec3(turbineRoot(index) * glm::vec4(0.0f, 12.35f, 1.2f, 1.0f));
}

glm::mat4 Placement::wheelRoot() {
    return Layout::damRoot() * translate(Dam::wheelMount());
}

glm::mat4 Placement::solarRoot() {
    static const glm::mat4 root =
        translate({Layout::SOLAR_XZ.x, terrainHeight(Layout::SOLAR_XZ.x, Layout::SOLAR_XZ.y), Layout::SOLAR_XZ.y})
        * rotateY(degreesToRadians(Layout::SOLAR_YAW_DEG));
    return root;
}
