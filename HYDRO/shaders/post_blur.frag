// post_blur.frag - 9-tap Gaussian blur in one direction (uDir = step in uv).
#version 330 core
uniform sampler2D uTex;
uniform vec2 uDir;
in vec2 vNdc;
out vec4 FragColor;
void main() {
    vec2 uv = vNdc * 0.5 + 0.5;
    float w[5] = float[](0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);
    vec3 c = texture(uTex, uv).rgb * w[0];
    for (int i = 1; i < 5; i++) {
        c += texture(uTex, uv + uDir * float(i) * 1.4).rgb * w[i];
        c += texture(uTex, uv - uDir * float(i) * 1.4).rgb * w[i];
    }
    FragColor = vec4(c, 1.0);
}
