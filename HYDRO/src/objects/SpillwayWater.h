// =============================================================================
//  SpillwayWater.h  -  the water rushing down the three spillway chutes.
//
//  The water is a thin sheet that follows the chute floor.  How far it has
//  travelled is simulated with free-fall physics:  speed = sqrt(2 g h),
//  where h is how far the water has dropped from the reservoir.  So the
//  water front starts slowly at the crest and becomes very fast at the bottom.
//
//  When the gates close, the "tail" of the water runs down the chute the same
//  way, so the chute drains naturally instead of the water just vanishing.
// =============================================================================
#pragma once
#include <vector>
#include "../render/Renderer.h"
#include "../render/ParticleSystem.h"

class SpillwayWater {
public:
    struct Slug { float tail, front; bool feeding; };   // one continuous body of water
    std::vector<Slug> slugs;
    float length = 0;            // length of the chute along the floor (m)
    float timeScale = 0.55f;     // < 1 = slow motion so the eye can follow it

    void build();
    void open();                 // gates opened: new water starts to flow
    void close();                // gates closed: the water drains away
    void fillInstantly();        // start a scene with water already flowing
    void update(float dt, ParticleSystem& spray, ParticleSystem& mist);
    void draw(Renderer& r, float time);
    bool reachesBottom() const;  // is water arriving at the river right now?
    float speedAt(float s) const;

private:
    Mesh sheet[3];
    std::vector<float> profS, profY;   // arc length -> height of the water surface
    Rng rng{77};
};
