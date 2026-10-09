// =============================================================================
//  ParticleSystem.h  -  spray, mist, foam bubbles, underwater dust ...
//
//  Each particle is a point with velocity, gravity, drag and a lifetime.
//  Every frame:  velocity += gravity*dt ; position += velocity*dt.
//  For drawing, each particle becomes a small square that always faces the
//  camera ("billboard"), with a soft round texture made in the shader.
// =============================================================================
#pragma once
#include <vector>
#include "Renderer.h"

struct Particle {
    Vec3  pos, vel;
    float life = 0, maxLife = 1;
    float size = 0.5f, grow = 0.0f;   // grow = size change per second
    Vec3  color{1, 1, 1};
    float alpha = 1.0f;
    float gravity = -9.81f;
    float drag = 0.0f;                // 0 = none, 1 = strong air resistance
};

class ParticleSystem {
public:
    std::vector<Particle> particles;
    int  maxParticles = 6000;
    bool additive = false;            // glowing sparks vs normal spray
    bool softNoise = true;            // puffy mist look

    void emit(const Particle& p) { if ((int)particles.size() < maxParticles) particles.push_back(p); }
    void update(float dt);
    void draw(Renderer& r);
    void clear() { particles.clear(); }

private:
    unsigned int vao = 0, vbo = 0;
    std::vector<float> buffer;
};
