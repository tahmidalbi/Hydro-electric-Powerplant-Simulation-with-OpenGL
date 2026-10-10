// blit.frag - draws a saved picture over the screen with a given opacity
// (used for the cross-dissolve between scenes).
#version 330 core
uniform sampler2D uTex;
uniform float uAlpha;
in vec2 vNdc;
out vec4 FragColor;
void main() {
    FragColor = vec4(texture(uTex, vNdc * 0.5 + 0.5).rgb, uAlpha);
}
