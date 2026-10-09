#include "FrameRenderer.h"
#include "../App.h"
#include "../objects/Layout.h"
#include "../systems/Environment.h"
#include "../systems/Waves.h"
#include "../ui/HudOverlay.h"
#include <glad/gl.h>
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>

namespace {
const std::vector<std::string> COMMON = {"shaders/lighting_common.glsl", "shaders/surface_common.glsl"};
}

FrameRenderer::FrameRenderer()
    : smoothShader("shaders/phong.vert", "shaders/phong.frag", "", COMMON)
    , flatShader("shaders/flat.vert", "shaders/flat.frag", "shaders/flat.geom", COMMON) {}

void FrameRenderer::render(int width, int height, const App& app, float fps, float realTimeSeconds) {
    const ViewSettings& view = app.state.view;
    const PlantState& plant = app.state.plant;
    const Camera& camera = app.activeCamera();
    const glm::vec3 eye = camera.getPosition();
    const glm::vec3 sun = Environment::sunDirection();

    glViewport(0, 0, width, height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // clipPoint = Projection * View * Model * localPoint
    float aspect = height > 0 ? (float)width / (float)height : 1.0f;
    glm::mat4 projection = camera.projection(aspect, Layout::VFOV_DEG, view.orthographic);
    glm::mat4 viewMatrix = camera.getViewMatrix();

    // The sky always uses a perspective matrix: in orthographic view every
    // pixel would otherwise get the same ray direction (one flat colour).
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    sky.draw(viewMatrix, camera.projection(aspect, Layout::VFOV_DEG, false), eye, sun);

    // Step 2 proof view: X shows every triangle edge.
    if (view.wireframe) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // Step 4: Flat uses its own program; Gouraud and Phong share one.
    const Shader& shader = (view.shading == ShadingMode::Flat) ? flatShader : smoothShader;
    shader.use();
    shader.setMat4("uProjection", projection);
    shader.setMat4("uView", viewMatrix);
    shader.setVec3("uViewPos", eye);
    shader.setInt("uShadingMode", (int)view.shading);
    shader.setInt("uTermMask", (int)view.terms);
    shader.setInt("uUseBlinn", view.useBlinn ? 1 : 0);
    shader.setInt("uDebugView", view.showNormals ? 1 : 0);
    shader.setVec3("uFogColor", Layout::FOG_COLOR);
    // No haze in the gallery: comparisons must not depend on distance.
    shader.setFloat("uFogDensity", view.galleryActive ? 0.0f : Layout::FOG_DENSITY);
    // Sky colours for the water's reflection, and the wave table (Step 7).
    shader.setVec3("uSkyHorizon", Layout::SKY_HORIZON);
    shader.setVec3("uSkyZenith", Layout::SKY_ZENITH);
    shader.setVec3("uSunDirection", sun);
    Waves::uploadToShader(shader, (float)plant.simulationTimeSeconds, plant.windMetersPerSecond);
    shader.setInt("uWaveMode", 0);
    shader.setInt("uFlowMode", 0);
    shader.setVec3("uHighlight", glm::vec3(0.0f));

    // Step 3: sun + sky fill + movable point lamp.
    LightRig rig = Environment::buildLightRig(app.state);
    Environment::uploadLightRig(shader, rig);

    if (view.galleryActive) {
        gallery.draw(shader, meshes, app.state);
        Environment::drawLampBulb(shader, meshes, rig, 0.45f);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    } else {
        // Step 8: the selected object glows with a slow warm pulse.
        glm::vec3 glow = glm::vec3(1.0f, 0.72f, 0.25f) * (0.16f + 0.10f * std::sin(realTimeSeconds * 4.0f));
        scene.draw(shader, meshes, app.state, glow);
        Environment::drawLampBulb(shader, meshes, rig, 0.9f);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        // Steps 6-7: spray, mist and wake -- after all solid geometry.
        particleRenderer.draw(app.particles, viewMatrix, projection, eye, Environment::particleLight(rig),
                              Layout::FOG_COLOR, Layout::FOG_DENSITY);
    }

    // Step 8: the on-screen panel, on top of everything.
    hud.begin(width, height);
    HudOverlay::draw(hud, app, galleryDetailSummary(app), fps);
    hud.end();
}

std::string FrameRenderer::galleryDetailSummary(const App& app) const {
    return gallery.detailSummary(app.state.view.galleryDetail);
}
