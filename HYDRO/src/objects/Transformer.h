// =============================================================================
//  Transformer.h  -  the three-phase bus ducts from the generator and the
//  STEP-UP TRANSFORMER outside the powerhouse (video 2:30 - 2:50).
//
//  The generator makes 11 kV; the transformer raises it to 220 kV so it can
//  travel far with little loss (same power, 20x less current).
//  Inside the tank: an iron core with three legs; on each leg a low-voltage
//  winding (few turns) and a high-voltage winding (many turns) around it.
//  Radiators with fans cool the oil; tall brown bushings lead the 220 kV out.
// =============================================================================
#pragma once
#include "../render/Renderer.h"
#include "FlowLines.h"

namespace TrafoDims {
    const float X0 = 79.5f, X1 = 86.5f, Z0 = 60.0f, Z1 = 68.0f;   // tank
    const float BASE_Y = 5.0f, TOP_Y = 11.2f;
    inline float phaseZ(int p) { return 61.7f + p * 2.3f; }
    inline Vec3 hvTop(int p) { return Vec3(85.6f, TOP_Y + 4.0f, phaseZ(p)); }   // top of the HV bushings
}

class Transformer {
public:
    void build(const Vec3 terminals[3]);       // generator terminal positions
    void draw(Renderer& r, float fanAngle, float xray);
    // glow: lv = generator current in the bus ducts, hv = 220 kV out of the bushings
    void drawGlow(Renderer& r, float lv, float hv, float freqVisual, float xray);

private:
    Mesh tank, tankCover, radiators, fanFrames, fanBlades, conservator, lvBushings, hvBushings, caps;
    Mesh ducts, ductFlanges, ductStands, core, lvWindings, hvWindings, platform;
    Vec3 fanPos[4];
    FlowLines ductCurrent, hvCurrent;
};
