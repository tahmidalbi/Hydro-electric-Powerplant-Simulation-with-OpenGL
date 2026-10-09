// particle.vert - spray, mist, bubbles, dust. The quads are already turned to
// face the camera on the CPU (billboards).
#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aUV;
layout(location = 2) in vec4 aColor;
uniform mat4 uView;
uniform mat4 uProj;
out vec2 vUV;
out vec4 vColor;
out vec3 vWorldPos;
void main() {
    vUV = aUV;
    vColor = aColor;
    vWorldPos = aPos;
    gl_Position = uProj * uView * vec4(aPos, 1.0);
}
