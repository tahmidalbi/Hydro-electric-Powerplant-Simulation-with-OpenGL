// =============================================================================
//  MainInletValve.h  -  the big gate valve between the end of the penstock
//  and the turbine's spiral casing (video 1:10).
//
//  A heavy steel gate slides up into the bonnet when the valve opens; the
//  hand-wheel on top turns while it moves.  The body is cut open on the
//  camera (-x) side so the moving gate can be seen.
// =============================================================================
#pragma once
#include "../render/Renderer.h"

namespace ValveDims {
    const float X = 58.0f, Y = 6.0f;          // pipe axis
    const float Z0 = 57.0f, Z1 = 60.6f;       // valve body along the flow
    const float R = 2.0f;                     // pipe radius
    const float STROKE = 4.4f;                // how far the gate rises
}

class MainInletValve {
public:
    void build();
    void draw(Renderer& r, float open01, float wheelAngle);
private:
    Mesh flanges, bolts, body, bonnet, gate, stem, wheel, actuator, label;
};
