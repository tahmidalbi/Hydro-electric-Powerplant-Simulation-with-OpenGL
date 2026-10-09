#version 330 core
// lighting_common.glsl is pasted here by Shader.cpp (finishColor).
//
// STRICT FLAT shading, stage 3 of 3: just paint the triangle's single colour.
in vec3 gFragPos;
in vec3 gFaceNormal;
flat in vec3 gFlatColor;
out vec4 FragColor;

void main() {
    FragColor = finishColor(gFlatColor, gFragPos, gFaceNormal);
}
