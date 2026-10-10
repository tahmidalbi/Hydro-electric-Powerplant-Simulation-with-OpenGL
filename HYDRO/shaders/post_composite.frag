// post_composite.frag - final picture: scene + bloom glow, vignette, film grain.
#version 330 core
uniform sampler2D uScene;
uniform sampler2D uBloom;
uniform float uStrength;
uniform float uTime;
in vec2 vNdc;
out vec4 FragColor;

float hash(vec2 p) {
    vec3 p3 = fract(vec3(p.xyx) * 0.1031);
    p3 += dot(p3, p3.yzx + 33.33);
    return fract((p3.x + p3.y) * p3.z);
}

void main() {
    vec2 uv = vNdc * 0.5 + 0.5;
    vec3 c = texture(uScene, uv).rgb;
    vec3 glow = texture(uBloom, uv).rgb;
    c += glow * uStrength;                                   // bloom
    c = 1.0 - (1.0 - c) * (1.0 - glow * uStrength * 0.25);   // soft screen blend for highlights
    vec2 d = uv - 0.5;
    c *= 1.0 - 0.38 * dot(d, d) * 2.0;                      // vignette
    c += (hash(gl_FragCoord.xy + fract(uTime * 7.0) * 1000.0) - 0.5) * 0.03;   // film grain
    FragColor = vec4(clamp(c, 0.0, 1.0), 1.0);
}
