// =============================================================================
//  Generator.h  -  the vertical generator above the turbine (video 2:00-2:30).
//
//  STATOR (fixed): a ring of thin steel sheets with 72 slots; copper coils lie
//  in the slots and loop round at the top and bottom (end windings).
//  ROTOR (turns with the shaft): a hub with spider arms, a rim, and 40 POLES,
//  each wrapped in a copper FIELD COIL.  When DC current excites the field
//  coils the poles become electromagnets; their spinning magnetic field
//  cuts the stator coils and induces three-phase alternating current.
//      f = poles x rpm / 120 = 40 x 150 / 120 = 50 Hz
//
//  The housing has a 120 degree window towards the camera (cut-away).
// =============================================================================
#pragma once
#include "../render/Renderer.h"
#include "FlowLines.h"

namespace GenDims {
    const Vec3 BASE(62.5f, 14.0f, 64.0f);     // centre of the generator floor opening
    const float CORE_Y0 = 14.6f, CORE_Y1 = 18.8f;
    const float WINDOW = 1.0471976f;          // half width of the cut-away window (60 deg)
}

class Generator {
public:
    void build();
    // excitation 0..1 (field current), current 0..1 (stator current shown glowing)
    void draw(Renderer& r, float rotorAngle, float excitation, float current, float time);
    void drawGlow(Renderer& r, float rotorAngle, float excitation, float current, float freqVisual);
    Vec3 terminal(int phase) const;           // where the phase leads leave the housing

private:
    Mesh housing, ribs, topCap, core, teeth, slotCopper, endWindings, terminals, insulators;
    Mesh rotorRim, rotorPoles, fieldCoils, slipRings;
    FlowLines fieldLines, currentLines;
};
