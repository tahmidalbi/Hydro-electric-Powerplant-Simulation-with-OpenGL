#pragma once
#include "../core/Mesh.h"

class Shader;

// Wooden post-and-rail fence running down the hillside from the solar array
// towards the river, following the ground.
class Fence {
public:
    Fence();
    void draw(const Shader& shader) const;

private:
    Mesh mesh;
};
