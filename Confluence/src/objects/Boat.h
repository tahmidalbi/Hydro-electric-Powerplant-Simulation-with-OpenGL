#pragma once
#include <glm/glm.hpp>
#include "../core/Mesh.h"

class Shader;

// White motorboat. Bow points along local +Z; the root sits on the waterline.
// Where it is and how it rocks comes from BoatSystem::modelMatrix (Step 7).
class Boat {
public:
    Boat();
    void draw(const Shader& shader, const glm::mat4& root) const;

private:
    Mesh mesh;
};
