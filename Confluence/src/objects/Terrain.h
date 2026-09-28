#pragma once
#include <glm/glm.hpp>
#include "../core/Mesh.h"

class Shader;

// Ground height / normal at world (x, z). Everything standing on the
// landscape (trees, turbines, fence, solar rack, rocks) samples these.
float terrainHeight(float x, float z);
glm::vec3 terrainNormal(float x, float z);

// The landscape: the hillside the viewer stands on rising to the turbine
// ridge, the river valley below the dam, the lake basin and the far hills.
class Terrain {
public:
    Terrain();
    void draw(const Shader& shader) const;

private:
    Mesh mesh;
};
