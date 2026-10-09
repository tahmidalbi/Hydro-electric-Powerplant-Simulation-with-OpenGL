#include "WindSystem.h"
#include "../core/SceneState.h"
#include <algorithm>
#include <cmath>

namespace {

constexpr float TWO_PI = 6.28318530718f;
constexpr float RPM_TO_RAD_PER_SECOND = TWO_PI / 60.0f;

// Time constants (seconds): how quickly each quantity follows its target.
constexpr float WIND_EASE_SECONDS = 1.5f;
constexpr float ROTOR_INERTIA_SECONDS = 3.0f;   // a 40-tonne rotor does not change speed instantly

// Moves `value` part of the way towards `target`. With this form the result
// after one second is the same whether that second had 30 frames or 144.
float easeTowards(float value, float target, float dt, float timeConstant) {
    return value + (target - value) * (1.0f - std::exp(-dt / timeConstant));
}

float wrapAngle(float a) {
    a = std::fmod(a, TWO_PI);
    return a < 0.0f ? a + TWO_PI : a;
}

}

glm::vec3 WindSystem::direction() { return {-1.0f, 0.0f, 0.0f}; }

float WindSystem::targetRotorSpeed(float wind) {
    if (wind < CUT_IN || wind > CUT_OUT) return 0.0f;
    float t = std::clamp((wind - CUT_IN) / (RATED - CUT_IN), 0.0f, 1.0f);
    float rpm = MIN_RPM + (MAX_RPM - MIN_RPM) * t;
    return rpm * RPM_TO_RAD_PER_SECOND;
}

float WindSystem::cloudSpeed(float wind) { return 0.5f + 0.55f * wind; }

float WindSystem::cloudX(float startX, float drift, float parallax) {
    // Moves towards -x (with the wind), then wraps: x in [-WRAP, WRAP).
    float x = startX - drift * parallax + CLOUD_WRAP;
    x = std::fmod(x, 2.0f * CLOUD_WRAP);
    if (x < 0.0f) x += 2.0f * CLOUD_WRAP;
    return x - CLOUD_WRAP;
}

void WindSystem::update(PlantState& plant, float dt) {
    plant.windTargetMetersPerSecond = std::clamp(plant.windTargetMetersPerSecond, 0.0f, MAX_WIND);
    plant.windMetersPerSecond = easeTowards(plant.windMetersPerSecond, plant.windTargetMetersPerSecond, dt, WIND_EASE_SECONDS);

    const float target = targetRotorSpeed(plant.windMetersPerSecond);
    const float factor[2] = {1.0f, SECOND_TURBINE_FACTOR};
    for (int i = 0; i < 2; ++i) {
        plant.rotorSpeedRadPerSecond[i] = easeTowards(plant.rotorSpeedRadPerSecond[i], target * factor[i], dt, ROTOR_INERTIA_SECONDS);
        // Integrate the speed: angle += speed * dt. (Never speed * totalTime:
        // that would make the rotor jump whenever the speed changes.)
        plant.rotorAngleRad[i] = wrapAngle(plant.rotorAngleRad[i] + plant.rotorSpeedRadPerSecond[i] * dt);
    }

    plant.cloudDriftMeters += cloudSpeed(plant.windMetersPerSecond) * dt;
    // Keep the number small; the wrap period of every cloud divides this.
    if (plant.cloudDriftMeters > 1.0e6f) plant.cloudDriftMeters -= 1.0e6f;
}
