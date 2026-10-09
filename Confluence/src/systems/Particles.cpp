#include "Particles.h"
#include <algorithm>
#include <cmath>

float Particle::currentSize() const {
    float t = lifeFraction();
    // Grows quickly at first, then slowly: sqrt shape.
    return startSize + (endSize - startSize) * std::sqrt(std::clamp(t, 0.0f, 1.0f));
}

float Particle::currentOpacity() const {
    float t = lifeFraction();
    float fadeIn = std::clamp(age / 0.2f, 0.0f, 1.0f);          // 0.2 s: no popping into view, no gap behind the boat
    float fadeOut = 1.0f - std::clamp((t - 0.35f) / 0.65f, 0.0f, 1.0f);
    return opacity * fadeIn * fadeOut * fadeOut;
}

ParticleSystem::ParticleSystem() : pool(CAPACITY) {}

void ParticleSystem::emit(const Particle& p) {
    for (int n = 0; n < CAPACITY; ++n) {
        int i = (searchStart + n) % CAPACITY;
        if (!pool[i].alive()) {
            pool[i] = p;
            pool[i].age = 0.0f;
            searchStart = (i + 1) % CAPACITY;
            return;
        }
    }
    // Pool full: drop it. The pool is the upper bound on memory and work.
}

void ParticleSystem::update(float dt, const glm::vec3& wind) {
    if (dt <= 0.0f) return;   // paused: everything freezes in place
    for (Particle& p : pool) {
        if (!p.alive()) continue;
        p.velocity += (wind * p.windCoupling + glm::vec3(0.0f, p.rise, 0.0f)) * dt;
        p.velocity *= std::exp(-p.drag * dt);   // frame-rate independent drag
        p.position += p.velocity * dt;
        p.age += dt;
    }
}

void ParticleSystem::clear() {
    for (Particle& p : pool) p.lifetime = 0.0f;
}

int ParticleSystem::aliveCount() const {
    int n = 0;
    for (const Particle& p : pool) n += p.alive() ? 1 : 0;
    return n;
}
