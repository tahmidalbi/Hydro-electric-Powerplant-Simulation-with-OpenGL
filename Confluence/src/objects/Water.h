#pragma once
#include "../core/Mesh.h"

class Shader;

// Reservoir lake behind the dam, the turquoise river below it, and the white
// churn where the spillways and the water wheel discharge.
class Water {
public:
    Water();
    void draw(const Shader& shader) const;

private:
    Mesh lake;
    Mesh river;
    Mesh foam;
};
