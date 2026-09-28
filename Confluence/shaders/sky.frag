#version 330 core
in vec2 vNdc;
out vec4 FragColor;

uniform mat4 uInvViewProj;
uniform vec3 uCamPos;
uniform vec3 uHorizon;
uniform vec3 uZenith;

// Vertical gradient keyed to the real view-ray elevation, so it stays
// consistent while orbiting: pale haze at the horizon, deep blue overhead.
void main() {
    vec4 world = uInvViewProj * vec4(vNdc, 1.0, 1.0);
    vec3 dir = normalize(world.xyz / world.w - uCamPos);
    float t = smoothstep(0.0, 0.55, max(dir.y, 0.0));
    FragColor = vec4(mix(uHorizon, uZenith, pow(t, 0.8)), 1.0);
}
