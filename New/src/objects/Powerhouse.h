// =============================================================================
//  Powerhouse.h  -  the power station at the foot of the dam, drawn as a
//  CUT-AWAY (the wall facing the camera is removed) like the video at 0:04.
//
//  Inside: the penstock pipe comes down the dam face, runs through the
//  building, past a bulb turbine (spinning blades) and leaves into the river.
//  A vertical shaft drives the generator above it: copper rotor + poles that
//  spin inside a stator housing, with an exciter cap on top.
// =============================================================================
#pragma once
#include "../render/Renderer.h"
#include "CutawayPipe.h"

class Powerhouse {
public:
    CutawayPipe penstock;            // the pipe that feeds the turbine
    float turbineZ = 61.0f;          // where the turbine sits along z

    void build();
    void draw(Renderer& r, float rotorAngle);

private:
    Mesh walls, floors, cutFaces, stator, statorTop, rotor, poles, shaft, bulb, blades, cabinets, leds, motor, crane, lights;
};
