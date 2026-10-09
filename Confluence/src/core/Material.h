#pragma once
#include <glm/glm.hpp>

// ka/kd/ks: ambient/diffuse/specular reflectivity. shininess: Phong exponent.
// The shader multiplies ka and kd by the per-vertex colour, so a material
// either carries its own colour (primitives with white vertices) or acts as
// a finish -- matte, glossy, water -- over baked vertex colours.
//
// STEP 3 - believable materials. Read ks and shininess together:
//   small ks + small shininess  -> weak, broad highlight   (concrete, terrain)
//   medium ks + medium          -> moderate highlight       (painted turbine)
//   large ks + large shininess  -> strong, tight highlight  (steel, panel glass, water)
// emission is light the surface gives off by itself: it is visible even with
// every light switched off, but it does NOT light up anything around it.
struct Material {
    glm::vec3 ka, kd, ks, emission;
    float shininess;
    float alpha = 1.0f;         // < 1: blended, drawn in the transparent pass
    float softEdge = 0.0f;      // > 0: fades out towards the silhouette (spray, mist)
    float reflectivity = 0.0f;  // Step 7: > 0 mirrors the sky (Fresnel), used for water
};

namespace Materials {
    // Own-colour materials
    inline const Material WHITE_PAINT = {{0.93f,0.94f,0.96f},{0.93f,0.94f,0.96f},{0.35f,0.35f,0.35f},{0,0,0}, 32.0f};
    inline const Material STEEL       = {{0.30f,0.36f,0.42f},{0.30f,0.36f,0.42f},{0.75f,0.78f,0.80f},{0,0,0}, 70.0f};
    // Lake: a deep blue-green body colour; the sky reflection (Fresnel) adds
    // the brightness, strongest far away where the surface is seen edge-on.
    inline const Material LAKE_WATER  = {{0.10f,0.36f,0.56f},{0.09f,0.34f,0.54f},{0.75f,0.75f,0.72f},{0,0,0}, 140.0f, 1.0f, 0.0f, 0.85f};

    // Finishes over baked vertex colours
    inline const Material TERRAIN  = {{1,1,1},{1,1,1},{0.03f,0.03f,0.03f},{0,0,0}, 6.0f};
    inline const Material CONCRETE = {{1,1,1},{1,1,1},{0.08f,0.08f,0.08f},{0,0,0}, 8.0f};     // dam: weak, broad
    inline const Material MATTE    = {{1,1,1},{1,1,1},{0.10f,0.10f,0.10f},{0,0,0}, 12.0f};
    inline const Material FOLIAGE  = {{1,1,1},{1,1,1},{0.05f,0.05f,0.05f},{0,0,0}, 8.0f};
    inline const Material GLOSSY   = {{1,1,1},{1,1,1},{0.65f,0.65f,0.65f},{0,0,0}, 60.0f};
    inline const Material PANEL_GLASS = {{1,1,1},{1,1,1},{0.85f,0.85f,0.85f},{0,0,0}, 120.0f};  // solar: strong, tight
    inline const Material WATER    = {{1,1,1},{1,1,1},{0.60f,0.64f,0.68f},{0,0,0}, 70.0f, 1.0f, 0.0f, 0.45f};
    inline const Material CLOUD    = {{0.95f,0.95f,0.95f},{0.45f,0.45f,0.45f},{0,0,0},{0.42f,0.42f,0.42f}, 1.0f};

    // Translucent water: glassy falling sheets, and the spray cloud at their foot.
    inline const Material WATERFALL = {{1,1,1},{0.72f,0.72f,0.72f},{0.70f,0.70f,0.70f},{0.18f,0.18f,0.18f}, 45.0f, 0.90f, 0.0f};
    inline const Material MIST      = {{1,1,1},{0.40f,0.40f,0.40f},{0,0,0},{0.55f,0.55f,0.55f}, 1.0f, 0.32f, 1.15f};

    // Emissive bulb that marks where the inspection lamp is. Pure emission:
    // it looks bright on its own; the point light (a separate Light) is what
    // actually illuminates the surfaces around it.
    inline const Material LAMP_GLOW = {{0,0,0},{0,0,0},{0,0,0},{1.0f,0.86f,0.55f}, 1.0f};

    // Gallery (Step 4): shiny, single-colour objects so the highlight is easy to judge.
    inline const Material POLISHED_RED  = {{0.72f,0.10f,0.09f},{0.72f,0.10f,0.09f},{0.90f,0.90f,0.90f},{0,0,0}, 80.0f};
    inline const Material POLISHED_BLUE = {{0.12f,0.30f,0.72f},{0.12f,0.30f,0.72f},{0.90f,0.90f,0.90f},{0,0,0}, 80.0f};
    inline const Material GALLERY_WATER = {{0.10f,0.34f,0.58f},{0.10f,0.34f,0.58f},{1.00f,1.00f,1.00f},{0,0,0}, 150.0f};
    inline const Material GALLERY_FLOOR = {{0.40f,0.43f,0.42f},{0.40f,0.43f,0.42f},{0.05f,0.05f,0.05f},{0,0,0}, 6.0f};
    inline const Material GALLERY_GOLD  = {{0.80f,0.60f,0.18f},{0.80f,0.60f,0.18f},{0.60f,0.55f,0.40f},{0,0,0}, 40.0f};
    inline const Material GALLERY_GREEN = {{0.20f,0.62f,0.32f},{0.20f,0.62f,0.32f},{0.35f,0.35f,0.35f},{0,0,0}, 30.0f};
}
