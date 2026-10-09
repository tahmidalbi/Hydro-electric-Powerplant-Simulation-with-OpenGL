#version 330 core
// The HUD is drawn in SCREEN PIXELS: (0,0) top-left, (width, height) bottom-right.
// This turns pixels into the -1..1 range OpenGL expects (y flipped: pixels grow down).
layout (location = 0) in vec2 aPixel;
layout (location = 1) in vec4 aColor;

uniform vec2 uScreenSize;

out vec4 vColor;

void main() {
    vec2 ndc = vec2(aPixel.x / uScreenSize.x * 2.0 - 1.0, 1.0 - aPixel.y / uScreenSize.y * 2.0);
    gl_Position = vec4(ndc, 0.0, 1.0);
    vColor = aColor;
}
