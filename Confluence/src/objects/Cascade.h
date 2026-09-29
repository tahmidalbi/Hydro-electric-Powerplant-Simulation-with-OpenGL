#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "../core/Mesh.h"

// A curved sheet of falling water following a profile given as (z, y) points,
// built in a local frame: x across the flow (centred on 0), y up, z along it.
// The sheet floats `lift` above the profile (the concrete it pours over),
// widens from widthTop to widthBottom, and carries glassy-blue-to-white
// streaks in its vertex colours.
MeshData buildWaterSheet(const std::vector<glm::vec2>& profile, float widthTop, float widthBottom,
                         float lift, unsigned int seed);
