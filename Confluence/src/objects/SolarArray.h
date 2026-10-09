#pragma once
#include <glm/glm.hpp>
#include "../core/Mesh.h"

class Shader;

// Single-axis solar TRACKER: framed blue PV panels (individual cells visible)
// on a torque tube carried by fixed posts. Faces the root's local +Z.
//
// Step 7 split the old single mesh into two, because a moving part must not
// be baked into a fixed one:
//   rack   - posts + torque tube, fixed to the ground
//   panels - glass, frames and rails, turned about the tube (Z / C keys)
class SolarArray {
public:
    SolarArray();
    // tiltDeg: angle of the panels above horizontal (within tiltLimits).
    void draw(const Shader& shader, const glm::mat4& root, float tiltDeg) const;

    // The moving panels' matrix (also used to place the selection box).
    static glm::mat4 panelMatrix(const glm::mat4& root, float tiltDeg);

    // The hinge's limits, computed from the real terrain: the panels' edges
    // must stay at least 0.35 m above the slope.
    static void tiltLimits(float& minDeg, float& maxDeg);
    // Smallest height of the panel edges above the ground at this tilt.
    static float groundClearance(float tiltDeg);

private:
    Mesh panels;
    Mesh rack;
};
