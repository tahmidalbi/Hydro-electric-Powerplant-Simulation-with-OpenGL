#version 330 core
// lighting_common.glsl is pasted here by Shader.cpp (computeLighting).
//
// STRICT FLAT shading, stage 2 of 3: one colour per triangle.
// A geometry shader sees the whole triangle (all three corners) at once, so it
// can do what the roadmap asks: compute the lighting ONCE, at the triangle's
// centre with the triangle's own face normal, and give that same colour to all
// three corners. Nothing varies across the triangle, not even for a point light.
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in vec3 vFragPos[];
in vec3 vNormal[];
in vec3 vColor[];

out vec3 gFragPos;          // still per pixel, but used only for fog
out vec3 gFaceNormal;
flat out vec3 gFlatColor;   // "flat": no blending across the triangle

void main() {
    vec3 p0 = vFragPos[0], p1 = vFragPos[1], p2 = vFragPos[2];

    // Face normal from two triangle edges.
    vec3 averageVertexNormal = vNormal[0] + vNormal[1] + vNormal[2];
    vec3 edgeCross = cross(p1 - p0, p2 - p0);
    vec3 faceNormal = length(edgeCross) > 1e-12 ? normalize(edgeCross) : normalize(averageVertexNormal);

    // The cross product's sign depends on the corner order (winding). Check it
    // against the authored vertex normals and flip it to point outwards.
    if (dot(faceNormal, averageVertexNormal) < 0.0) faceNormal = -faceNormal;

    vec3 centre = (p0 + p1 + p2) / 3.0;
    vec3 albedo = (vColor[0] + vColor[1] + vColor[2]) / 3.0;
    vec3 triangleColor = computeLighting(faceNormal, centre, albedo);   // ONCE per triangle

    for (int i = 0; i < 3; ++i) {
        gl_Position = gl_in[i].gl_Position;
        gFragPos    = vFragPos[i];
        gFaceNormal = faceNormal;
        gFlatColor  = triangleColor;
        EmitVertex();
    }
    EndPrimitive();
}
