#pragma once
#include "../core/Mesh.h"

class Shader;

// Reservoir lake behind the dam; the river below it, churned white and
// turbulent where the spillways and the water wheel discharge into it; the
// wheel channel's outflow; and the spray hanging over the falls.
class Water {
public:
    Water();
    void draw(const Shader& shader) const;          // opaque surfaces
    void drawFalls(const Shader& shader) const;     // translucent sheet out of the wheel channel
    void drawSpray(const Shader& shader) const;     // soft mist, drawn last

private:
    Mesh lake;
    Mesh river;
    Mesh wheelFall;
    Mesh spray;
};
