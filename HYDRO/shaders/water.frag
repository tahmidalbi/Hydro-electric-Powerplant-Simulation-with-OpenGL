// =============================================================================
//  water.frag - animated water.
//
//  mode 0 : lake / river / channel surface (ripples move with the wind/current)
//  mode 1 : flowing sheet (spillway chute, jets) - streaks race along uv.y
//  mode 2 : water inside a pipe (tube) - same streaks + rounded front
//  mode 3 : the cut face of a water volume in cut-away diagrams
//
//  Look = mix(body colour, reflected sky, Fresnel) + sun glitter + foam.
//  Seen from BELOW (under water) the surface shows "Snell's window": the sky
//  is visible straight up, at flatter angles total internal reflection.
// =============================================================================
#version 330 core
#include "common.glsl"

in vec3 vWorldPos;
in vec3 vNormal;
in vec2 vUV;
in float vNose;

uniform int   uWaterMode;
uniform vec3  uDeep;
uniform vec3  uShallow;
uniform float uAlpha;
uniform float uFoam;
uniform float uFlowSpeed;
uniform float uRipple;
uniform float uRippleStrength;
uniform float uWaveAmp;
uniform float uWaveFreq;
uniform float uSurfaceY;
uniform float uFront;
uniform float uTail;
uniform float uSheetWidth;
uniform float uGlow;
uniform float uNoseLen;
uniform vec4  uFoamSpot;
uniform vec4  uFoamSpot2;

out vec4 FragColor;

float waveHeight(vec2 p) {          // must match water.vert
    float f = uWaveFreq;
    float t = uTime;
    vec2 q = p - vec2(uFlowSpeed * t, 0.0);
    return uWaveAmp * (0.50 * sin(q.x * f + t * 1.3)
                     + 0.30 * sin(q.y * f * 1.4 - t * 1.1 + q.x * f * 0.5)
                     + 0.20 * sin((q.x + q.y) * f * 2.3 + t * 2.1));
}
// small ripples on calm water (world xz)
float lakeHeight(vec2 p) {
    vec2 q = p * uRipple;
    return fbm2(q + vec2(uTime * (0.12 + uFlowSpeed * uRipple), uTime * 0.07))
         + 0.5 * vnoise2(q * 1.9 - vec2(uTime * 0.1, -uTime * 0.13));
}
// ripples on fast water: stretched along the flow and moving with it
float flowHeight(vec2 uv) {
    vec2 q = vec2(uv.x * 1.3, uv.y * 0.35 - uTime * uFlowSpeed * 0.35) * uRipple;
    return fbm2(q) + 0.4 * vnoise2(q * vec2(2.5, 1.5) + vec2(0.0, -uTime * 1.5));
}

void main() {
    vec2 uv = vUV;
    if (uWaterMode == 1 || uWaterMode == 2) {
        if (uv.y < uTail) discard;                         // water has already drained here
        if (uWaterMode == 1 && uv.y > uFront) discard;     // water has not arrived yet
    }

    vec3 Ngeo = normalize(vNormal);
    if (vNose > 0.0) Ngeo = normalize(cross(dFdx(vWorldPos), dFdy(vWorldPos)));
    vec3 V = normalize(uCamPos - vWorldPos);
    bool fromBelow = dot(Ngeo, V) < 0.0;
    if (fromBelow) Ngeo = -Ngeo;
    float dist = length(uCamPos - vWorldPos);
    float detail = uRippleStrength / (1.0 + dist * 0.012);

    // ---------------- normal with ripples (bump mapping) -------------------
    vec3 N = Ngeo;
    if (uWaterMode == 0) {
        float e = 0.08;
        vec2 p = vWorldPos.xz;
        float h0 = lakeHeight(p);
        vec3 g = vec3(lakeHeight(p + vec2(e, 0.0)) - h0, 0.0, lakeHeight(p + vec2(0.0, e)) - h0) / e * 0.35 * detail;
        if (uWaveAmp > 0.0) {
            float w0 = waveHeight(p);
            g.x += (waveHeight(p + vec2(e, 0.0)) - w0) / e;
            g.z += (waveHeight(p + vec2(0.0, e)) - w0) / e;
        }
        N = normalize(Ngeo - g * (fromBelow ? -1.0 : 1.0));
    } else if (uWaterMode == 1 || uWaterMode == 2) {
        // tangent frame from the uv derivatives (works on curved chutes and tubes)
        vec3 dp1 = dFdx(vWorldPos), dp2 = dFdy(vWorldPos);
        vec2 du1 = dFdx(uv), du2 = dFdy(uv);
        vec3 dp2perp = cross(dp2, Ngeo), dp1perp = cross(Ngeo, dp1);
        vec3 T = dp2perp * du1.x + dp1perp * du2.x;
        vec3 B = dp2perp * du1.y + dp1perp * du2.y;
        float invmax = inversesqrt(max(max(dot(T, T), dot(B, B)), 1e-12));
        T *= invmax;
        B *= invmax;
        float e = 0.06;
        float h0 = flowHeight(uv);
        float hu = flowHeight(uv + vec2(e, 0.0)), hv = flowHeight(uv + vec2(0.0, e));
        N = normalize(Ngeo - (T * (hu - h0) + B * (hv - h0)) / e * 0.3 * detail);
    }

    float NdV = max(dot(N, V), 0.0);
    float F = 0.02 + 0.98 * pow(1.0 - NdV, 5.0);       // Fresnel (Schlick)
    float sh = shadowFactor(vWorldPos, Ngeo);
    vec3 ambientLight = (uSkyTop * 0.5 + uSkyHorizon * 0.5) * uAmbient;
    vec3 sunLight = uSunColor * max(dot(Ngeo, uSunDir), 0.0) * sh;

    // water body colour (light scattered inside the water)
    vec3 body = mix(uDeep, uShallow, 0.25 + 0.75 * pow(NdV, 0.6));
    body *= ambientLight * 1.6 + sunLight * 0.5;

    vec3 col;
    float alpha = uAlpha;
    if (fromBelow) {
        // under water looking up: refraction into air or total internal reflection
        vec3 Rt = refract(-V, N, 1.33);
        if (dot(Rt, Rt) > 0.0) {
            vec3 sky = uFogColor * 2.4 + vec3(0.25, 0.3, 0.3);
            sky += uSunColor * 1.5 * pow(max(dot(Rt, uSunDir), 0.0), 40.0);
            col = mix(sky, uFogColor * 0.6, F);
        } else {
            col = uFogColor * 0.55;
        }
        alpha = 0.95;
    } else if (uWaterMode == 3) {
        // cut face of a water body (cut-away view): blue gradient with depth
        float depth = max(uSurfaceY - vWorldPos.y, 0.0);
        vec3 c = mix(uShallow * 1.2, uDeep, clamp(depth / 9.0, 0.0, 1.0));
        c *= ambientLight * 1.5 + uSunColor * 0.25;
        float rip = causticPattern(vec2(vWorldPos.x * 0.09 - uTime * uFlowSpeed * 0.03, vWorldPos.y * 0.15));
        c += rip * uShallow * 0.6;
        float edge = 1.0 - smoothstep(0.0, 0.3, depth);           // bright line under the surface
        c += edge * 0.5 * vec3(0.75, 0.9, 1.0);
        col = c;
        alpha = clamp(uAlpha + edge * 0.3, 0.0, 1.0);
    } else {
        vec3 R = reflect(-V, N);
        vec3 refl = skyColor(R);
        vec3 H = normalize(uSunDir + V);
        float nh = max(dot(N, H), 0.0);
        float glitter = pow(nh, 900.0) * 30.0 + pow(nh, 90.0) * 0.8;
        col = mix(body, refl, F) + uSunColor * glitter * sh * 0.3;
        alpha = mix(uAlpha, 1.0, F * 0.7);
    }

    // ----------------------------- foam -----------------------------------
    float foam = 0.0;
    if (uWaterMode == 1) {
        float streak = vnoise2(vec2(uv.x * 2.4, uv.y * 0.07 - uTime * uFlowSpeed * 0.07));
        float fine = vnoise2(vec2(uv.x * 9.0, uv.y * 0.3 - uTime * uFlowSpeed * 0.3));
        foam = smoothstep(0.62, 0.95, streak * 0.65 + fine * 0.5) * uFoam;
        if (uSheetWidth > 0.0) foam += (1.0 - smoothstep(0.0, 1.3, min(uv.x, uSheetWidth - uv.x))) * 0.5 * uFoam;
        foam += smoothstep(uFront - 5.0, uFront, uv.y) * 0.85;            // churning leading edge
        foam += (1.0 - smoothstep(uTail, uTail + 4.0, uv.y)) * 0.6;       // thin trailing edge
    } else if (uWaterMode == 2) {
        float streak = vnoise2(vec2(uv.x * 2.0, uv.y * 0.3 - uTime * uFlowSpeed * 0.3));
        foam = smoothstep(0.62, 0.92, streak) * uFoam;
        foam += smoothstep(uFront - 3.0, uFront + uNoseLen, uv.y) * 0.35 * uFoam;
    } else if (!fromBelow) {
        if (uFoamSpot.w > 0.0) {
            float d = length(vWorldPos.xz - uFoamSpot.xy) / uFoamSpot.z;
            float n = fbm2(vWorldPos.xz * 0.3 + vec2(uTime * 0.25, -uTime * 0.5));
            foam += smoothstep(1.0, 0.15, d + (n - 0.5) * 0.9) * uFoamSpot.w;
        }
        if (uFoamSpot2.w > 0.0) {
            float d = length(vWorldPos.xz - uFoamSpot2.xy) / uFoamSpot2.z;
            float n = fbm2(vWorldPos.xz * 0.4 + vec2(uTime * 0.2, -uTime * 0.6));
            foam += smoothstep(1.0, 0.15, d + (n - 0.5) * 0.9) * uFoamSpot2.w;
        }
        if (uFoam > 0.0)
            foam += uFoam * smoothstep(0.55, 0.85, fbm2(vWorldPos.xz * 0.45 - vec2(uTime * uFlowSpeed * 0.45, 0.0)));
    }
    foam = clamp(foam, 0.0, 1.0);
    vec3 foamCol = vec3(0.9, 0.95, 1.0) * (ambientLight * 1.6 + uSunColor * 0.45 * (0.35 + 0.65 * sh));
    col = mix(col, foamCol, foam);
    alpha = max(alpha, foam);

    // ----------------------- sci-fi glowing streaks ------------------------
    if (uGlow > 0.0) {
        float s1 = vnoise2(vec2(uv.x * 3.0, uv.y * 0.18 - uTime * uFlowSpeed * 0.18));
        float s2 = vnoise2(vec2(uv.x * 9.0, uv.y * 0.5 - uTime * uFlowSpeed * 0.5));
        float lines = pow(1.0 - abs(s1 * 2.0 - 1.0), 6.0) + 0.6 * pow(1.0 - abs(s2 * 2.0 - 1.0), 10.0);
        col += vec3(0.25, 0.65, 1.0) * lines * uGlow;
    }

    col = applyFog(col, vWorldPos);
    FragColor = vec4(finalColor(col), alpha);
}
