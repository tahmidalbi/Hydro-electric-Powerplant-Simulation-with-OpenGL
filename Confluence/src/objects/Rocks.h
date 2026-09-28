#pragma once
#include "../core/Mesh.h"

class Shader;

// Weathered boulders in the foreground and along the river banks: noise-
// displaced spheres with recomputed (averaged) vertex normals.
class Rocks {
public:
    Rocks();
    void draw(const Shader& shader) const;

private:
    Mesh mesh;
};
