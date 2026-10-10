// gouraud.vert - GOURAUD shading: the lighting equation is evaluated only at
// the VERTICES here; the resulting light colours are interpolated across each
// triangle by the GPU. Cheaper, but highlights look blurry on big triangles.
// Press G in the program to compare with Phong.
#version 330 core
#include "common.glsl"
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aUV;

uniform mat4 uModel;
uniform mat4 uNormalMat;
uniform float uSpecular;
uniform float uShininess;

out vec3 vWorldPos;
out vec3 vNormal;
out vec2 vUV;
out vec3 vAmbient;    // ambient + point lights (not shadowed)
out vec3 vSunDiffuse; // sun diffuse  (the shadow is applied per pixel)
out vec3 vSunSpec;    // sun specular

void main() {
    vec4 wp = uModel * vec4(aPos, 1.0);
    vec3 N = normalize(mat3(uNormalMat) * aNormal);
    vec3 V = normalize(uCamPos - wp.xyz);

    float hemi = N.y * 0.5 + 0.5;
    vAmbient = mix(uGroundColor, uSkyTop * 0.55 + uSkyHorizon * 0.45, hemi) * uAmbient;
    for (int i = 0; i < uNumPoint; i++) {
        vec3 d = uPointPos[i] - wp.xyz;
        float dist = length(d);
        vAmbient += uPointColor[i] * max(dot(N, d / dist), 0.0) / (1.0 + 0.06 * dist * dist);
    }
    float ndl = max(dot(N, uSunDir), 0.0);
    vec3 H = normalize(uSunDir + V);
    vSunDiffuse = uSunColor * ndl;
    vSunSpec = uSunColor * pow(max(dot(N, H), 0.0), uShininess) * uSpecular
             * (uShininess + 8.0) / 32.0 * smoothstep(0.0, 0.15, ndl);

    vWorldPos = wp.xyz;
    vNormal = N;
    vUV = aUV;
    gl_Position = uProj * uView * wp;
}
