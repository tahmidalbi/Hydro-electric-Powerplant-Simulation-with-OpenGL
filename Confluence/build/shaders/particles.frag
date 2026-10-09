#version 330 core
// A soft, round, slightly lumpy puff: opaque in the middle, fading to zero at
// the edge, so overlapping puffs blend into one mist instead of showing discs.
in vec2 vCorner;
in vec4 vColor;
in float vSeed;
in vec3 vWorldPos;
out vec4 FragColor;

uniform vec3 uLightColor;    // sun + sky light reaching the spray
uniform vec3 uViewPos;
uniform vec3 uFogColor;
uniform float uFogDensity;

void main() {
    float r = length(vCorner);
    if (r > 1.0) discard;

    // Lumpy edge: the radius wobbles with the angle, different per particle.
    float angle = atan(vCorner.y, vCorner.x);
    float wobble = 0.85 + 0.15 * sin(angle * 3.0 + vSeed * 40.0) * sin(angle * 5.0 + vSeed * 17.0);
    float soft = 1.0 - smoothstep(0.25 * wobble, wobble, r);

    vec3 color = vColor.rgb * uLightColor;
    float d = length(vWorldPos - uViewPos) * uFogDensity;
    color = mix(color, uFogColor, 1.0 - exp(-d * d));
    FragColor = vec4(color, vColor.a * soft);
}
