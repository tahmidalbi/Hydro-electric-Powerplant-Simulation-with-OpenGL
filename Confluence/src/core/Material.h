#pragma once
#include <glm/glm.hpp>

// ka/kd/ks: ambient/diffuse/specular reflectivity. shininess: Phong exponent.
// The shader multiplies ka and kd by the per-vertex colour, so a material
// either carries its own colour (primitives with white vertices) or acts as
// a finish -- matte, glossy, water -- over baked vertex colours.
struct Material {
    glm::vec3 ka, kd, ks, emission;
    float shininess;
    float alpha = 1.0f;      // < 1: blended, drawn in the transparent pass
    float softEdge = 0.0f;   // > 0: fades out towards the silhouette (spray, mist)
};

namespace Materials {
    // Own-colour materials
    inline const Material WHITE_PAINT = {{0.93f,0.94f,0.96f},{0.93f,0.94f,0.96f},{0.35f,0.35f,0.35f},{0,0,0}, 32.0f};
    inline const Material STEEL       = {{0.30f,0.36f,0.42f},{0.30f,0.36f,0.42f},{0.75f,0.78f,0.80f},{0,0,0}, 70.0f};
    inline const Material LAKE_WATER  = {{0.14f,0.46f,0.78f},{0.12f,0.42f,0.74f},{0.55f,0.60f,0.65f},{0,0,0}, 90.0f};

    // Finishes over baked vertex colours
    inline const Material TERRAIN = {{1,1,1},{1,1,1},{0.03f,0.03f,0.03f},{0,0,0}, 6.0f};
    inline const Material MATTE   = {{1,1,1},{1,1,1},{0.10f,0.10f,0.10f},{0,0,0}, 12.0f};
    inline const Material FOLIAGE = {{1,1,1},{1,1,1},{0.05f,0.05f,0.05f},{0,0,0}, 8.0f};
    inline const Material GLOSSY  = {{1,1,1},{1,1,1},{0.65f,0.65f,0.65f},{0,0,0}, 60.0f};
    inline const Material WATER   = {{1,1,1},{1,1,1},{0.60f,0.64f,0.68f},{0,0,0}, 70.0f};
    inline const Material CLOUD   = {{0.95f,0.95f,0.95f},{0.45f,0.45f,0.45f},{0,0,0},{0.42f,0.42f,0.42f}, 1.0f};

    // Translucent water: glassy falling sheets, and the spray cloud at their foot.
    inline const Material WATERFALL = {{1,1,1},{0.72f,0.72f,0.72f},{0.70f,0.70f,0.70f},{0.18f,0.18f,0.18f}, 45.0f, 0.90f, 0.0f};
    inline const Material MIST      = {{1,1,1},{0.40f,0.40f,0.40f},{0,0,0},{0.55f,0.55f,0.55f}, 1.0f, 0.32f, 1.15f};
}
