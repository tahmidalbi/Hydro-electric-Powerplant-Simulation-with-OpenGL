#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>

// World layout shared by the terrain and every object, tuned so the default
// view reproduces the proposal's concept image.
// Axes: x right, y up, z towards the default camera (the scene lies along -z).
namespace Layout {
    // View -- aspect matches the concept image so the framing lines up.
    inline constexpr float ASPECT   = 1216.0f / 900.0f;
    inline constexpr float VFOV_DEG = 48.0f;
    // Horizon sits 30% down the frame, as in the photo (pitch ~ -9.6 degrees).
    inline const glm::vec3 CAMERA_EYE{0.0f, 23.0f, 0.0f};
    inline const glm::vec3 CAMERA_TARGET{0.0f, 9.5f, -80.0f};

    // Sky and aerial haze.
    inline const glm::vec3 SKY_HORIZON{0.66f, 0.81f, 0.96f};
    inline const glm::vec3 SKY_ZENITH{0.09f, 0.37f, 0.84f};
    inline const glm::vec3 FOG_COLOR{0.62f, 0.76f, 0.92f};
    inline constexpr float FOG_DENSITY = 0.0029f;

    // Water levels.
    inline constexpr float RIVER_LEVEL = 0.6f;
    inline constexpr float LAKE_LEVEL  = 13.2f;

    // Dam: a straight gravity dam set diagonally across the valley so its
    // right end (powerhouse + wheel) is nearest the viewer.
    inline constexpr float DAM_CREST   = 14.0f;
    inline constexpr float DAM_LENGTH  = 72.0f;
    inline constexpr float DAM_YAW_DEG = -37.9f;
    inline const glm::vec3 DAM_ORIGIN{-22.2f, 0.0f, -101.5f};   // left end, river-bed level

    // Dam-local frame: x along the crest (left end -> right end), y up,
    // z downstream (towards the viewer).
    inline glm::mat4 damRoot() {
        return glm::rotate(glm::translate(glm::mat4(1.0f), DAM_ORIGIN), glm::radians(DAM_YAW_DEG), glm::vec3(0, 1, 0));
    }
    inline glm::vec2 damAxis()   { float t = glm::radians(DAM_YAW_DEG); return {std::cos(t), -std::sin(t)}; }
    inline glm::vec2 damNormal() { float t = glm::radians(DAM_YAW_DEG); return {std::sin(t),  std::cos(t)}; }
    // > 0 downstream (river side), < 0 upstream (lake side).
    inline float damSide(float x, float z)  { return glm::dot(glm::vec2(x - DAM_ORIGIN.x, z - DAM_ORIGIN.z), damNormal()); }
    inline float damAlong(float x, float z) { return glm::dot(glm::vec2(x - DAM_ORIGIN.x, z - DAM_ORIGIN.z), damAxis()); }

    // Left shore as x(z): the lake's shore recedes left beyond the dam, the
    // valley is widest at the dam (its whole left half stays in view) and the
    // river narrows towards the viewer.
    inline float leftBankX(float z) {
        if (z < -100.0f) return -14.4f + 0.35f * (z + 100.0f);
        if (z < -80.0f)  return -14.0f + 0.02f * (z + 80.0f);
        if (z < -35.0f)  return -14.0f + 0.267f * (z + 80.0f);
        return -2.0f + 0.19f * (z + 35.0f);
    }
    inline float rightBankX(float z) { return 27.5f + 0.20f * (z + 40.0f); }

    // Placements (x, z) on the landscape.
    inline const glm::vec2 TURBINE1_XZ{-28.5f, -65.0f};
    inline const glm::vec2 TURBINE2_XZ{-31.0f, -106.0f};
    // Array faces the viewer's front-right, so it recedes to the right as in the photo.
    inline const glm::vec2 SOLAR_XZ{-8.0f, -16.5f};
    inline constexpr float SOLAR_YAW_DEG = 53.0f;
    inline const glm::vec3 BOAT_POS{55.0f, LAKE_LEVEL, -134.0f};

    // Spillway gates: 0 = shut, 1 = fully raised.
    inline constexpr float GATE_OPENING = 0.45f;
}
