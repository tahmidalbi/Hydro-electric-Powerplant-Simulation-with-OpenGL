#pragma once
#include <glm/glm.hpp>
#include "../core/Shader.h"

// Gradient sky drawn as a full-screen triangle behind everything else.
class Sky {
public:
    Sky();
    ~Sky();
    Sky(const Sky&) = delete;
    Sky& operator=(const Sky&) = delete;

    void draw(const glm::mat4& view, const glm::mat4& projection, const glm::vec3& cameraPos) const;

private:
    Shader shader;
    unsigned int vao = 0;
};
