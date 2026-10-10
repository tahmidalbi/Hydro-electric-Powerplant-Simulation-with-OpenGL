// depth.frag - only depth is written. The wire mesh (grating) cuts holes so
// its shadow is a mesh pattern too.
#version 330 core
in vec2 vUV;
uniform int uPattern;
uniform float uPatternScale;
void main() {
    if (uPattern == 8) {
        vec2 p = vUV * uPatternScale;
        vec2 q = fract(vec2(p.x + p.y, p.x - p.y));
        if (min(min(q.x, 1.0 - q.x), min(q.y, 1.0 - q.y)) > 0.11) discard;
    }
}
