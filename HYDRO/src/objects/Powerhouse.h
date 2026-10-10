// =============================================================================
//  Powerhouse.h  -  the power station at the foot of the dam, drawn as a
//  CUT-AWAY (the wall facing the camera, x = 46, is removed).
//
//  Inside, from the bottom up:
//    * the TURBINE PIT: the penstock ends at the main inlet valve, which feeds
//      the spiral casing of the Francis turbine (TurbineUnit),
//    * the vertical SHAFT with its guide bearing and coupling flange,
//    * the GENERATOR FLOOR with the generator (Generator), yellow railings,
//      control cabinets, big windows and the overhead crane.
//  The building, the penstock and the shaft are drawn here; the valve,
//  turbine and generator are separate objects.
// =============================================================================
#pragma once
#include "../render/Renderer.h"
#include "CutawayPipe.h"

namespace HouseDims {
    const float X0 = 46, X1 = 72, Z0 = 46, Z1 = 80;     // inside of the hall (x = 46 is the cut)
    const float PIT_Y = 3.5f, GEN_FLOOR_Y = 14.0f, ROOF_Y = 28.0f;
}

class Powerhouse {
public:
    CutawayPipe penstock;            // from the intake conduit down to the main inlet valve

    void build();
    void draw(Renderer& r, float shaftAngle);

private:
    Mesh foundation, walls, floors, cutFaces, windows, mullions, railings, stairs, cabinets, leds;
    Mesh crane, lights, shaft, bearings, outlet;
};
