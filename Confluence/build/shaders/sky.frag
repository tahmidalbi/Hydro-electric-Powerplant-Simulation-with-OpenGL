#version 330 core
// lighting_common.glsl + surface_common.glsl are pasted here by Shader.cpp,
// so the sky and the water's reflection use the SAME skyColour() function.
in vec2 vNdc;
out vec4 FragColor;

uniform mat4 uInvViewProj;
uniform vec3 uCamPos;

// Turn this pixel back into a view ray, then colour it by where it points:
// pale haze at the horizon, deep blue overhead, a warm glow around the sun.
void main() {
    vec4 world = uInvViewProj * vec4(vNdc, 1.0, 1.0);
    vec3 dir = normalize(world.xyz / world.w - uCamPos);
    FragColor = vec4(skyColour(dir), 1.0);
}
