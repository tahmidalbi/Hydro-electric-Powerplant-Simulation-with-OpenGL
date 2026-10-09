// =====================================================================
// surface_common.glsl  --  shared by every scene shader stage
// =====================================================================
// Pasted by Shader.cpp right AFTER lighting_common.glsl (it uses uViewPos
// from there). Contents:
//   1. waves          (Step 7)  the lake surface: height + normal
//   2. flowing water  (Step 6)  streaks that run down the falls / along the river
//   3. reflection     (Step 7)  water reflects the sky more at grazing angles
//   4. finishColor             highlight (Step 8), fog, transparency, debug view

uniform float uTime;   // SIMULATION time in seconds (stops when paused)

// ---------------------------------------------------------------------
// 1. WAVES -- the same table the boat uses (src/systems/Waves.cpp).
//    height = meanLevel + sum  A * sin(k * dot(d, p) - w * t + phase)
// ---------------------------------------------------------------------
#define WAVE_COUNT 3
uniform int   uWaveMode;                       // 1: this draw is the lake surface
uniform float uWaveMeanLevel;
uniform vec3  uWaveDirK[WAVE_COUNT];           // (direction.x, direction.z, k)
uniform vec3  uWaveAmpOmegaPhase[WAVE_COUNT];  // (amplitude, omega, phase)

// Far from the camera the lake grid is coarse (12 m cells) and a 6 m wave
// would only produce random-looking spikes ("aliasing"). So the waves fade
// out between 250 and 420 m from the camera. The boat's route is always
// closer than that in the preset views, so it still floats on what you see.
float waveFade(vec2 p) {
    return 1.0 - smoothstep(250.0, 420.0, distance(p, uViewPos.xz));
}

float waveHeight(vec2 p) {
    float h = 0.0;
    for (int i = 0; i < WAVE_COUNT; ++i) {
        vec3 dk = uWaveDirK[i];
        vec3 a = uWaveAmpOmegaPhase[i];
        h += a.x * sin(dk.z * dot(dk.xy, p) - a.y * uTime + a.z);
    }
    return uWaveMeanLevel + waveFade(p) * h;
}

// Normal from the slope:  n = normalize(-dh/dx, 1, -dh/dz)
vec3 waveNormal(vec2 p) {
    vec2 slope = vec2(0.0);
    for (int i = 0; i < WAVE_COUNT; ++i) {
        vec3 dk = uWaveDirK[i];
        vec3 a = uWaveAmpOmegaPhase[i];
        slope += a.x * dk.z * dk.xy * cos(dk.z * dot(dk.xy, p) - a.y * uTime + a.z);
    }
    slope *= waveFade(p);
    return normalize(vec3(-slope.x, 1.0, -slope.y));
}

// Phong mode only: two tiny extra ripples (5 cm high, 1-2 m long). They are
// far too small to move a vertex or the boat; they only bend the light, so
// the sun breaks into sparkles on the water. Gouraud/Flat use waveNormal().
// They fade out between 40 and 160 m, where they would be smaller than a pixel.
vec3 waveNormalDetailed(vec2 p) {
    vec3 n = waveNormal(p);
    float t = uTime;
    vec2 d1 = normalize(vec2(-0.9, 0.45)), d2 = normalize(vec2(-0.2, -1.0));
    float k1 = 6.2832 / 1.7, k2 = 6.2832 / 1.1;
    vec2 slope = 0.05 * k1 * d1 * cos(k1 * dot(d1, p) - 1.6 * k1 * t)
               + 0.03 * k2 * d2 * cos(k2 * dot(d2, p) - 1.3 * k2 * t + 2.0);
    slope *= 1.0 - smoothstep(40.0, 160.0, distance(p, uViewPos.xz));
    return normalize(n + vec3(-slope.x, 0.0, -slope.y));
}

// ---------------------------------------------------------------------
// 2. FLOWING WATER -- a moving streak pattern (no texture image needed).
// ---------------------------------------------------------------------
uniform int   uFlowMode;    // 0 none, 1 falling sheet, 2 river surface
uniform vec2  uFlowDir;     // horizontal direction the river runs (unit, x-z)
uniform float uFlowSpeed;   // how fast the pattern moves
uniform float uFoam;        // river only: 0 = calm (gate shut) .. 1 = full white water

float hash12(vec2 p) {
    vec3 q = fract(vec3(p.xyx) * 0.1031);
    q += dot(q, q.yzx + 33.33);
    return fract((q.x + q.y) * q.z);
}

// Smooth value noise in [0, 1]: random values on a grid, blended smoothly.
float valueNoise(vec2 p) {
    vec2 i = floor(p), f = fract(p);
    vec2 u = f * f * (3.0 - 2.0 * f);
    return mix(mix(hash12(i), hash12(i + vec2(1, 0)), u.x),
               mix(hash12(i + vec2(0, 1)), hash12(i + vec2(1, 1)), u.x), u.y);
}

vec3 animateFlow(vec3 color, vec3 pos) {
    if (uFlowMode == 0) return color;
    vec2 across = vec2(-uFlowDir.y, uFlowDir.x);
    if (uFlowMode == 1) {
        // Falling sheet: streaks are long vertically and slide DOWN over time.
        float u = dot(pos.xz, across) * 1.6;
        float v = pos.y * 0.35 + uTime * uFlowSpeed;
        float n = 0.65 * valueNoise(vec2(u, v)) + 0.35 * valueNoise(vec2(u * 2.7, v * 2.3));
        return color * mix(0.80, 1.18, n);
    }
    // River foam follows the gate. The mesh's baked colour is white where the
    // foam is and blue elsewhere, so "how white" (red channel: ~0.2 for the
    // water, ~0.93 for foam) tells us how much foam there is here; we fade
    // that part towards plain river blue when uFoam < 1.
    float foamHere = clamp((color.r - 0.22) / 0.71, 0.0, 1.0);
    color = mix(color, vec3(0.05, 0.30, 0.41), foamHere * (1.0 - uFoam));
    // River: streaks stretched along the current, drifting downstream.
    float along = dot(pos.xz, uFlowDir), side = dot(pos.xz, across);
    float n = valueNoise(vec2(side * 0.35, along * 0.10 - uTime * uFlowSpeed * 0.10));
    return color * mix(0.90, 1.10, n);
}

// ---------------------------------------------------------------------
// 3. REFLECTION -- water mirrors the sky, strongly when seen edge-on.
//    Fresnel (Schlick): F = F0 + (1 - F0) * (1 - cos(theta))^5,  F0 = 0.02
// ---------------------------------------------------------------------
uniform float uReflectivity;          // material: 0 = no mirror, water ~ 0.9
uniform vec3  uSkyHorizon, uSkyZenith;
uniform vec3  uSunDirection;          // the way sunlight travels

// The same sky the sky shader paints (sky.frag calls this too).
vec3 skyColour(vec3 dir) {
    float up = max(dir.y, 0.0);
    vec3 sky = mix(uSkyHorizon, uSkyZenith, pow(smoothstep(0.0, 0.55, up), 0.8));
    sky = mix(sky, uSkyHorizon * 0.85, smoothstep(0.0, -0.25, dir.y));   // below the horizon: haze
    float toSun = max(dot(dir, -uSunDirection), 0.0);
    sky += vec3(1.0, 0.85, 0.6) * (0.22 * pow(toSun, 10.0) + 0.6 * pow(toSun, 300.0));   // glow + disc
    return sky;
}

vec3 applyReflection(vec3 color, vec3 N, vec3 pos) {
    if (uReflectivity <= 0.0) return color;
    vec3 V = normalize(uViewPos - pos);
    float cosTheta = clamp(dot(N, V), 0.0, 1.0);
    float fresnel = 0.02 + 0.98 * pow(1.0 - cosTheta, 5.0);
    vec3 R = reflect(-V, N);
    return mix(color, skyColour(R), clamp(fresnel * uReflectivity, 0.0, 1.0));
}

// ---------------------------------------------------------------------
// 4. FINISH -- in this order: debug view, flow streaks, reflection,
//    selection highlight, distance fog, transparency.
// ---------------------------------------------------------------------
uniform vec3  uFogColor;     // aerial haze: far geometry fades to this colour
uniform float uFogDensity;   // 0 = no fog (clouds, gallery)
uniform float uAlpha;        // < 1 for water sheets
uniform float uSoftEdge;     // > 0 fades a surface out where it turns edge-on
uniform int   uDebugView;    // N key: 0 lit colour, 1 normal as colour
uniform vec3  uHighlight;    // Step 8: glow added to the selected object (0 otherwise)

vec4 finishColor(vec3 litColor, vec3 fragPos, vec3 N) {
    if (uDebugView == 1)                       // normal (-1..1) mapped to colour (0..1)
        return vec4(N * 0.5 + 0.5, 1.0);

    vec3 color = animateFlow(litColor, fragPos);
    color = applyReflection(color, N, fragPos);
    color += uHighlight;

    float d = length(fragPos - uViewPos) * uFogDensity;
    color = mix(color, uFogColor, 1.0 - exp(-d * d));

    float alpha = uAlpha;
    if (uSoftEdge > 0.0) {
        vec3 V = normalize(uViewPos - fragPos);
        alpha *= pow(abs(dot(N, V)), uSoftEdge);
    }
    return vec4(color, alpha);
}
