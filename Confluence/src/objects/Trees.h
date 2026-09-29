#pragma once
#include "../core/Mesh.h"

class Shader;

// Mixed forest over the hillsides: tall layered spruces (ragged branch tiers,
// dark inside, lighter at the tips) and rounded broadleaf bushes, clumped by
// noise and kept off water, cliffs, the dam and the foreground. A handful of
// prototype trees are built once and instanced into one baked mesh.
class Trees {
public:
    Trees();
    void draw(const Shader& shader) const;

private:
    Mesh mesh;
};
