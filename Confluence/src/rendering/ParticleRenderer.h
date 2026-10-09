#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "../core/Shader.h"

class ParticleSystem;

// Draws every live particle in ONE draw call.
//
// Each frame the live particles are
//   1. sorted far -> near (transparent things must be drawn back to front,
//      otherwise a near puff would hide one behind it incorrectly),
//   2. written as 4 vertices each into a buffer that is re-filled every frame,
//   3. drawn with alpha blending, depth TEST on (the dam still hides them) and
//      depth WRITE off (puffs do not hide each other's soft edges).
class ParticleRenderer {
public:
    ParticleRenderer();
    ~ParticleRenderer();
    ParticleRenderer(const ParticleRenderer&) = delete;
    ParticleRenderer& operator=(const ParticleRenderer&) = delete;

    void draw(const ParticleSystem& particles, const glm::mat4& view, const glm::mat4& projection,
              const glm::vec3& cameraPos, const glm::vec3& lightColor,
              const glm::vec3& fogColor, float fogDensity);

private:
    struct GpuVertex {
        glm::vec3 centre;
        glm::vec2 corner;
        glm::vec4 color;          // rgb + opacity
        glm::vec3 sizeFlatSeed;
    };
    Shader shader;
    unsigned int vao = 0, vbo = 0, ebo = 0;
    std::vector<GpuVertex> vertices;   // reused every frame (no reallocation)
    std::vector<int> order;
};
