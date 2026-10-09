// =============================================================================
//  Debris.h  -  floating trash carried by the current: a big drift-wood
//  branch, logs, leaves, plastic bits and straw.
//
//  Simple physics per piece:
//    * the current pushes it downstream (+x),
//    * buoyancy pulls it to its floating depth, with a little bobbing,
//    * it tumbles slowly (rotation speed),
//    * at the trash rack: pieces bigger than the gap between the bars STOP
//      (they pile up), small leaves slip through.
// =============================================================================
#pragma once
#include <vector>
#include "../render/Renderer.h"
#include "../render/ParticleSystem.h"

enum DebrisType { DEB_BRANCH, DEB_LOG, DEB_LEAF, DEB_PLASTIC, DEB_STRAW };

struct DebrisPiece {
    DebrisType type;
    Vec3 pos, vel;
    Vec3 rot, spin;            // Euler angles (x, y, z) and their speed
    Vec3 color{1, 1, 1};
    float scale = 1.0f;
    float floatY = 6.0f;       // height it floats at
    float extent = 0.3f;       // half size along the flow (contact distance)
    float phase = 0;
    bool passes = false;       // small enough to go between the bars
    bool stuck = false;
    float stuckTime = 0;
    Vec3 stuckPos, stuckRot;   // where it ends up resting against the rack
    bool raking = false;       // being lifted out by the rake
};

class Debris {
public:
    std::vector<DebrisPiece> pieces;
    float rackX = -4.0f;       // upstream face of the rack
    float waterY = 6.5f;
    float current = 2.4f;      // m/s

    void build();
    void reset();
    void spawnWave(int wave);
    void rakeClean();
    void update(float dt, float time, ParticleSystem& splash);
    void draw(Renderer& r);
    int stuckCount() const;
    int passedCount() const { return passed; }

private:
    Mesh branch, log, leaf, plastic, straw;
    Rng rng{2024};
    int passed = 0;
    int stuckSlots = 0;
};
