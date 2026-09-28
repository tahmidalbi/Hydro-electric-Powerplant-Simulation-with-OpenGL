#pragma once
#include "../core/Mesh.h"

class Shader;

// Conifer forest scattered over the hillsides (clumped by noise, kept off
// water, cliffs, the dam and the foreground), baked into one mesh.
class Trees {
public:
    Trees();
    void draw(const Shader& shader) const;

private:
    Mesh mesh;
};
