// gouraud.frag - only combines the light colours computed at the vertices with
// the surface texture. No lighting maths happens per pixel (that is Gouraud).
#version 330 core
#include "common.glsl"
#include "materials.glsl"

in vec3 vWorldPos;
in vec3 vNormal;
in vec2 vUV;
in vec3 vAmbient;
in vec3 vSunDiffuse;
in vec3 vSunSpec;
out vec4 FragColor;

void main() {
    vec3 N = normalize(vNormal);
    float spec = uSpecular, shin = uShininess, refl = uReflect, alpha = uAlpha;
    vec3 emis = uEmissive;
    vec3 albedo = surfaceAlbedo(vWorldPos, N, vUV, spec, shin, refl, alpha, emis);
    float sh = shadowFactor(vWorldPos, N);
    vec3 col = albedo * (vAmbient + vSunDiffuse * sh) + vSunSpec * sh + emis;
    col += albedo * uSunColor * caustics(vWorldPos, N) * sh;
    col = applyFog(col, vWorldPos);
    FragColor = vec4(finalColor(col), alpha);
}
