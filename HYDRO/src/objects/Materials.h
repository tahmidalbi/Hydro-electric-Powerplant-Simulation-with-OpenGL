// =============================================================================
//  Materials.h  -  ready-made materials shared by many objects, so all the
//  concrete in the project looks like the same concrete, etc.
// =============================================================================
#pragma once
#include "../render/Renderer.h"

namespace Mat {

inline Material concrete(float scale = 1.0f, Vec3 color = Vec3(0.5f, 0.5f, 0.49f)) {
    Material m;
    m.color = color;
    m.pattern = PAT_CONCRETE;
    m.patternScale = scale;
    m.specular = 0.12f;
    m.shininess = 18.0f;
    return m;
}
inline Material darkConcrete(float scale = 1.0f) { return concrete(scale, Vec3(0.3f, 0.3f, 0.3f)); }

inline Material steel(Vec3 color = Vec3(0.55f, 0.57f, 0.6f)) {
    Material m;
    m.color = color;
    m.pattern = PAT_STEEL;
    m.specular = 0.9f;
    m.shininess = 70.0f;
    m.reflect = 0.35f;
    return m;
}
inline Material darkSteel() { return steel(Vec3(0.25f, 0.27f, 0.3f)); }

inline Material paint(Vec3 color, float spec = 0.35f) {
    Material m;
    m.color = color;
    m.pattern = PAT_PAINT;
    m.specular = spec;
    m.shininess = 40.0f;
    m.reflect = 0.08f;
    return m;
}
inline Material plain(Vec3 color, float spec = 0.2f, float shin = 24.0f) {
    Material m;
    m.color = color;
    m.specular = spec;
    m.shininess = shin;
    return m;
}
inline Material glow(Vec3 color) {
    Material m;
    m.color = Vec3(0, 0, 0);
    m.emissive = color;
    m.specular = 0.0f;
    m.castShadow = false;
    return m;
}
inline Material copper() {
    Material m;
    m.color = Vec3(0.62f, 0.27f, 0.17f);
    m.pattern = PAT_COPPER;
    m.patternScale = 1.4f;
    m.specular = 0.8f;
    m.shininess = 50.0f;
    m.reflect = 0.2f;
    return m;
}

}  // namespace Mat
