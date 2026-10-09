#version 330 core
// lighting_common.glsl + surface_common.glsl are pasted here by Shader.cpp.

in vec3 vFragPos;
in vec3 vNormal;
in vec3 vColor;
in vec3 vGouraudColor;
out vec4 FragColor;

uniform int uShadingMode;   // 1 = Gouraud, 2 = Phong

void main() {
    // The blended normal is shorter than 1 between the corners: normalise it.
    vec3 N = normalize(vNormal);

    vec3 color;
    if (uShadingMode == 1) {
        color = vGouraudColor;                          // GOURAUD: blended corner colours
    } else {
        // PHONG: lighting for THIS pixel. On the lake the exact wave normal
        // (plus tiny ripples) is computed here instead of being blended.
        if (uWaveMode == 1) N = waveNormalDetailed(vFragPos.xz);
        color = computeLighting(N, vFragPos, vColor);
    }
    FragColor = finishColor(color, vFragPos, N);
}
