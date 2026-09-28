#pragma once
#include <glm/glm.hpp>
#include "../core/Mesh.h"

class Shader;

// White motorboat with a wake trailing behind it. Bow points along local +Z;
// the root sits on the waterline.
class Boat {
public:
    Boat();
    void draw(const Shader& shader, const glm::mat4& root) const;

private:
    Mesh mesh;
};
