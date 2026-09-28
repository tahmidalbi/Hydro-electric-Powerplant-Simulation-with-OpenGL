#pragma once
#include <glm/glm.hpp>
#include "../core/Mesh.h"

class Shader;

// Flat-bottomed cumulus built from overlapping spheres; `extent` is the
// cloud's width, height and depth. Shaded brighter on top, greyer beneath.
class Cloud {
public:
    Cloud(unsigned int seed, glm::vec3 extent);
    void draw(const Shader& shader, const glm::mat4& root) const;

private:
    Mesh mesh;
};
