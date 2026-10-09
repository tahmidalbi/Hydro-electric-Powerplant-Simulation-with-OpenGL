#include "HydroSystem.h"
#include "../core/SceneState.h"
#include <algorithm>
#include <cmath>

namespace {
constexpr float TWO_PI = 6.28318530718f;
constexpr float WHEEL_INERTIA_SECONDS = 2.5f;
}

float HydroSystem::flowStrength(const PlantState& plant) { return std::clamp(plant.gateOpening, 0.0f, 1.0f); }

float HydroSystem::targetWheelSpeed(const PlantState& plant) { return flowStrength(plant) * WHEEL_MAX_SPEED; }

void HydroSystem::toggleGate(PlantState& plant) {
    plant.gateAutoDemo = false;   // any gate input takes back manual control
    plant.gateTarget = plant.gateTarget > 0.5f ? 0.0f : 1.0f;
}

void HydroSystem::nudgeTarget(PlantState& plant, float delta) {
    plant.gateAutoDemo = false;
    plant.gateTarget = std::clamp(plant.gateTarget + delta, 0.0f, 1.0f);
}

void HydroSystem::update(PlantState& plant, float dt) {
    // Automatic demonstration: the target follows a slow wave between shut
    // and open, so every linked effect can be shown hands-free.
    if (plant.gateAutoDemo) {
        float phase = (float)plant.simulationTimeSeconds * TWO_PI / AUTO_DEMO_PERIOD_SECONDS;
        plant.gateTarget = 0.5f - 0.5f * std::cos(phase);
    }

    // The gate moves towards the target, but never faster than its motors allow,
    // and never outside its slot (0 .. 1).
    float step = GATE_SPEED_PER_SECOND * dt;
    float difference = plant.gateTarget - plant.gateOpening;
    plant.gateOpening += std::clamp(difference, -step, step);
    plant.gateOpening = std::clamp(plant.gateOpening, 0.0f, 1.0f);

    // The wheel speeds up / slows down smoothly towards what the flow supports,
    // then its angle is integrated (it turns so its bottom moves downstream).
    float target = targetWheelSpeed(plant);
    plant.wheelSpeedRadPerSecond += (target - plant.wheelSpeedRadPerSecond) * (1.0f - std::exp(-dt / WHEEL_INERTIA_SECONDS));
    plant.wheelAngleRad = std::fmod(plant.wheelAngleRad - plant.wheelSpeedRadPerSecond * dt, TWO_PI);
}
