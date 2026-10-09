#pragma once
#include <glm/glm.hpp>
#include "../core/Light.h"

class Shader;
struct MeshLibrary;
struct SceneState;
enum class LightSet;

// STEP 3 - The three lights.
//
//   [0] sun   - directional. One direction for the whole plant; it decides
//               which sides of every object are lit.
//   [1] fill  - directional, weak and bluish, from the opposite side: the sky
//               lighting the shaded faces so they are not black.
//   [2] lamp  - POINT light (the movable inspection lamp). Its light gets
//               weaker with distance: 1 / (a0 + a1*d + a2*d*d).
//
// Plus a small global ambient term (in the shader) so nothing is pure black.
struct LightRig {
    Light sun, fill, lamp;
    glm::vec3 globalAmbient{0.0f};
};

namespace Environment {
    // Builds the three lights for this frame from the state: lamp position,
    // and which lights the L key has switched on.
    LightRig buildLightRig(const SceneState& state);

    // Sends the rig to a shader's uLights[] / uGlobalAmbient uniforms.
    void uploadLightRig(const Shader& shader, const LightRig& rig);

    // Draws a small glowing bulb where the lamp is (emission only), so you can
    // see where the point light comes from. Skipped when the lamp is off.
    void drawLampBulb(const Shader& shader, const MeshLibrary& meshes, const LightRig& rig, float bulbSize);

    // The way the sunlight travels (shared by the lights, the sky and the water).
    glm::vec3 sunDirection();

    // Colour of the light that reaches airborne spray: ambient + whichever
    // of sun / fill are switched on (particles are not lit per pixel).
    glm::vec3 particleLight(const LightRig& rig);

    // Start position of the lamp in the plant scene (Enter resets it).
    // The gallery's start position is DemoGallery::lampStartPosition().
    glm::vec3 defaultSceneLampPosition();

    const char* lightSetName(LightSet set);
}
