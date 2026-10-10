// lit.vert - PHONG shading, vertex part: only moves the vertex and hands the
// world position + normal to the fragment shader, which does the lighting.
#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aUV;

uniform mat4 uModel;       // object -> world   (the TRANSFORMATION of this object)
uniform mat4 uNormalMat;   // inverse-transpose of uModel, for normals
uniform mat4 uView;        // world -> camera
uniform mat4 uProj;        // camera -> screen (perspective)

out vec3 vWorldPos;
out vec3 vNormal;
out vec2 vUV;

void main() {
    vec4 wp = uModel * vec4(aPos, 1.0);
    vWorldPos = wp.xyz;
    vNormal = mat3(uNormalMat) * aNormal;
    vUV = aUV;
    gl_Position = uProj * uView * wp;
}
