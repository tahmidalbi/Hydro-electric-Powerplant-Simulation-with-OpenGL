#pragma once
#include <glm/glm.hpp>
#include "../core/Shader.h"

// Gradient sky drawn as a full-screen triangle behind everything else.
// Its colour function (skyColour in surface_common.glsl) is shared with the
// water, so the lake reflects exactly this sky.
class Sky {
public:
    Sky();
    ~Sky();
    Sky(const Sky&) = delete;
    Sky& operator=(const Sky&) = delete;

    // `projection` must be a PERSPECTIVE matrix (even in orthographic view),
    // so every pixel still gets its own ray direction.
    void draw(const glm::mat4& view, const glm::mat4& projection, const glm::vec3& cameraPos,
              const glm::vec3& sunDirection) const;

private:
    Shader shader;
    unsigned int vao = 0;
};
