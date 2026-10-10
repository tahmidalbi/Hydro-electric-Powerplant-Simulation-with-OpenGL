// =============================================================================
//  IntakeGate.h  -  the steel vertical-lift gate inside the dam that opens
//  the conduit, lifted by hydraulic cylinders standing on the dam crest.
//
//  Local frame: the water flows along +z, the gate leaf lies in the plane
//  z = 0, its width runs along x (centred on 0), its bottom is at y = 0.
//
//  HIERARCHICAL TRANSFORMATION: the gate leaf moves up by 'lift' metres; the
//  piston rods are attached to the leaf, so they use the same matrix and
//  slide into the fixed cylinders above.
// =============================================================================
#pragma once
#include "../render/Renderer.h"

class IntakeGate {
public:
    float width = 11.0f;        // conduit width
    float height = 10.0f;       // conduit height = gate leaf height
    float crestAbove = 29.2f;   // dam crest height above the conduit floor
    float maxLift = 10.6f;      // how far the gate rises into its shaft

    void build();
    void draw(Renderer& r, const Mat4& base, float lift);

private:
    Mesh leaf, rods, guides, cylinders, frame, ladder, ladderFrame;
};
