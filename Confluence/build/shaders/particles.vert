#version 330 core
// Particles (spray, mist, wake). Each particle is ONE quad: 4 vertices that
// share the particle's centre and differ only in `aCorner` (-1/+1, -1/+1).
//   billboard: the quad is spread along the camera's right/up vectors, so it
//              always faces the viewer (spray, mist)
//   flat:      the quad is spread along world x/z, so it lies on the water (wake)
layout (location = 0) in vec3 aCentre;
layout (location = 1) in vec2 aCorner;
layout (location = 2) in vec4 aColor;        // rgb + opacity
layout (location = 3) in vec3 aSizeFlatSeed; // size (m), flat flag, random seed

uniform mat4 uView, uProjection;
uniform vec3 uCameraRight, uCameraUp;

out vec2 vCorner;
out vec4 vColor;
out float vSeed;
out vec3 vWorldPos;

void main() {
    float size = aSizeFlatSeed.x;
    vec3 offset = aSizeFlatSeed.y > 0.5
        ? vec3(aCorner.x, 0.0, aCorner.y) * size
        : (uCameraRight * aCorner.x + uCameraUp * aCorner.y) * size;
    vWorldPos = aCentre + offset;
    vCorner = aCorner;
    vColor = aColor;
    vSeed = aSizeFlatSeed.z;
    gl_Position = uProjection * uView * vec4(vWorldPos, 1.0);
}
