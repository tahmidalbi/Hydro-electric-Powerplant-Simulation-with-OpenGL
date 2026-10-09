#include "Scene.h"
#include "Layout.h"
#include "Placement.h"
#include "Turbine.h"
#include "../core/SceneState.h"
#include "../core/Shader.h"
#include "../core/Transform.h"
#include "../systems/BoatSystem.h"
#include "../systems/HydroSystem.h"
#include "../systems/WindSystem.h"
#include <glad/gl.h>

namespace {
using glm::vec3;
using glm::mat4;
using Transform::translate;
}

Scene::Scene() {
    // Cumulus banks placed where the concept image has them: a large bank
    // upper right, one on the upper left, a small one mid-sky. `parallax` < 1
    // makes the more distant banks drift a little slower (Step 5).
    const CloudSpec specs[] = {
        {7u,  {130.0f, 45.0f, 60.0f}, {110.0f, 70.0f, -290.0f}, 1.00f},
        {19u, {80.0f, 32.0f, 45.0f},  {-150.0f, 58.0f, -270.0f}, 1.08f},
        {23u, {55.0f, 18.0f, 30.0f},  {-35.0f, 48.0f, -300.0f}, 0.92f},
        {31u, {40.0f, 12.0f, 25.0f},  {70.0f, 92.0f, -320.0f}, 0.85f},
    };
    clouds.reserve(sizeof(specs) / sizeof(specs[0]));
    for (const CloudSpec& c : specs) {
        clouds.emplace_back(c.seed, c.extent);
        cloudSpecs.push_back(c);
    }
}

// Drawing only READS the state. It never changes it -- that is the update
// step's job. `highlight` is the glow added to the selected object (Step 8).
void Scene::draw(const Shader& shader, const MeshLibrary& m, const SceneState& state, const vec3& highlight) const {
    const PlantState& plant = state.plant;
    const Selectable selected = state.view.selected;
    // glow(X): the next draws glow only if X is the selected object
    // (glow(None) always switches the glow off).
    auto glow = [&](Selectable who) {
        bool on = who != Selectable::None && who == selected;
        shader.setVec3("uHighlight", on ? highlight : vec3(0.0f));
    };
    glow(Selectable::None);

    terrain.draw(shader);
    water.drawSurfaces(shader, HydroSystem::flowStrength(plant));
    dam.draw(shader);
    glow(Selectable::Gate);
    dam.drawGates(shader, m, plant.gateOpening);
    glow(Selectable::Wheel);
    wheel.draw(shader, m, Placement::wheelRoot(), plant.wheelAngleRad);
    glow(Selectable::None);
    trees.draw(shader);
    rocks.draw(shader);
    fence.draw(shader);
    glow(Selectable::Solar);
    solar.draw(shader, Placement::solarRoot(), plant.solarTiltDeg);
    glow(Selectable::Turbine1);
    drawTurbine(shader, m, Placement::turbineRoot(0), plant.rotorAngleRad[0]);
    glow(Selectable::Turbine2);
    drawTurbine(shader, m, Placement::turbineRoot(1), plant.rotorAngleRad[1]);
    glow(Selectable::Boat);
    boat.draw(shader, BoatSystem::modelMatrix(plant));
    glow(Selectable::None);

    // Clouds drift with the wind and wrap far outside the view (Step 5).
    // They sit in the sky itself, so no distance haze on them.
    shader.setFloat("uFogDensity", 0.0f);
    for (size_t i = 0; i < clouds.size(); ++i) {
        const CloudSpec& c = cloudSpecs[i];
        vec3 position = c.position;
        position.x = WindSystem::cloudX(c.position.x, plant.cloudDriftMeters, c.parallax);
        clouds[i].draw(shader, translate(position));
    }
    shader.setFloat("uFogDensity", Layout::FOG_DENSITY);

    // Transparent pass, after everything opaque. Both falls follow the gate.
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    dam.drawSpillwayWater(shader, plant.gateOpening);
    water.drawFalls(shader, HydroSystem::flowStrength(plant));
    glDisable(GL_BLEND);
}
