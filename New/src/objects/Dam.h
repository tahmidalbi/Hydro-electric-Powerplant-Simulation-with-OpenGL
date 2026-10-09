// =============================================================================
//  Dam.h  -  the concrete gravity dam with a 3-bay spillway.
//
//  World layout used by the first scenes (metres):
//     x : along the dam crest (left bank -> right bank)
//     y : up               (river bed ~0, crest 63, reservoir 56)
//     z : downstream       (reservoir is at z < 0, river at z > 50)
//
//  Parts: dam body (extruded cross-section), spillway chute with an ogee
//  crest and a bucket at the bottom, piers/training walls, bridge deck,
//  vertical-lift gates (animated), gate hoists, parapet, railings, lamp posts
//  and the yellow gantry crane.
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

    inline float bayX0(int i) { return SPILL_X0 + PIER_W + i * (BAY_W + PIER_W); }
    // downstream face of the non-overflow dam: z for a given height y
    inline float faceZ(float y) { return y > 57.0f ? 8.0f : 8.0f + (57.0f - y) * 0.78f; }

    std::vector<Vec2> chuteProfile();   // spillway floor, list of (z, y)
    float chuteY(float z);              // height of the chute floor at z
}

class Dam {
public:
    void build();
    void draw(Renderer& r, float gateOpen);   // gateOpen 0 = closed, 1 = fully raised

private:
    Mesh body, ribs, spillBody, piers, deck, road, roadLines, parapet, railing;
    Mesh hoistFrame, hoistHouses, gate, craneMesh, craneRails, lampPoles, lampHeads;
};
