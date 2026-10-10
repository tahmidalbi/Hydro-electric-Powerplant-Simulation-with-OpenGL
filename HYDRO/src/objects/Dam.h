// =============================================================================
//  Dam.h  -  the concrete gravity dam with a 3-bay spillway and the power
//  intake built into it.
//
//  World layout (metres):
//     x : along the dam crest (left bank -> right bank)
//     y : up               (river bed ~0, crest 63, reservoir 56)
//     z : downstream       (reservoir is at z < 0, river at z > 50)
//
//  The POWER WATERWAY is at x = 58 (under the yellow gantry crane):
//     trash-rack cage in the reservoir (z -17 .. -2)
//     -> intake mouth in the dam face (z = -2, y 34 .. 44)
//     -> conduit through the dam with the intake gate in a shaft (z ~ 1.5)
//     -> bulkhead at z = 16 where the steel penstock starts
//     -> penstock down the downstream face -> turbine in the powerhouse.
//
//  CUT-AWAY: the dam part between x = 24.5 and x = 52.5 (the "cut block")
//  can be lifted away so the conduit, gate and penstock become visible from
//  the side, exactly like the cut-away shots of the video.
// =============================================================================
#pragma once
#include <vector>
#include "../render/Renderer.h"

namespace DamDims {
    const float CREST_Y = 63.0f;        // top of the dam
    const float RESERVOIR_Y = 56.0f;    // water level behind the dam
    const float RIVER_Y = 3.0f;         // water level below the dam
    const float UPSTREAM_Z = -2.0f;     // upstream (vertical) face
    const float SPILL_X0 = -24.5f, SPILL_X1 = 24.5f;
    const float BAY_W = 11.0f, PIER_W = 4.0f;
    const int   BAYS = 3;
    const float DAM_X0 = -150.0f, DAM_X1 = 150.0f;
    const float GATE_Z = -0.8f;         // where the spillway gates sit
    const float GATE_BOTTOM_Y = 50.6f;
    const float GATE_LIFT = 9.5f;

    // ---- power intake --------------------------------------------------------
    const float INTAKE_X = 58.0f;                        // centre line of the waterway
    const float CONDUIT_X0 = 52.5f, CONDUIT_X1 = 63.5f;  // conduit width 11 m
    const float CONDUIT_Y0 = 34.0f, CONDUIT_Y1 = 44.0f;  // conduit floor / roof
    const float BULKHEAD_Z = 16.0f;                      // end of the conduit, penstock starts here
    const float SHAFT_Z0 = 0.4f, SHAFT_Z1 = 2.6f;        // vertical slot for the intake gate
    const float CUT_X = 52.5f;                           // the cut-away plane

    inline float bayX0(int i) { return SPILL_X0 + PIER_W + i * (BAY_W + PIER_W); }
    // downstream face of the non-overflow dam: z for a given height y
    inline float faceZ(float y) { return y > 57.0f ? 8.0f : 8.0f + (57.0f - y) * 0.78f; }

    std::vector<Vec2> chuteProfile();   // spillway floor, list of (z, y)
    float chuteY(float z);              // height of the chute floor at z
}

class Dam {
public:
    void build();
    // gateOpen: spillway gates 0..1,  cutLift: 0 = dam whole, 1 = cut block lifted away
    void draw(Renderer& r, float gateOpen, float cutLift);

private:
    Mesh body, intakeBlock, ribs, spillBody, piers, deck, parapet, railing, road, roadLines;
    Mesh hoistFrame, hoistHouses, gate, craneMesh, craneRails, lampPoles, lampHeads;
    // the parts that belong to the removable cut block
    Mesh cutBody, cutRoad, cutLines, cutParapet, cutRailing, cutLampPoles, cutLampHeads, cutRails;
};
