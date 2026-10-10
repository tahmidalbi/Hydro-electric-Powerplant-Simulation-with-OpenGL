// lit.frag - PHONG shading: the normal is interpolated across the triangle and
// the full Blinn-Phong lighting equation is evaluated for EVERY PIXEL.
// Result: smooth, sharp specular highlights even on big triangles.
#version 330 core
#include "common.glsl"
#include "materials.glsl"

in vec3 vWorldPos;
in vec3 vNormal;
in vec2 vUV;
out vec4 FragColor;

void main() {
    vec3 N = normalize(vNormal);
    vec3 V = normalize(uCamPos - vWorldPos);
    if (uTwoSided == 1 && dot(N, V) < 0.0) N = -N;

    float spec = uSpecular, shin = uShininess, refl = uReflect, alpha = uAlpha;
    vec3 emis = uEmissive;
    vec3 albedo = surfaceAlbedo(vWorldPos, N, vUV, spec, shin, refl, alpha, emis);

    vec3 col;
    if (uPattern == PAT_BEAM || uPattern == PAT_FLOW) {
        col = emis;                                   // pure glow, no lighting
    } else {
        float sh = shadowFactor(vWorldPos, N);
        col = shade(albedo, N, vWorldPos, spec, shin, refl, sh) + emis;
    }
    col = applyFog(col, vWorldPos);
    FragColor = vec4(finalColor(col), alpha);
}
