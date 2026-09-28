#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "Boat.h"
#include "Cloud.h"
#include "Dam.h"
#include "Fence.h"
#include "Rocks.h"
#include "SolarArray.h"
#include "Terrain.h"
#include "Trees.h"
#include "Water.h"
#include "WaterWheel.h"

class Shader;
struct MeshLibrary;

// Owns every scene object and places it -- the static composition of the
// proposal's concept image (no animation yet).
class Scene {
public:
    Scene();
    void draw(const Shader& shader, const MeshLibrary& meshes) const;

private:
    Terrain terrain;
    Water water;
    Dam dam;
    WaterWheel wheel;
    SolarArray solar;
    Trees trees;
    Rocks rocks;
    Fence fence;
    Boat boat;
    std::vector<Cloud> clouds;

    glm::mat4 turbine1Root, turbine2Root, wheelRoot, solarRoot, boatRoot;
    std::vector<glm::mat4> cloudRoots;
};
