// =============================================================================
//  Mesh.h  -  GPU mesh (VAO/VBO/EBO) + MeshBuilder that creates geometry.
//
//  Every object in the project is built from simple shapes generated here in
//  code (boxes, cylinders, spheres, tori, tubes along a path, grids, extruded
//  profiles).  No model files are loaded.
//
//  Vertex layout:  position (3 floats) | normal (3 floats) | uv (2 floats)
//  UVs are in METRES for most shapes, so procedural textures in the shaders
//  have the same size everywhere.
// =============================================================================
#pragma once
#include <vector>
#include "MathLib.h"

struct Vertex {
    Vec3 pos;
    Vec3 normal;
    Vec2 uv;
};

class Mesh {
public:
    unsigned int vao = 0, vbo = 0, ebo = 0;
    int indexCount = 0;

    void upload(const std::vector<Vertex>& verts, const std::vector<unsigned int>& idx, bool dynamic = false);
    void updateVertices(const std::vector<Vertex>& verts);   // for meshes that move every frame
    void draw() const;
    bool valid() const { return indexCount > 0; }
};

class MeshBuilder {
public:
    std::vector<Vertex> verts;
    std::vector<unsigned int> idx;
    Mat4 xf;          // every shape added is moved by this transform
    Mat4 xfNormal;    // matching normal matrix

    void setTransform(const Mat4& m) { xf = m; xfNormal = normalMatrix(m); }
    void resetTransform() { setTransform(Mat4()); }

    unsigned int addVertex(const Vec3& p, const Vec3& n, const Vec2& uv);
    void tri(unsigned int a, unsigned int b, unsigned int c) { idx.push_back(a); idx.push_back(b); idx.push_back(c); }

    // a,b,c,d in counter-clockwise order (seen from the front)
    void quad(const Vec3& a, const Vec3& b, const Vec3& c, const Vec3& d, float uvScale = 1.0f);

    void box(const Vec3& mn, const Vec3& mx);                       // axis aligned box
    void boxCentered(const Vec3& c, const Vec3& size) { box(c - size * 0.5f, c + size * 0.5f); }
    void beam(const Vec3& p0, const Vec3& p1, float w, float h);    // box from p0 to p1 (lattice members, bars)

    // Cylinder / cone along local +Y from y=0 to y=h. a0..a1 = angle range (for cut-away pipes).
    void cylinder(float r0, float r1, float h, int seg, bool capBottom, bool capTop,
                  float a0 = 0.0f, float a1 = 2.0f * PI);
    void cylinderBetween(const Vec3& p0, const Vec3& p1, float r, int seg, bool caps = true);
    void sphere(float r, int seg, int rings);
    void torus(float R, float r, int segR, int segr);               // ring lying in the XZ plane
    void disk(float r, int seg);                                    // facing +Y
    void annulus(float rIn, float rOut, int seg);                   // flat ring facing +Y
    void grid(float w, float d, int nx, int nz);                    // XZ plane, centred, facing +Y

    // Tube that follows a path (pipes, water inside pipes).  The angle is measured
    // from the path's "up" vector, so a0 = 90deg, a1 = 270deg gives the bottom half.
    // inward = true makes the normals point to the centre (inner wall of a pipe).
    void sweep(const std::vector<Vec3>& path, float radius, int seg, float a0, float a1, bool inward);

    // Extrude a 2D outline drawn in the (z, y) plane along X from x0 to x1.
    // The outline must go from the upstream bottom corner, over the top, to the
    // downstream bottom corner with z never decreasing (true for dam profiles).
    void extrudeOutlineX(const std::vector<Vec2>& outlineZY, float x0, float x1, float baseY);

    Mesh build(bool dynamic = false) const;
    void clear() { verts.clear(); idx.clear(); resetTransform(); }   // also forget the old transform
};

// Frames (tangent/up/side) along a path, used by sweep() and by objects that
// need to place rings or water along the same path.
struct PathFrame { Vec3 pos, tangent, up, side; float s; };
std::vector<PathFrame> buildPathFrames(const std::vector<Vec3>& path);

// A poly-line with rounded corners (radius r), sampled every 'step' metres.
std::vector<Vec3> roundedPath(const std::vector<Vec3>& corners, float radius, float step);
