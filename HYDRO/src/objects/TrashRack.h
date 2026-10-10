// =============================================================================
//  TrashRack.h  -  the steel bar screen that stops branches, logs, leaves and
//  plastic from entering the intake (they would damage the turbine).
//
//  A rack "panel" lies in its local XY plane: vertical flat bars spaced a
//  little apart, held by a top and a bottom beam with bolted end brackets.
//  Scene 3 uses one flat panel on the dam face; scene 4 builds a cage from
//  four panels.
// =============================================================================
#pragma once
#include "../render/Renderer.h"

struct RackSpec {
    float width = 13.0f;      // along local x
    float y0 = 0.0f, y1 = 12.0f;
    float spacing = 0.9f;     // centre-to-centre bar distance (a bar sits on each side of x = 0)
    float barW = 0.14f;       // bar thickness across the flow
    float barD = 0.5f;        // bar depth along the flow
    bool tips = true;         // pointed tops above the top beam
    bool brackets = true;     // big bolted end plates
    bool beams = true;        // top/bottom beams
};

class TrashRack {
public:
    void build(const RackSpec& spec, const Mat4& placement);   // can be called several times (cage)
    void finish();                                             // upload to the GPU
    void draw(Renderer& r, const Mat4& model);
private:
    MeshBuilder barsB, frameB, boltsB;
    Mesh bars, frame, bolts;
};
