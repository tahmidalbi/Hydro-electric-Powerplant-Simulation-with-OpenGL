// =============================================================================
//  TurbineUnit.h  -  the Francis turbine (video 1:10 - 1:50).
//
//  Water path:  main inlet valve -> SPIRAL CASING (a snail-shaped pipe that
//  gets narrower as it wraps round) -> 12 fixed STAY VANES -> 24 movable
//  GUIDE VANES (wicket gates: they open/close to control the flow) ->
//  RUNNER (15 curved blades, turns the shaft) -> DRAFT TUBE (down and out
//  to the river).
//
//  The casing is open on top (cut-away) so the swirling water is visible;
//  for the SECTION VIEW the near half (x < centre) is hidden as well.
//  Glowing streamlines show the path of the water through the machine.
// =============================================================================
#pragma once
#include <vector>
#include "../render/Renderer.h"
#include "FlowLines.h"

namespace TurbineDims {
    const Vec3 CENTRE(62.5f, 6.0f, 64.0f);   // runner centre (spiral casing plane)
    const float RUNNER_R = 1.62f;
    const float GUIDE_R = 1.98f;
    const float STAY_R = 2.55f;
}

class TurbineUnit {
public:
    float casingLength = 0;                  // length of the casing water path (m)

    void build();
    void draw(Renderer& r, float runnerAngle, float vaneOpen01, bool section);
    void drawWater(Renderer& r, float casingFront, bool section, float glow);
    void drawFlowLines(Renderer& r, float strength, float speed, float front, bool section);
    float flowLinesLength() const { return lines.longest; }

private:
    Mesh shellNear, shellFar, rimsNear, rimsFar, flangeRings, waterNear, waterFar;
    Mesh stayRing, stayVanes, guideVane, operatingRing, runner, runnerBlades, cone, pedestals;
    FlowLines lines;
    FlowLines sectionLines;                  // the same lines, only the far half (for the section view)
};
