#pragma once
#include <vector>
#include <glm/glm.hpp>

// A bounded pool of simple particles (spray, mist, boat wake).
//
// Every particle has: position, velocity, age, lifetime, size (grows from
// startSize to endSize), opacity (fades in quickly, then out with age), colour,
// and whether it lies FLAT on the water (wake foam) or always FACES the camera
// (a billboard: spray and mist).
//
// The pool never grows: CAPACITY slots are made once. A new particle takes a
// free (dead) slot; when every slot is busy the new one is simply dropped.
// So memory is fixed no matter how long the program runs.
//
// update(dt):  velocity += (wind pull + rise) * dt,  velocity *= drag,
//              position += velocity * dt,            age += dt
// A particle whose age passes its lifetime is dead and its slot is reused.
struct Particle {
    glm::vec3 position{0.0f};
    glm::vec3 velocity{0.0f};
    glm::vec3 color{1.0f};
    float age = 0.0f;
    float lifetime = 0.0f;          // 0 = free slot
    float startSize = 1.0f;
    float endSize = 1.0f;
    float opacity = 1.0f;           // peak opacity
    float drag = 0.5f;              // fraction of velocity lost per second
    float rise = 0.0f;              // upward acceleration (warm mist rises)
    float windCoupling = 0.0f;      // how strongly the wind carries it
    bool flat = false;              // lies on the water instead of facing the camera
    float seed = 0.0f;              // random number for a little shape variety

    bool alive() const { return age < lifetime; }
    float lifeFraction() const { return lifetime > 0.0f ? age / lifetime : 1.0f; }
    float currentSize() const;
    float currentOpacity() const;
};

class ParticleSystem {
public:
    static constexpr int CAPACITY = 3000;

    ParticleSystem();

    // Puts a particle into a free slot (does nothing if the pool is full).
    void emit(const Particle& p);

    // Ages and moves every live particle. dt = 0 (paused) freezes them.
    void update(float dt, const glm::vec3& windVelocity);

    void clear();

    const std::vector<Particle>& slots() const { return pool; }
    std::vector<Particle>& slots() { return pool; }
    int aliveCount() const;

private:
    std::vector<Particle> pool;
    int searchStart = 0;   // where to start looking for a free slot (round robin)
};
