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
struct SceneState;

// Owns every scene object and draws it -- the composition of the proposal's
// concept image. Fixed placements come from Placement.h; everything that moves
// (rotors, gate, wheel, boat, clouds, panel tilt) is read from SceneState.
class Scene {
public:
    Scene();
    // highlight: glow colour for the selected object (Step 8).
    void draw(const Shader& shader, const MeshLibrary& meshes, const SceneState& state, const glm::vec3& highlight) const;

private:
    struct CloudSpec { unsigned int seed; glm::vec3 extent; glm::vec3 position; float parallax; };

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
    std::vector<CloudSpec> cloudSpecs;
};
