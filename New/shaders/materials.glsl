// =============================================================================
//  materials.glsl  -  procedural textures (no image files are used).
//
//  surfaceAlbedo() returns the colour of a surface point.  The pattern is
//  chosen by uPattern; most patterns are computed from the WORLD position so
//  big objects (the dam) get big panels and small ones keep fine detail.
// =============================================================================

uniform vec3  uColor;
uniform int   uPattern;
uniform float uPatternScale;
uniform float uSpecular;
uniform float uShininess;
uniform float uReflect;
uniform vec3  uEmissive;
uniform float uAlpha;
uniform int   uTwoSided;

#define PAT_NONE     0
#define PAT_CONCRETE 1
#define PAT_STEEL    2
#define PAT_PAINT    3
#define PAT_BARK     4
#define PAT_TERRAIN  5
#define PAT_SOIL     6
#define PAT_TILES    7
#define PAT_GRATING  8
#define PAT_COPPER   9
#define PAT_HOLO     10
#define PAT_LEAF     11
#define PAT_BEAM     12
#define PAT_ROCK     13
#define PAT_PLASTIC  14

// Choose the 2 world axes that lie in the surface (cheap "tri-planar" mapping).
vec2 planarUV(vec3 wp, vec3 n) {
    vec3 a = abs(n);
    if (a.y > a.x && a.y > a.z) return wp.xz;
    if (a.x > a.z) return vec2(wp.z, wp.y);
    return vec2(wp.x, wp.y);
}

vec3 surfaceAlbedo(vec3 wp, vec3 n, vec2 uv, inout float spec, inout float shin,
                   inout float refl, inout float alpha, inout vec3 emis) {
    vec3 c = uColor;
    float sc = uPatternScale;

    if (uPattern == PAT_CONCRETE) {
        // poured concrete: formwork panels, joints, tie-rod holes, stains, rain streaks
        vec2 p = planarUV(wp, n) * sc;
        vec2 cell = p / vec2(6.0, 3.0);
        vec2 id = floor(cell), f = fract(cell);
        vec2 dEdge = min(f, 1.0 - f) * vec2(6.0, 3.0);
        float w = fwidth(p.x) + fwidth(p.y);
        float far = smoothstep(0.08, 0.35, w);                 // fade small details far away
        float joint = (1.0 - smoothstep(0.02, 0.02 + w * 1.5, min(dEdge.x, dEdge.y))) * (1.0 - far);
        vec2 hp = fract(cell * vec2(2.0, 1.0)) - 0.5;
        float hole = (1.0 - smoothstep(0.05, 0.08, length(hp * 3.0))) * (1.0 - far);
        float panel = 0.94 + 0.1 * hash12(id);                          // each pour is a bit different
        float weather = 0.7 + 0.45 * fbm2(wp.xz * 0.02 + wp.y * 0.015);  // big soft patches
        float stain = 0.8 + 0.3 * fbm2(p * 0.12 + id * 0.37);
        float streak = smoothstep(0.5, 0.95, vnoise2(vec2(p.x * 1.6, p.y * 0.06))) * 0.28;   // rain streaks
        float grain = 0.93 + 0.1 * vnoise2(p * 13.0);
        c *= panel * weather * stain * grain * (1.0 - streak) * (1.0 - joint * 0.16) * (1.0 - hole * 0.4);
        c *= vec3(1.0, 1.0, 1.02) - vec3(0.03, 0.0, 0.0) * vnoise2(p * 0.05);
        // big structures (dam): damp, darker concrete near the water line
        if (sc < 0.8) c *= 0.7 + 0.3 * smoothstep(1.0, 16.0, wp.y);
        spec *= 0.5 + 0.5 * stain;
    }
    else if (uPattern == PAT_STEEL) {
        // brushed steel: fine lines along the part + soft blotches
        float brushed = vnoise2(vec2(uv.x * 30.0, uv.y * 1.2) * sc);
        float blotch = fbm2(wp.xz * 0.35 + wp.y * 0.25);
        c *= (0.86 + 0.14 * brushed) * (0.82 + 0.3 * blotch);
        shin *= 0.7 + 0.3 * blotch;
    }
    else if (uPattern == PAT_PAINT) {
        vec2 p = planarUV(wp, n) * sc;
        c *= 0.9 + 0.14 * fbm2(p * 1.5);
        float wear = smoothstep(0.72, 0.8, fbm2(p * 3.0 + 4.0));
        c = mix(c, vec3(0.32, 0.3, 0.28), wear * 0.6);
    }
    else if (uPattern == PAT_BARK) {
        // weathered drift-wood: long grooves along the branch (uv.y = along)
        float g = vnoise2(vec2(uv.x * 7.0, uv.y * 0.9) * sc);
        float fine = vnoise2(vec2(uv.x * 30.0, uv.y * 4.0) * sc);
        float groove = smoothstep(0.3, 0.7, g);
        c = mix(c * 0.42, c * 1.18, groove) * (0.82 + 0.32 * fine);
        spec *= 0.35;
    }
    else if (uPattern == PAT_TERRAIN) {
        // rock on steep slopes, dirt + grass on flat ground
        float nn = fbm2(wp.xz * 0.04);
        float strata = 0.9 + 0.1 * sin(wp.y * 0.9 + fbm2(wp.xz * 0.08) * 9.0);
        vec3 rock = c * (0.45 + 0.9 * fbm2(wp.xz * 0.11 + wp.y * 0.2)) * (0.7 + 0.5 * vnoise2(wp.xz * 0.9 + wp.y)) * strata;
        vec3 grass = vec3(0.13, 0.17, 0.08) * (0.7 + 0.6 * nn);
        vec3 dirt = vec3(0.24, 0.21, 0.16) * (0.8 + 0.4 * nn);
        float flat_ = smoothstep(0.62, 0.86, n.y + (nn - 0.5) * 0.35);
        c = mix(rock, mix(dirt, grass, smoothstep(0.35, 0.6, nn)), flat_);
        spec *= 0.4;
    }
    else if (uPattern == PAT_SOIL) {
        vec2 p = planarUV(wp, n) * sc;
        float pebble = smoothstep(0.66, 0.72, vnoise2(p * 4.0));
        c *= 0.6 + 0.55 * fbm2(p * 1.6);
        c = mix(c, vec3(0.42, 0.38, 0.33) * (0.7 + 0.5 * vnoise2(p * 9.0)), pebble * 0.8);
        spec *= 0.2;
    }
    else if (uPattern == PAT_TILES) {
        vec2 p = planarUV(wp, n) * sc;
        vec2 f = fract(p), id = floor(p);
        float grout = 1.0 - smoothstep(0.012, 0.03, min(min(f.x, 1.0 - f.x), min(f.y, 1.0 - f.y)));
        c *= (0.86 + 0.2 * hash12(id)) * (1.0 - grout * 0.45) * (0.88 + 0.18 * fbm2(p * 0.4));
    }
    else if (uPattern == PAT_GRATING) {
        // diamond wire mesh: holes are cut out with discard
        vec2 p = uv * sc;
        vec2 q = fract(vec2(p.x + p.y, p.x - p.y));
        float wire = min(min(q.x, 1.0 - q.x), min(q.y, 1.0 - q.y));
        if (wire > 0.11) discard;
    }
    else if (uPattern == PAT_COPPER) {
        // generator coils: copper bands
        float band = fract(uv.y * sc);
        c = mix(c * 0.55, c * 1.15, smoothstep(0.08, 0.25, band) * smoothstep(0.92, 0.75, band));
        refl = max(refl, 0.25);
    }
    else if (uPattern == PAT_HOLO) {
        // "x-ray" hologram grid used when the pipe becomes see-through
        vec2 p = uv * sc;
        vec2 f = abs(fract(p) - 0.5);
        float lw = 0.035 + fwidth(p.x) + fwidth(p.y);
        float line = smoothstep(0.5 - lw, 0.5, max(f.x, f.y));
        emis = uEmissive * (0.18 + 1.6 * line);
        alpha = uAlpha * (0.25 + 0.75 * line);
    }
    else if (uPattern == PAT_LEAF) {
        float vein = 1.0 - smoothstep(0.0, 0.04, abs(uv.x - 0.5));
        c *= 0.8 + 0.3 * vnoise2(uv * 9.0 * sc);
        c = mix(c, c * 1.35 + 0.05, vein * 0.6);
    }
    else if (uPattern == PAT_BEAM) {
        // light shaft under water (uv.y: 0 at the surface .. 1 at the end)
        float fade = smoothstep(0.0, 0.12, uv.y) * (1.0 - smoothstep(0.35, 1.0, uv.y));
        float flicker = 0.55 + 0.45 * vnoise2(vec2(uv.x * 3.0 + uTime * 0.35, uTime * 0.25));
        float edge = smoothstep(0.0, 0.35, uv.x) * smoothstep(1.0, 0.65, uv.x);
        emis = uEmissive;
        alpha = uAlpha * fade * flicker * edge;   // additive: the glow strength is the alpha
    }
    else if (uPattern == PAT_ROCK) {
        float r = fbm3(wp * sc);
        c *= 0.5 + 0.8 * r;
        c = mix(c, c * vec3(0.8, 0.95, 0.85), smoothstep(0.5, 0.7, vnoise3(wp * sc * 3.0)) * 0.4);
        spec *= 0.35;
    }
    else if (uPattern == PAT_PLASTIC) {
        c *= 0.85 + 0.2 * vnoise2(uv * 12.0 * sc);
    }
    return c;
}
