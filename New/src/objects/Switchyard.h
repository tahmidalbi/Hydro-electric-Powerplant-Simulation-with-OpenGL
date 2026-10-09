// =============================================================================
//  Switchyard.h  -  the electrical yard next to the powerhouse: lattice steel
//  towers, gantries, transformers with insulators, wires and a fence.
// =============================================================================
#pragma once
#include "../render/Renderer.h"

class Switchyard {
public:
    void build();
    void draw(Renderer& r);
    // helper: a 4-legged lattice tower made of thin beams
    static void latticeTower(MeshBuilder& b, const Vec3& base, float baseHalf, float topHalf, float height, int levels);
private:
    Mesh platform, steelwork, transformers, insulators, wires, fence;
};
