// =============================================================================
//  IntakeGate.h  -  the steel vertical-lift gate that opens the way from the
//  reservoir into the penstock, lifted by hydraulic cylinders.
//
//  HIERARCHICAL TRANSFORMATION: the gate leaf moves up by 'lift' metres; the
//  piston rods are attached to the gate, so they move with it and slide into
//  the fixed cylinders above.
// =============================================================================
#pragma once
#include "../render/Renderer.h"

class IntakeGate {
public:
    float height = 10.0f;       // gate leaf height (m)
    float maxLift = 9.0f;       // how far it can rise

    void build();
    void draw(Renderer& r, float lift);   // lift in metres

private:
    Mesh leaf, guides, cylinders, rods, deck, grating, gratingFrame;
};
