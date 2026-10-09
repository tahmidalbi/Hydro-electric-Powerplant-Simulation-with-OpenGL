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
    // The concrete structure (static).
    void draw(const Shader& shader) const;
    // The two steel gates. gateOpening: 0 = shut (no spill), 1 = fully raised.
    void drawGates(const Shader& shader, const MeshLibrary& meshes, float gateOpening) const;
    // Translucent water pouring down the two spillway chutes (transparent pass).
    void drawSpillwayWater(const Shader& shader, float gateOpening) const;

    // Where the water wheel's axle sits, in dam-local space.
    static glm::vec3 wheelMount();

    // Spillway bays in dam-local space (used by the spray emitters and picking).
    static float bayCentre(int bay);   // bay 0 or 1, along the crest
    static float bayWidth();
    static float spillwayStart();      // first and last x of the gated section
    static float spillwayEnd();

private:
    Mesh structure;   // everything static, baked in dam-local space
    Mesh spillway;    // the water sheets of both bays
};
