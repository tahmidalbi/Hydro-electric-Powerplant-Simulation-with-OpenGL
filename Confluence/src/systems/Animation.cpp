#include "Animation.h"
#include "BoatSystem.h"
#include "HydroSystem.h"
#include "WindSystem.h"
#include "../core/SceneState.h"
#include "../objects/SolarArray.h"
#include <algorithm>

void Animation::update(PlantState& plant, float dt) {
    plant.simulationTimeSeconds += dt;

    WindSystem::update(plant, dt);    // Step 5
    HydroSystem::update(plant, dt);   // Step 6
    BoatSystem::update(plant, dt);    // Step 7

    // Step 7: the solar tracker's hinge has limits, computed from the
    // terrain so the panel edges never dig into the slope.
    float minTilt, maxTilt;
    SolarArray::tiltLimits(minTilt, maxTilt);
    plant.solarTiltDeg = std::clamp(plant.solarTiltDeg, minTilt, maxTilt);
}

void Animation::reset(PlantState& plant) {
    plant = PlantState{};   // the default member values ARE the documented start pose

    plant.rotorSpeedRadPerSecond[0] = WindSystem::targetRotorSpeed(plant.windMetersPerSecond);
    plant.rotorSpeedRadPerSecond[1] = plant.rotorSpeedRadPerSecond[0] * WindSystem::SECOND_TURBINE_FACTOR;
    plant.wheelSpeedRadPerSecond = HydroSystem::targetWheelSpeed(plant);
    plant.boatSpeed = BoatSystem::PATROL_SPEED;
    BoatSystem::floatOnWaves(plant);
}
