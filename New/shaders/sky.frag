// sky.frag - background: for every pixel build the view ray and colour it.
#version 330 core
#include "common.glsl"
uniform vec3 uCamRight;
uniform vec3 uCamUp;
uniform vec3 uCamForward;
uniform float uTanHalfFov;
uniform float uAspect;
in vec2 vNdc;
out vec4 FragColor;
void main() {
    vec3 dir = normalize(uCamForward + vNdc.x * uTanHalfFov * uAspect * uCamRight + vNdc.y * uTanHalfFov * uCamUp);
    vec3 col = skyColor(dir);
    if (uSkyMode == 0) {
        // far haze near the horizon, same colour as the fog
        col = mix(col, uFogColor, (1.0 - smoothstep(0.0, 0.25, abs(dir.y))) * 0.55);
    }
    FragColor = vec4(finalColor(col), 1.0);
}
