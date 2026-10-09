#include "Effects.h"
#include "BoatSystem.h"
#include "HydroSystem.h"
#include "Particles.h"
#include "Waves.h"
#include "WindSystem.h"
#include "../core/SceneState.h"
#include "../objects/Dam.h"
#include "../objects/Layout.h"
#include <cmath>

namespace {

// Dam-local point -> world point, and dam-local direction -> world direction.
glm::vec3 damPoint(float a, float y, float s) { return glm::vec3(Layout::damRoot() * glm::vec4(a, y, s, 1.0f)); }
glm::vec3 damDirection(float a, float y, float s) { return glm::vec3(Layout::damRoot() * glm::vec4(a, y, s, 0.0f)); }

// How many particles to emit this frame for a given rate, keeping the fraction.
int takeCount(float& carry, float ratePerSecond, float dt) {
    carry += ratePerSecond * dt;
    int n = (int)carry;
    carry -= (float)n;
    return n;
}

const glm::vec3 SPRAY_WHITE(0.93f, 0.96f, 0.98f);

}

void Effects::update(ParticleSystem& particles, EffectsState& fx, const PlantState& plant, float dt) {
    if (dt <= 0.0f) return;   // paused: no new particles
    Noise::Rng& rng = fx.rng;
    const float flow = HydroSystem::flowStrength(plant);

    // ---- Spillway spray: billowing clouds where the two chutes hit the basin.
    for (int i = takeCount(fx.sprayCarry, SPRAY_PER_SECOND * flow, dt); i > 0; --i) {
        int bay = rng.next() < 0.5f ? 0 : 1;
        Particle p;
        p.position = damPoint(Dam::bayCentre(bay) + rng.range(-0.5f, 0.5f) * Dam::bayWidth(), rng.range(0.6f, 1.8f), rng.range(10.5f, 13.5f));
        p.velocity = damDirection(rng.range(-1.2f, 1.2f), rng.range(1.2f, 3.6f), rng.range(2.0f, 5.5f)) * (0.6f + 0.6f * flow);
        p.color = SPRAY_WHITE * rng.range(0.92f, 1.0f);
        p.lifetime = rng.range(3.5f, 6.0f);
        p.startSize = rng.range(1.6f, 2.8f);
        p.endSize = rng.range(6.0f, 10.0f);
        p.opacity = 0.16f + 0.10f * flow;
        p.drag = 0.55f;
        p.rise = 0.45f;
        p.windCoupling = 0.12f;
        p.seed = rng.next();
        particles.emit(p);
    }

    // ---- Mist creeping up the lower part of each chute.
    for (int i = takeCount(fx.mistCarry, MIST_PER_SECOND * flow, dt); i > 0; --i) {
        int bay = rng.next() < 0.5f ? 0 : 1;
        float y = rng.range(1.0f, 6.0f);
        float s = 3.84f + 0.536f * (11.5f - y) + rng.range(0.6f, 1.8f);   // on the chute face
        Particle p;
        p.position = damPoint(Dam::bayCentre(bay) + rng.range(-0.45f, 0.45f) * Dam::bayWidth(), y, s);
        p.velocity = damDirection(rng.range(-0.4f, 0.4f), rng.range(0.4f, 1.2f), rng.range(0.6f, 1.8f));
        p.color = SPRAY_WHITE;
        p.lifetime = rng.range(2.5f, 4.0f);
        p.startSize = rng.range(1.0f, 1.6f);
        p.endSize = rng.range(3.0f, 4.5f);
        p.opacity = 0.14f;
        p.drag = 0.8f;
        p.rise = 0.3f;
        p.windCoupling = 0.1f;
        p.seed = rng.next();
        particles.emit(p);
    }

    // ---- Spray where the wheel channel spills into the river.
    const glm::vec3 wheel = Dam::wheelMount();
    for (int i = takeCount(fx.wheelCarry, WHEEL_PER_SECOND * flow, dt); i > 0; --i) {
        Particle p;
        p.position = damPoint(wheel.x + rng.range(-2.8f, 2.8f), rng.range(0.8f, 2.0f), rng.range(16.6f, 18.2f));
        p.velocity = damDirection(rng.range(-0.6f, 0.6f), rng.range(0.6f, 1.8f), rng.range(1.0f, 2.6f));
        p.color = SPRAY_WHITE;
        p.lifetime = rng.range(2.2f, 3.6f);
        p.startSize = rng.range(0.9f, 1.4f);
        p.endSize = rng.range(3.0f, 4.5f);
        p.opacity = 0.22f;
        p.drag = 0.7f;
        p.rise = 0.35f;
        p.windCoupling = 0.1f;
        p.seed = rng.next();
        particles.emit(p);
    }

    // ---- Boat wake: flat foam left behind the stern, spreading sideways.
    const float speed = std::fabs(plant.boatSpeed);
    const glm::vec3 stern = BoatSystem::sternPoint(plant);
    const glm::vec2 f = BoatSystem::forwardVector(plant.boatHeadingRad);
    const glm::vec3 forward(f.x, 0.0f, f.y), side(f.y, 0.0f, -f.x);
    for (int i = takeCount(fx.wakeCarry, WAKE_PER_METRE * speed, dt); i > 0; --i) {
        float sideSign = rng.next() < 0.5f ? -1.0f : 1.0f;
        Particle p;
        // Two arms of a "V": each particle drifts out to its side, so the
        // foam lines spread apart behind the boat as in a real wake.
        p.position = stern + side * (sideSign * rng.range(0.2f, 0.7f)) - forward * rng.range(0.0f, 0.3f);
        p.velocity = side * (sideSign * (0.30f + 0.09f * speed)) - forward * (0.10f * speed);
        p.color = glm::vec3(0.95f, 0.97f, 1.0f);
        p.lifetime = rng.range(4.5f, 6.5f);
        p.startSize = rng.range(0.45f, 0.7f);
        p.endSize = rng.range(1.8f, 2.6f);
        p.opacity = std::min(0.5f, 0.12f + 0.06f * speed);
        p.drag = 0.9f;
        p.flat = true;
        p.seed = rng.next();
        particles.emit(p);
    }

    // Move everything, then keep the wake foam lying ON the moving waves.
    particles.update(dt, WindSystem::direction() * plant.windMetersPerSecond);
    const float t = (float)plant.simulationTimeSeconds;
    for (Particle& p : particles.slots()) {
        if (!p.alive() || !p.flat) continue;
        p.position.y = Waves::height(p.position.x, p.position.z, t, plant.windMetersPerSecond) + 0.06f;
    }
}
