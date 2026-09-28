#include "Scene.h"
#include "Layout.h"
#include "Turbine.h"
#include "../core/Shader.h"
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>

namespace {

using glm::vec3;
using glm::mat4;

// A turbine standing on the ground at (x, z), rotor turned to face the viewer.
mat4 turbineOnGround(glm::vec2 xz) {
    float yaw = std::atan2(Layout::CAMERA_EYE.x - xz.x, Layout::CAMERA_EYE.z - xz.y);
    return glm::rotate(glm::translate(mat4(1.0f), {xz.x, terrainHeight(xz.x, xz.y) - 0.3f, xz.y}), yaw, vec3(0, 1, 0));
}

}

Scene::Scene() {
    turbine1Root = turbineOnGround(Layout::TURBINE1_XZ);
    turbine2Root = turbineOnGround(Layout::TURBINE2_XZ);
    wheelRoot = Layout::damRoot() * glm::translate(mat4(1.0f), Dam::wheelMount());
    solarRoot = glm::rotate(glm::translate(mat4(1.0f), {Layout::SOLAR_XZ.x, terrainHeight(Layout::SOLAR_XZ.x, Layout::SOLAR_XZ.y), Layout::SOLAR_XZ.y}),
                            glm::radians(Layout::SOLAR_YAW_DEG), vec3(0, 1, 0));
    boatRoot = glm::rotate(glm::translate(mat4(1.0f), Layout::BOAT_POS), glm::radians(90.0f), vec3(0, 1, 0));   // heading +x

    // Cumulus banks placed where the concept image has them: a large bank
    // upper right, one on the upper left, a small one mid-sky.
    struct CloudSpec { unsigned int seed; vec3 extent; vec3 position; };
    const CloudSpec specs[] = {
        {7u,  {130.0f, 45.0f, 60.0f}, {110.0f, 70.0f, -290.0f}},
        {19u, {80.0f, 32.0f, 45.0f},  {-150.0f, 58.0f, -270.0f}},
        {23u, {55.0f, 18.0f, 30.0f},  {-35.0f, 48.0f, -300.0f}},
        {31u, {40.0f, 12.0f, 25.0f},  {70.0f, 92.0f, -320.0f}},
    };
    clouds.reserve(sizeof(specs) / sizeof(specs[0]));
    for (const CloudSpec& c : specs) {
        clouds.emplace_back(c.seed, c.extent);
        cloudRoots.push_back(glm::translate(mat4(1.0f), c.position));
    }
}

void Scene::draw(const Shader& shader, const MeshLibrary& m) const {
    terrain.draw(shader);
    water.draw(shader);
    dam.draw(shader, m, Layout::GATE_OPENING);
    wheel.draw(shader, m, wheelRoot, 0.3f);
    trees.draw(shader);
    rocks.draw(shader);
    fence.draw(shader);
    solar.draw(shader, solarRoot);
    drawTurbine(shader, m, turbine1Root, 0.35f);
    drawTurbine(shader, m, turbine2Root, 1.35f);
    boat.draw(shader, boatRoot);

    // Clouds sit in the sky itself, so no distance haze on them.
    shader.setFloat("uFogDensity", 0.0f);
    for (size_t i = 0; i < clouds.size(); ++i) clouds[i].draw(shader, cloudRoots[i]);
    shader.setFloat("uFogDensity", Layout::FOG_DENSITY);
}
