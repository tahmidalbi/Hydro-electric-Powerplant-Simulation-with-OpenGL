// =============================================================================
//  Terrain.h  -  the river valley around the dam: rocky mountain sides, the
//  reservoir bed upstream and the river bed downstream.
//
//  The shape is a height function h(x, z) built from fractal value noise
//  (computed on the CPU here), sampled on a 260 x 260 grid.  It is drawn
//  with GOURAUD shading by default (large, smooth object = good use case).
// =============================================================================
#pragma once
#include "../render/Renderer.h"

class Terrain {
public:
    void build();
    void draw(Renderer& r);
    static float height(float x, float z);
    static float fbm(float x, float z);       // 0..1 fractal noise on the CPU
private:
    Mesh mesh;
};
