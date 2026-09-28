#pragma once
#include <glm/glm.hpp>

class Shader;
struct MeshLibrary;

// White three-blade wind turbine. The rotor faces the root's local +Z.
// Hierarchy: tapered tower -> nacelle -> hub/spinner -> 3 blades (120 degrees apart).
void drawTurbine(const Shader& shader, const MeshLibrary& meshes, const glm::mat4& turbineRoot, float spinAngle);
