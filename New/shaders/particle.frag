// particle.frag - soft round sprite; mist gets a little noise so it looks puffy.
#version 330 core
#include "common.glsl"
uniform int uSoftNoise;
in vec2 vUV;
in vec4 vColor;
in vec3 vWorldPos;
out vec4 FragColor;
void main() {
    vec2 p = vUV * 2.0 - 1.0;
    float r2 = dot(p, p);
    if (r2 > 1.0) discard;
    float a = 1.0 - r2;
    a *= a;
    if (uSoftNoise == 1) a *= 0.5 + 0.7 * vnoise2(vUV * 3.5 + vColor.a * 37.0 + vWorldPos.xy * 0.2);
    vec3 col = applyFog(vColor.rgb, vWorldPos);
    FragColor = vec4(finalColor(col), clamp(a * vColor.a, 0.0, 1.0));
}
