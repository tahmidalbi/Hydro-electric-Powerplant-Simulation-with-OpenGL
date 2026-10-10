// =============================================================================
//  TransmissionLine.h  -  the 220 kV power line leaving the switchyard
//  (video 2:55 - 3:00): lattice steel towers marching down the valley with
//  six conductors (two circuits) hanging between them in sagging curves
//  (catenaries), plus an earth wire on top.
//  When the breaker closes, the energy runs along the wires as a glow.
// =============================================================================
#pragma once
#include <vector>
#include "../render/Renderer.h"
#include "FlowLines.h"

class TransmissionLine {
public:
    static const int TOWERS = 10;
    float lineLength = 0;                 // length of the conductors (m)

    void build();
    void draw(Renderer& r);
    void drawGlow(Renderer& r, float front, float strength, float speed);
    Vec3 towerBase(int i) const { return bases[i]; }
    Vec3 pointAlong(float s) const;       // a point on the first conductor, s metres from the yard

private:
    Vec3 bases[TOWERS];
    std::vector<Vec3> guide;              // centre line used for the camera flight
    Mesh towers, insulators, wires;
    FlowLines glow;
};
