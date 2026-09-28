#pragma once
#include <cmath>
#include <cstdint>

// Deterministic noise helpers: the landscape, tree scatter, rocks and clouds
// are all generated from these, so every run builds the identical scene.
namespace Noise {

inline float hash(int x, int z, int seed) {
    uint32_t h = static_cast<uint32_t>(x) * 374761393u
               + static_cast<uint32_t>(z) * 668265263u
               + static_cast<uint32_t>(seed) * 2246822519u;
    h = (h ^ (h >> 13)) * 1274126177u;
    h ^= h >> 16;
    return static_cast<float>(h & 0xFFFFFFu) / 16777215.0f;
}

// Smooth value noise in [0, 1].
inline float value(float x, float z, int seed) {
    float x0 = std::floor(x), z0 = std::floor(z);
    int xi = static_cast<int>(x0), zi = static_cast<int>(z0);
    float fx = x - x0, fz = z - z0;
    float ux = fx * fx * (3.0f - 2.0f * fx), uz = fz * fz * (3.0f - 2.0f * fz);
    float a = hash(xi, zi, seed), b = hash(xi + 1, zi, seed);
    float c = hash(xi, zi + 1, seed), d = hash(xi + 1, zi + 1, seed);
    float top = a + (b - a) * ux, bottom = c + (d - c) * ux;
    return top + (bottom - top) * uz;
}

// Fractal sum of value-noise octaves, normalised to [0, 1].
inline float fbm(float x, float z, int octaves, int seed) {
    float sum = 0.0f, amp = 0.5f, norm = 0.0f;
    for (int i = 0; i < octaves; ++i) {
        sum += amp * value(x, z, seed + i * 17);
        norm += amp;
        x *= 2.0f; z *= 2.0f; amp *= 0.5f;
    }
    return sum / norm;
}

// xorshift32 generator for scattering objects.
struct Rng {
    uint32_t s;
    explicit Rng(uint32_t seed) : s(seed ? seed : 1u) {}
    float next() {
        s ^= s << 13; s ^= s >> 17; s ^= s << 5;
        return static_cast<float>(s & 0xFFFFFFu) / 16777215.0f;
    }
    float range(float a, float b) { return a + (b - a) * next(); }
};

}
