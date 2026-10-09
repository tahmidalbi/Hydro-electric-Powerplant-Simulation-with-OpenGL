#include "Waves.h"
#include "../core/Shader.h"
#include "../objects/Layout.h"
#include <algorithm>
#include <cmath>
#include <string>

namespace {

constexpr float TWO_PI = 6.28318530718f;
constexpr float GRAVITY = 9.81f;

struct WaveComponent {
    glm::vec2 direction;   // where it travels (normalised below)
    float amplitude;       // metres at amplitudeScale = 1
    float wavelength;      // metres
    float phase;           // radians, so the three crests do not line up
};

// Three gentle swells, mostly travelling with the wind (towards -x) but not
// all exactly parallel, so the surface does not look like corrugated iron.
const WaveComponent WAVES[Waves::COUNT] = {
    {{-1.0f,  0.25f}, 0.13f, 12.0f, 0.0f},
    {{-0.7f, -0.70f}, 0.07f,  7.0f, 1.7f},
    {{-0.3f,  0.95f}, 0.04f,  4.5f, 4.1f},
};

// Per-wave derived numbers: unit direction, k and w.
struct Derived { glm::vec2 d; float k, w; };
Derived derive(const WaveComponent& wave) {
    Derived r;
    r.d = glm::normalize(wave.direction);
    r.k = TWO_PI / wave.wavelength;
    float speed = std::sqrt(GRAVITY * wave.wavelength / TWO_PI);   // deep-water dispersion
    r.w = r.k * speed;
    return r;
}

}

float Waves::amplitudeScale(float wind) {
    return std::clamp(0.35f + wind / 12.0f, 0.35f, 1.6f);
}

float Waves::height(float x, float z, float t, float wind) {
    const float scale = amplitudeScale(wind);
    float h = Layout::LAKE_LEVEL;
    for (const WaveComponent& wave : WAVES) {
        Derived d = derive(wave);
        h += scale * wave.amplitude * std::sin(d.k * (d.d.x * x + d.d.y * z) - d.w * t + wave.phase);
    }
    return h;
}

glm::vec3 Waves::normal(float x, float z, float t, float wind) {
    // Derivative of A*sin(k*(dx*x + dz*z) - w*t + p) with respect to x is
    // A*k*dx*cos(...), and with respect to z is A*k*dz*cos(...).
    const float scale = amplitudeScale(wind);
    float dhdx = 0.0f, dhdz = 0.0f;
    for (const WaveComponent& wave : WAVES) {
        Derived d = derive(wave);
        float c = std::cos(d.k * (d.d.x * x + d.d.y * z) - d.w * t + wave.phase);
        dhdx += scale * wave.amplitude * d.k * d.d.x * c;
        dhdz += scale * wave.amplitude * d.k * d.d.y * c;
    }
    return glm::normalize(glm::vec3(-dhdx, 1.0f, -dhdz));
}

void Waves::uploadToShader(const Shader& shader, float t, float wind) {
    const float scale = amplitudeScale(wind);
    shader.setFloat("uTime", t);   // also drives the flowing-water streaks
    shader.setFloat("uWaveMeanLevel", Layout::LAKE_LEVEL);
    for (int i = 0; i < COUNT; ++i) {
        Derived d = derive(WAVES[i]);
        std::string idx = "[" + std::to_string(i) + "]";
        shader.setVec3("uWaveDirK" + idx, glm::vec3(d.d.x, d.d.y, d.k));                         // direction + k
        shader.setVec3("uWaveAmpOmegaPhase" + idx, glm::vec3(scale * WAVES[i].amplitude, d.w, WAVES[i].phase));
    }
}
