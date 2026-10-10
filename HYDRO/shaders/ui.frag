// ui.frag - solid rectangles (uv.x < 0) or font glyphs from the font texture.
#version 330 core
uniform sampler2D uFont;
in vec2 vUV;
in vec4 vColor;
out vec4 FragColor;
void main() {
    float a = vColor.a;
    if (vUV.x >= 0.0) a *= texture(uFont, vUV).r;
    FragColor = vec4(vColor.rgb, a);
}
