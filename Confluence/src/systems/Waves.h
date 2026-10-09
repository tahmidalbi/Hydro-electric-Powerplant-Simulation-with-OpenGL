#pragma once
#include <glm/glm.hpp>

class Shader;

// STEP 7 - One wave description for everything that touches the lake.
//
//   height(x, z, t) = LAKE_LEVEL + sum over waves of  A * sin(k * dot(d, p) - w * t + phase)
//
//   d  direction the wave travels (unit vector in x-z)
//   A  amplitude (metres, half the crest-to-trough height)
//   k  = 2*pi / wavelength          (how many radians per metre)
//   w  = k * speed                  (how many radians per second)
//   speed = sqrt(g * wavelength / (2*pi))   -- real deep-water waves: long
//                                              waves travel faster
//
// The SAME numbers are used by
//   * the vertex shaders, which move the lake's vertices (uploadToShader),
//   * the boat, which floats on height() and tilts with the slope,
//   * (later, Step 9) the ray-tracing snapshot.
// There is no second copy of the formula with different numbers.
//
// Stronger wind -> taller waves (amplitudeScale).
namespace Waves {
    constexpr int COUNT = 3;

    // Water surface height at world (x, z) at simulation time t.
    float height(float x, float z, float timeSeconds, float windMetersPerSecond);

    // Unit surface normal there: from the slope, n = normalize(-dh/dx, 1, -dh/dz).
    glm::vec3 normal(float x, float z, float timeSeconds, float windMetersPerSecond);

    // 0.35 in calm air .. 1.6 in a gale.
    float amplitudeScale(float windMetersPerSecond);

    // Sends the wave table (already scaled by the wind) to a shader.
    void uploadToShader(const Shader& shader, float timeSeconds, float windMetersPerSecond);
}
