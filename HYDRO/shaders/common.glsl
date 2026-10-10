// =============================================================================
//  common.glsl  -  shared by lit / gouraud / water / sky / particle shaders.
//  (Shader.cpp pastes this file wherever it finds  #include "common.glsl")
//
//  Contents:
//    1. uniforms describing camera, sun, sky, fog, point lights, shadow map
//    2. noise functions  (procedural textures and water ripples)
//    3. skyColor()       (gradient sky + sun disk + moving clouds)
//    4. shadowFactor()   (shadow mapping with 3x3 soft filtering = PCF)
//    5. caustics()       (dancing light lines seen under water)
//    6. shade()          (Blinn-Phong lighting: ambient + diffuse + specular)
//    7. applyFog(), finalColor()  (distance fog, ACES tone mapping, gamma)
// =============================================================================

uniform mat4  uView;
uniform mat4  uProj;
uniform vec3  uCamPos;
uniform float uTime;

uniform vec3  uSunDir;          // unit vector pointing TO the sun
uniform vec3  uSunColor;        // colour * intensity of the sun
uniform vec3  uSkyTop;
uniform vec3  uSkyHorizon;
uniform vec3  uGroundColor;
uniform float uAmbient;
uniform vec3  uFogColor;
uniform float uFogDensity;
uniform int   uSkyMode;         // 0 outdoor, 1 studio backdrop, 2 underwater
uniform int   uUnderwater;
uniform float uWaterLevel;
uniform float uCaustics;
uniform vec4  uCausticRect;
uniform float uExposure;

uniform int   uNumPoint;
uniform vec3  uPointPos[4];
uniform vec3  uPointColor[4];

uniform sampler2D uShadowMap;
uniform mat4  uLightSpace;
uniform int   uShadowsOn;
uniform float uShadowBias;
uniform float uShadowNormalOffset;

// ---------------------------------------------------------------- noise ----
float hash12(vec2 p) {
    vec3 p3 = fract(vec3(p.xyx) * 0.1031);
    p3 += dot(p3, p3.yzx + 33.33);
    return fract((p3.x + p3.y) * p3.z);
}
float hash13(vec3 p3) {
    p3 = fract(p3 * 0.1031);
    p3 += dot(p3, p3.zyx + 31.32);
    return fract((p3.x + p3.y) * p3.z);
}
float vnoise2(vec2 p) {                         // smooth value noise, 0..1
    vec2 i = floor(p), f = fract(p);
    vec2 u = f * f * (3.0 - 2.0 * f);
    return mix(mix(hash12(i), hash12(i + vec2(1, 0)), u.x),
               mix(hash12(i + vec2(0, 1)), hash12(i + vec2(1, 1)), u.x), u.y);
}
float vnoise3(vec3 p) {
    vec3 i = floor(p), f = fract(p);
    vec3 u = f * f * (3.0 - 2.0 * f);
    float a = hash13(i),               b = hash13(i + vec3(1, 0, 0));
    float c = hash13(i + vec3(0, 1, 0)), d = hash13(i + vec3(1, 1, 0));
    float e = hash13(i + vec3(0, 0, 1)), f1 = hash13(i + vec3(1, 0, 1));
    float g = hash13(i + vec3(0, 1, 1)), h = hash13(i + vec3(1, 1, 1));
    return mix(mix(mix(a, b, u.x), mix(c, d, u.x), u.y),
               mix(mix(e, f1, u.x), mix(g, h, u.x), u.y), u.z);
}
float fbm2(vec2 p) {                           // "fractal" noise = several octaves added
    float s = 0.0, a = 0.5;
    for (int i = 0; i < 4; i++) { s += a * vnoise2(p); p = p * 2.03 + vec2(17.1, 9.2); a *= 0.5; }
    s += a * 0.5;   // average value of the missing octave
    return s;
}
float fbm3(vec3 p) {
    float s = 0.0, a = 0.5;
    for (int i = 0; i < 4; i++) { s += a * vnoise3(p); p = p * 2.01 + vec3(7.3, 1.7, 4.1); a *= 0.5; }
    return s;
}

// ------------------------------------------------------------------ sky ----
vec3 skyColor(vec3 d) {
    float sd = max(dot(d, uSunDir), 0.0);
    if (uSkyMode == 1) {                       // studio backdrop (cut-away diagrams)
        return mix(uSkyHorizon, uSkyTop, clamp(d.y * 0.8 + 0.45, 0.0, 1.0));
    }
    if (uSkyMode == 2) {                       // under water: bright towards the surface
        vec3 c = mix(uFogColor * 0.55, uFogColor * 1.9, smoothstep(-0.4, 1.0, d.y));
        c += uSunColor * 0.12 * pow(sd, 5.0);
        return c;
    }
    float h = d.y;
    vec3 col = mix(uSkyHorizon, uSkyTop, pow(clamp(h, 0.0, 1.0), 0.55));
    if (h < 0.0) col = mix(uSkyHorizon, uGroundColor, clamp(-h * 4.0, 0.0, 1.0));
    col += uSunColor * (0.05 * pow(sd, 6.0) + 0.18 * pow(sd, 64.0));
    col += uSunColor * smoothstep(0.9994, 0.9998, sd) * 3.0;          // sun disk
    if (h > 0.0) {                                                    // clouds
        vec2 cp = d.xz / (h + 0.12) * 0.9 + vec2(uTime * 0.006, uTime * 0.002);
        float c = fbm2(cp * 1.2);
        c = smoothstep(0.42, 0.8, c) * smoothstep(0.0, 0.2, h);
        vec3 cloud = mix(uSkyHorizon * 0.85, vec3(length(uSunColor) * 0.33), 0.35 + 0.65 * sd);
        col = mix(col, cloud, c * 0.8);
    }
    return col;
}

// --------------------------------------------------------------- shadows ---
float shadowFactor(vec3 wp, vec3 n) {
    if (uShadowsOn == 0) return 1.0;
    vec4 ls = uLightSpace * vec4(wp + n * uShadowNormalOffset, 1.0);
    vec3 p = ls.xyz / ls.w * 0.5 + 0.5;
    if (p.z > 1.0 || p.x < 0.0 || p.x > 1.0 || p.y < 0.0 || p.y > 1.0) return 1.0;
    vec2 texel = 1.0 / vec2(textureSize(uShadowMap, 0));
    float lit = 0.0;
    for (int x = -1; x <= 1; x++)
        for (int y = -1; y <= 1; y++) {
            float d = texture(uShadowMap, p.xy + vec2(x, y) * texel * 1.25).r;
            lit += (p.z - uShadowBias > d) ? 0.0 : 1.0;
        }
    return lit / 9.0;
}

// -------------------------------------------------------------- caustics ---
// Classic "tileable water caustic" pattern: bright wobbling lines.
float causticPattern(vec2 uv) {
    float t = uTime * 0.45 + 23.0;
    vec2 p = mod(uv * 6.28318, 6.28318) - 250.0;
    vec2 i = p;
    float c = 1.0;
    float inten = 0.005;
    for (int n = 0; n < 5; n++) {
        float tt = t * (1.0 - (3.5 / float(n + 1)));
        i = p + vec2(cos(tt - i.x) + sin(tt + i.y), sin(tt - i.y) + cos(tt + i.x));
        c += 1.0 / length(vec2(p.x / (sin(i.x + tt) / inten), p.y / (cos(i.y + tt) / inten)));
    }
    c /= 5.0;
    c = 1.17 - pow(c, 1.4);
    return pow(abs(c), 8.0);
}
float caustics(vec3 wp, vec3 n) {
    if (uCaustics <= 0.0 || wp.y > uWaterLevel) return 0.0;
    if (wp.x < uCausticRect.x || wp.z < uCausticRect.y || wp.x > uCausticRect.z || wp.z > uCausticRect.w) return 0.0;
    vec2 uv = abs(n.y) > 0.5 ? wp.xz : vec2(wp.x + wp.z, wp.y * 0.8);
    float c = causticPattern(uv * 0.11);
    return c * uCaustics;
}

// -------------------------------------------------------------- lighting ---
// Blinn-Phong:  colour = ambient + diffuse * (N.L) + specular * (N.H)^shininess
vec3 shade(vec3 albedo, vec3 N, vec3 wp, float specStrength, float shininess, float reflectAmt, float shadow) {
    vec3 V = normalize(uCamPos - wp);
    // ambient light from the sky above and the ground below
    float hemi = N.y * 0.5 + 0.5;
    vec3 amb = mix(uGroundColor, uSkyTop * 0.55 + uSkyHorizon * 0.45, hemi) * uAmbient;
    vec3 col = albedo * amb;

    // the sun (directional light)
    vec3 L = uSunDir;
    float ndl = max(dot(N, L), 0.0);
    vec3 H = normalize(L + V);
    float spec = pow(max(dot(N, H), 0.0), shininess) * specStrength * (shininess + 8.0) / 32.0;
    col += (albedo * ndl + spec * smoothstep(0.0, 0.15, ndl)) * uSunColor * shadow;

    // point lights (lamps inside buildings, underwater lights ...)
    for (int i = 0; i < uNumPoint; i++) {
        vec3 d = uPointPos[i] - wp;
        float dist = length(d);
        vec3 Lp = d / dist;
        float att = 1.0 / (1.0 + 0.06 * dist * dist);
        float nd = max(dot(N, Lp), 0.0);
        float sp = pow(max(dot(N, normalize(Lp + V)), 0.0), shininess) * specStrength;
        col += (albedo * nd + sp) * uPointColor[i] * att;
    }

    // mirror reflection of the environment (metal, wet surfaces)
    if (reflectAmt > 0.0) {
        float fres = reflectAmt + (1.0 - reflectAmt) * pow(1.0 - max(dot(N, V), 0.0), 5.0);
        col += skyColor(reflect(-V, N)) * fres * reflectAmt * (0.4 + 0.6 * shadow);
    }

    // under-water caustics
    col += albedo * uSunColor * caustics(wp, N) * shadow * (0.35 + 0.65 * max(N.y, 0.0));
    return col;
}

// ------------------------------------------------------------- fog / tone --
vec3 applyFog(vec3 col, vec3 wp) {
    float d = length(wp - uCamPos);
    if (uUnderwater == 1) {
        // water absorbs red first, then green -> distant things turn blue-green
        vec3 absorb = exp(-d * uFogDensity * vec3(2.2, 0.9, 0.65));
        float f = 1.0 - exp(-d * uFogDensity * 1.2);
        float depthDark = clamp((uWaterLevel - wp.y) * 0.012, 0.0, 0.5);
        return mix(col * absorb, uFogColor * (1.0 - depthDark), f);
    }
    float f = 1.0 - exp(-d * uFogDensity);
    vec3 v = normalize(wp - uCamPos);
    vec3 fc = uFogColor + uSunColor * 0.06 * pow(max(dot(v, uSunDir), 0.0), 8.0);
    return mix(col, fc, f);
}

vec3 aces(vec3 x) {          // filmic tone curve (cinema look)
    const float a = 2.51, b = 0.03, c = 2.43, d = 0.59, e = 0.14;
    return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}
vec3 finalColor(vec3 c) {
    c = aces(c * uExposure);
    return pow(c, vec3(1.0 / 2.2));    // gamma correction for the monitor
}
