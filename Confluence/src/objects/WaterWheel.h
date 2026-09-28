#pragma once
#include <glm/glm.hpp>
#include "../core/Mesh.h"

class Shader;
struct MeshLibrary;

// Blue hydro water wheel: twin outer and inner rims, spokes and buckets.
// Hierarchy: mount -> axle -> wheel (spins about its local Z axis).
class WaterWheel {
public:
    WaterWheel();
    void draw(const Shader& shader, const MeshLibrary& meshes, const glm::mat4& root, float spinAngle) const;

private:
    Mesh wheel;
};
