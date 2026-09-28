#pragma once
#include <glm/glm.hpp>
#include "../core/Mesh.h"

class Shader;

// Ground-mounted array of framed blue PV panels (individual cells visible)
// on an aluminium rack, tilted towards the sun. Faces the root's local +Z.
class SolarArray {
public:
    SolarArray();
    void draw(const Shader& shader, const glm::mat4& root) const;

private:
    Mesh mesh;
};
