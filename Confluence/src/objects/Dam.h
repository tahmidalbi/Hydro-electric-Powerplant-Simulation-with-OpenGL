#pragma once
#include <glm/glm.hpp>
#include "../core/Mesh.h"

class Shader;
struct MeshLibrary;

// Concrete gravity dam with a two-bay gated spillway, gate houses on the
// piers, crest road with railings and lamps, and the powerhouse at its right
// end. Hierarchy: dam root -> gate slot (per bay) -> gate slab.
class Dam {
public:
    Dam();
    // gateOpening: 0 = gates shut (no spill), 1 = fully raised.
    void draw(const Shader& shader, const MeshLibrary& meshes, float gateOpening) const;

    // Where the water wheel's axle sits, in dam-local space.
    static glm::vec3 wheelMount();

private:
    Mesh structure;   // everything static, baked in dam-local space
    Mesh cascade;     // one spillway chute of falling water, reused per bay
};
