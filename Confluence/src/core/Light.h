#pragma once
#include <glm/glm.hpp>

// Mirrors the `Light` struct in phong.vert/phong.frag.
struct Light {
    int type = 0;                        // 0 = directional (sun), 1 = point
    glm::vec3 direction{0.0f, -1.0f, 0.0f};
    glm::vec3 position{0.0f, 0.0f, 0.0f};
    glm::vec3 diffuse{1.0f, 1.0f, 1.0f};
    glm::vec3 specular{1.0f, 1.0f, 1.0f};
    glm::vec3 attenuation{1.0f, 0.0f, 0.0f};
    float enabled = 1.0f;
};
