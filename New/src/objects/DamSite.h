// =============================================================================
//  DamSite.h  -  the whole outdoor world of scenes 1 and 2, put together:
//  terrain + reservoir + river + dam + spillway water + powerhouse +
//  switchyard + spray/mist particles.  Both scenes share this one object.
// =============================================================================
#pragma once
#include "Dam.h"
#include "Terrain.h"
#include "Powerhouse.h"
#include "Switchyard.h"
#include "SpillwayWater.h"
#include "../render/ParticleSystem.h"

class DamSite {
public:
    Dam dam;
    Terrain terrain;
    Powerhouse powerhouse;
    Switchyard yard;
    SpillwayWater spill;
    ParticleSystem spray, mist, outflow;

    bool  gatesOpen = true;
    float gateOpen = 1.0f;          // animated 0..1 position of the spillway gates
    // water inside the powerhouse penstock
    float pipeFront = 0, pipeTail = -1e3f;
    bool  pipeFeeding = true;
    float rotorAngle = 0, rotorSpeed = 0;
    float time = 0;

    void build();
    void reset(bool waterRunning);  // set up the start state of a scene
    void setWaterRunning(bool on);  // the STOP / START WATER button
    void update(float dt);
    void drawOpaque(Renderer& r);
    void drawTransparent(Renderer& r);
    bool turbineRunning() const { return rotorSpeed > 0.3f; }

private:
    Mesh reservoir, river;
    Rng rng{99};
    float pipeSpeedAt(float s) const;
};
