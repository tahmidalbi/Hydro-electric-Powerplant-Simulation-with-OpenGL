#pragma once
#include <glm/glm.hpp>

// Mirrors the `Light` struct in shaders/lighting_common.glsl.
//
//   directional (type 0): parallel rays from far away (the sun). Only a
//                         direction; no position, no fall-off with distance.
//   point       (type 1): rays spread out from one position (a lamp). Its
//                         contribution falls off with distance d:
//                         attenuation = 1 / (a0 + a1*d + a2*d*d)
struct Light {
    int type = 0;                        // 0 = directional (sun), 1 = point
    glm::vec3 direction{0.0f, -1.0f, 0.0f};
    glm::vec3 position{0.0f, 0.0f, 0.0f};
    glm::vec3 diffuse{1.0f, 1.0f, 1.0f};
    glm::vec3 specular{1.0f, 1.0f, 1.0f};
    glm::vec3 attenuation{1.0f, 0.0f, 0.0f};   // (a0, a1, a2)
    float enabled = 1.0f;
};

// The shader holds exactly this many lights: [0] sun, [1] sky fill, [2] inspection lamp.
constexpr int MAX_LIGHTS = 3;
