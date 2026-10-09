#include "Environment.h"
#include "../core/Material.h"
#include "../core/MeshLibrary.h"
#include "../core/Render.h"
#include "../core/SceneState.h"
#include "../core/Shader.h"
#include "../core/Transform.h"
#include "../objects/Layout.h"

namespace {

// Background light reaching every surface equally. Kept low enough that the
// diffuse term still clearly shapes the objects.
const glm::vec3 GLOBAL_AMBIENT(0.40f, 0.44f, 0.50f);

// Sun high behind the viewer's left shoulder, so the faces of the dam,
// turbines, panels and hillside turned towards the default camera are sunlit.
const glm::vec3 SUN_DIRECTION = glm::normalize(glm::vec3(0.20f, -0.85f, -0.50f));
const glm::vec3 SUN_COLOR(1.00f, 0.97f, 0.90f);

// Sky fill: soft light from the opposite side so shaded faces aren't black.
const glm::vec3 FILL_DIRECTION = glm::normalize(glm::vec3(-0.6f, -0.5f, 0.6f));
const glm::vec3 FILL_COLOR(0.28f, 0.31f, 0.36f);

// Warm inspection lamp. Attenuation (a0, a1, a2) = (1, 0.045, 0.0075):
// full strength at the bulb, about half at 9 units, about a quarter at 17.
const glm::vec3 LAMP_DIFFUSE(1.30f, 1.08f, 0.74f);
const glm::vec3 LAMP_SPECULAR(1.60f, 1.50f, 1.30f);
const glm::vec3 LAMP_ATTENUATION(1.0f, 0.045f, 0.0075f);

}

LightRig Environment::buildLightRig(const SceneState& state) {
    LightRig rig;
    rig.globalAmbient = GLOBAL_AMBIENT;

    rig.sun.type = 0;
    rig.sun.direction = SUN_DIRECTION;
    rig.sun.diffuse = SUN_COLOR;
    rig.sun.specular = {1.0f, 1.0f, 1.0f};

    rig.fill.type = 0;
    rig.fill.direction = FILL_DIRECTION;
    rig.fill.diffuse = FILL_COLOR;
    rig.fill.specular = {0.0f, 0.0f, 0.0f};   // the sky gives no sharp highlight

    rig.lamp.type = 1;
    rig.lamp.position = state.activeLampPosition();
    rig.lamp.diffuse = LAMP_DIFFUSE;
    rig.lamp.specular = LAMP_SPECULAR;
    rig.lamp.attenuation = LAMP_ATTENUATION;

    // L key: which lights are on.
    const LightSet set = state.view.lights;
    rig.sun.enabled  = (set == LightSet::All || set == LightSet::SunOnly)  ? 1.0f : 0.0f;
    rig.fill.enabled = (set == LightSet::All || set == LightSet::FillOnly) ? 1.0f : 0.0f;
    rig.lamp.enabled = (set == LightSet::All || set == LightSet::LampOnly) ? 1.0f : 0.0f;
    return rig;
}

void Environment::uploadLightRig(const Shader& shader, const LightRig& rig) {
    shader.setVec3("uGlobalAmbient", rig.globalAmbient);
    setLightUniforms(shader, 0, rig.sun);
    setLightUniforms(shader, 1, rig.fill);
    setLightUniforms(shader, 2, rig.lamp);
}

void Environment::drawLampBulb(const Shader& shader, const MeshLibrary& meshes, const LightRig& rig, float bulbSize) {
    if (rig.lamp.enabled < 0.5f) return;
    drawMesh(shader, meshes.sphere,
             Transform::translate(rig.lamp.position) * Transform::scale(glm::vec3(bulbSize)),
             Materials::LAMP_GLOW);
}

glm::vec3 Environment::sunDirection() { return SUN_DIRECTION; }

glm::vec3 Environment::particleLight(const LightRig& rig) {
    glm::vec3 light = rig.globalAmbient * 0.85f;
    light += rig.sun.diffuse * rig.sun.enabled * 0.65f;
    light += rig.fill.diffuse * rig.fill.enabled * 0.5f;
    return glm::min(light, glm::vec3(1.05f));
}

glm::vec3 Environment::defaultSceneLampPosition() {
    // In front of the powerhouse (dam-local x 58.5, 9 m up, 5 m in front of
    // its wall -- far enough from the tall dam face not to wash it out),
    // turned into world space by the dam's root matrix.
    return glm::vec3(Layout::damRoot() * glm::vec4(58.5f, 9.0f, 18.0f, 1.0f));
}

const char* Environment::lightSetName(LightSet set) {
    switch (set) {
        case LightSet::All:      return "sun+fill+lamp";
        case LightSet::SunOnly:  return "sun only";
        case LightSet::FillOnly: return "fill only";
        case LightSet::LampOnly: return "lamp only";
    }
    return "?";
}
