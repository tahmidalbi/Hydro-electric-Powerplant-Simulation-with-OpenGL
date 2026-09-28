#pragma once
#include "Mesh.h"

// One instance of every reusable primitive, built once at startup and shared
// by every object that is drawn part-by-part through drawMesh().
struct MeshLibrary {
    Mesh cube;
    Mesh cylinder;
    Mesh tower;    // tapered cylinder (wind-turbine tower)
    Mesh sphere;
    Mesh cone;
    Mesh quad;

    MeshLibrary()
        : cube(cubeData())
        , cylinder(cylinderData(24))
        , tower(cylinderData(32, 0.55f))
        , sphere(sphereData(16, 24))
        , cone(coneData(24))
        , quad(quadData())
    {}
};
