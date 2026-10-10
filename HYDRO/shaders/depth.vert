// depth.vert - shadow map pass: draw the scene as seen from the sun.
#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 2) in vec2 aUV;
uniform mat4 uLightSpace;
uniform mat4 uModel;
out vec2 vUV;
void main() {
    vUV = aUV;
    gl_Position = uLightSpace * uModel * vec4(aPos, 1.0);
}
