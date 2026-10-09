// post_bright.frag - keeps only the bright parts of the picture (for bloom).
#version 330 core
uniform sampler2D uTex;
in vec2 vNdc;
out vec4 FragColor;
void main() {
    vec2 uv = vNdc * 0.5 + 0.5;
    // average 4 pixels (we render at quarter size) to avoid flicker
    vec2 px = 1.0 / vec2(textureSize(uTex, 0));
    vec3 c = 0.25 * (texture(uTex, uv + px * vec2(-1, -1)).rgb + texture(uTex, uv + px * vec2(1, -1)).rgb +
                     texture(uTex, uv + px * vec2(-1, 1)).rgb + texture(uTex, uv + px * vec2(1, 1)).rgb);
    float lum = dot(c, vec3(0.2126, 0.7152, 0.0722));
    float k = smoothstep(0.78, 1.0, lum);           // soft threshold: only really bright parts
    FragColor = vec4(c * k, 1.0);
}
