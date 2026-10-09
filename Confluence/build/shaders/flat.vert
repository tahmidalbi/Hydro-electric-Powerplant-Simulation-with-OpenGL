#version 330 core
// lighting_common.glsl + surface_common.glsl are pasted here by Shader.cpp.
//
// STRICT FLAT shading, stage 1 of 3: only transforms the corners.
// No lighting here -- the geometry shader lights the whole triangle at once.
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec3 aColor;

uniform mat4 uModel, uView, uProjection;
uniform mat3 uNormalMatrix;

out vec3 vFragPos;
out vec3 vNormal;
out vec3 vColor;

void main() {
    vFragPos = vec3(uModel * vec4(aPos, 1.0));
    vNormal  = uNormalMatrix * aNormal;
    vColor   = aColor;
    if (uWaveMode == 1) {                 // the lake follows the waves here too
        vFragPos.y = waveHeight(vFragPos.xz);
        vNormal = waveNormal(vFragPos.xz);
    }
    gl_Position = uProjection * uView * vec4(vFragPos, 1.0);
}
