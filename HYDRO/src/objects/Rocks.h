// =============================================================================
//  Rocks.h  -  lumpy boulders: a sphere whose vertices are pushed in/out by
//  noise, with a flattened bottom so it sits on the ground.
// =============================================================================
#pragma once
#include "../core/Mesh.h"

void addRock(MeshBuilder& b, const Vec3& centre, float radius, unsigned int seed);
