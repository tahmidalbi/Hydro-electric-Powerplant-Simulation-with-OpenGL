// =============================================================================
//  IntakeStructure.h  -  the concrete intake in front of the dam face at
//  x = 58: a foundation on the reservoir bed, two side piers, a top deck with
//  the blue trash-rake motors, and the TRASH-RACK CAGE of steel bars that
//  every drop of turbine water must pass through.
//
//  The parts on the near side (x < 52.5) are a separate "cut" group: they are
//  hidden in the cut-away views so we can look at the cage from the side.
// =============================================================================
#pragma once
#include "../render/Renderer.h"
#include "TrashRack.h"

namespace IntakeDims {
    const float FRONT_Z = -17.0f;        // upstream edge of the intake structure
    const float CAGE_FRONT_Z = -15.8f;   // the upstream bar screen (trash stops here)
    const float CAGE_X0 = 52.3f, CAGE_X1 = 63.7f;
    const float FLOOR_Y = 34.0f;         // cage floor = conduit floor
    const float TOP_Y = 61.0f;           // top of the bars (above the water)
}

class IntakeStructure {
public:
    void build();
    void draw(Renderer& r, bool showNearSide);

private:
    Mesh foundation, piersFar, deckFar, nearParts, motors, motorShafts, conduitFloor, rocks, mouthFrame;
    TrashRack cage;
};
