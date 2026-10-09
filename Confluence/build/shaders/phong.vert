#version 330 core
// lighting_common.glsl + surface_common.glsl are pasted here by Shader.cpp.

// Program used for GOURAUD (uShadingMode 1) and PHONG (uShadingMode 2).
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec3 aColor;

uniform mat4 uModel, uView, uProjection;
uniform mat3 uNormalMatrix;      // (M^-1)^T -- correct under non-uniform scale
uniform int  uShadingMode;       // 1 = Gouraud, 2 = Phong

out vec3 vFragPos;        // world position     (blended across the triangle)
out vec3 vNormal;         // world normal       (blended across the triangle)
out vec3 vColor;          // vertex colour
out vec3 vGouraudColor;   // Gouraud only: lighting already done HERE, per vertex

void main() {
    vFragPos = vec3(uModel * vec4(aPos, 1.0));
    vNormal  = uNormalMatrix * aNormal;
    vColor   = aColor;

    // STEP 7: the lake surface moves. Its flat grid is lifted to the wave
    // height and given the wave's normal, right here on the GPU.
    if (uWaveMode == 1) {
        vFragPos.y = waveHeight(vFragPos.xz);
        vNormal = waveNormal(vFragPos.xz);
    }

    // GOURAUD: evaluate the lighting at the three corners only. The rasteriser
    // then blends these three colours across the triangle. A highlight whose
    // peak falls BETWEEN the corners is never sampled, so it is missed.
    vGouraudColor = vec3(0.0);
    if (uShadingMode == 1)
        vGouraudColor = computeLighting(normalize(vNormal), vFragPos, aColor);

    gl_Position = uProjection * uView * vec4(vFragPos, 1.0);
}
