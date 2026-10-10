#include "Mesh.h"
#include <glad/gl.h>
#include <cstddef>

// ============================================================================
//  Mesh (GPU side)
// ============================================================================
void Mesh::upload(const std::vector<Vertex>& v, const std::vector<unsigned int>& idx, bool dynamic) {
    if (!vao) {
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &ebo);
    }
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, v.size() * sizeof(Vertex), v.data(), dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx.size() * sizeof(unsigned int), idx.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);   // position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, pos));
    glEnableVertexAttribArray(1);   // normal
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(2);   // uv
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
    glBindVertexArray(0);
    indexCount = (int)idx.size();
}

void Mesh::updateVertices(const std::vector<Vertex>& v) {
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, v.size() * sizeof(Vertex), v.data());
}

void Mesh::draw() const {
    if (!indexCount) return;
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
}

// ============================================================================
//  MeshBuilder (CPU side geometry generation)
// ============================================================================
unsigned int MeshBuilder::addVertex(const Vec3& p, const Vec3& n, const Vec2& uv) {
    Vertex v;
    v.pos = xf.transformPoint(p);
    v.normal = normalize(xfNormal.transformDir(n));
    v.uv = uv;
    verts.push_back(v);
    return (unsigned int)verts.size() - 1;
}

void MeshBuilder::quad(const Vec3& a, const Vec3& b, const Vec3& c, const Vec3& d, float uvScale) {
    Vec3 n = normalize(cross(b - a, d - a));
    float w = length(b - a) * uvScale, h = length(d - a) * uvScale;
    unsigned int i0 = addVertex(a, n, Vec2(0, 0));
    unsigned int i1 = addVertex(b, n, Vec2(w, 0));
    unsigned int i2 = addVertex(c, n, Vec2(w, h));
    unsigned int i3 = addVertex(d, n, Vec2(0, h));
    tri(i0, i1, i2);
    tri(i0, i2, i3);
}

void MeshBuilder::box(const Vec3& mn, const Vec3& mx) {
    // Each face gets UVs in metres taken from the two coordinates that lie in it.
    Vec3 p[8] = {
        Vec3(mn.x, mn.y, mn.z), Vec3(mx.x, mn.y, mn.z), Vec3(mx.x, mx.y, mn.z), Vec3(mn.x, mx.y, mn.z),
        Vec3(mn.x, mn.y, mx.z), Vec3(mx.x, mn.y, mx.z), Vec3(mx.x, mx.y, mx.z), Vec3(mn.x, mx.y, mx.z)};
    struct Face { int a, b, c, d; Vec3 n; };
    Face faces[6] = {
        {4, 5, 6, 7, Vec3(0, 0, 1)},  {1, 0, 3, 2, Vec3(0, 0, -1)},
        {5, 1, 2, 6, Vec3(1, 0, 0)},  {0, 4, 7, 3, Vec3(-1, 0, 0)},
        {7, 6, 2, 3, Vec3(0, 1, 0)},  {0, 1, 5, 4, Vec3(0, -1, 0)}};
    for (const Face& f : faces) {
        Vec3 q[4] = {p[f.a], p[f.b], p[f.c], p[f.d]};
        unsigned int base = 0;
        for (int k = 0; k < 4; k++) {
            Vec2 uv;
            if (f.n.z != 0) uv = Vec2(q[k].x, q[k].y);
            else if (f.n.x != 0) uv = Vec2(q[k].z, q[k].y);
            else uv = Vec2(q[k].x, q[k].z);
            unsigned int id = addVertex(q[k], f.n, uv);
            if (k == 0) base = id;
        }
        tri(base, base + 1, base + 2);
        tri(base, base + 2, base + 3);
    }
}

void MeshBuilder::beam(const Vec3& p0, const Vec3& p1, float w, float h) {
    Vec3 d = p1 - p0;
    float len = length(d);
    if (len < 1e-5f) return;
    Mat4 saved = xf;
    setTransform(saved * alignY(p0, d));
    box(Vec3(-w * 0.5f, 0, -h * 0.5f), Vec3(w * 0.5f, len, h * 0.5f));
    setTransform(saved);
}

void MeshBuilder::cylinder(float r0, float r1, float h, int seg, bool capBottom, bool capTop, float a0, float a1) {
    float slope = (r0 - r1) / h;   // makes cone normals tilt correctly
    unsigned int start = (unsigned int)verts.size();
    for (int i = 0; i <= seg; i++) {
        float a = a0 + (a1 - a0) * i / seg;
        float c = std::cos(a), s = std::sin(a);
        Vec3 n = normalize(Vec3(c, slope, s));
        float arc = a * (r0 + r1) * 0.5f;
        addVertex(Vec3(c * r0, 0, s * r0), n, Vec2(arc, 0));
        addVertex(Vec3(c * r1, h, s * r1), n, Vec2(arc, h));
    }
    for (int i = 0; i < seg; i++) {
        unsigned int b = start + i * 2;
        tri(b, b + 1, b + 3);
        tri(b, b + 3, b + 2);
    }
    for (int cap = 0; cap < 2; cap++) {
        if ((cap == 0 && !capBottom) || (cap == 1 && !capTop)) continue;
        float y = cap == 0 ? 0 : h, r = cap == 0 ? r0 : r1;
        Vec3 n(0, cap == 0 ? -1.0f : 1.0f, 0);
        unsigned int centre = addVertex(Vec3(0, y, 0), n, Vec2(0, 0));
        for (int i = 0; i <= seg; i++) {
            float a = a0 + (a1 - a0) * i / seg;
            addVertex(Vec3(std::cos(a) * r, y, std::sin(a) * r), n, Vec2(std::cos(a) * r, std::sin(a) * r));
        }
        for (int i = 0; i < seg; i++) tri(centre, centre + 1 + i, centre + 2 + i);
    }
}

void MeshBuilder::cylinderBetween(const Vec3& p0, const Vec3& p1, float r, int seg, bool caps) {
    Vec3 d = p1 - p0;
    Mat4 saved = xf;
    setTransform(saved * alignY(p0, d));
    cylinder(r, r, length(d), seg, caps, caps);
    setTransform(saved);
}

void MeshBuilder::sphere(float r, int seg, int rings) {
    unsigned int start = (unsigned int)verts.size();
    for (int j = 0; j <= rings; j++) {
        float v = (float)j / rings, phi = v * PI;
        for (int i = 0; i <= seg; i++) {
            float u = (float)i / seg, th = u * 2.0f * PI;
            Vec3 n(std::sin(phi) * std::cos(th), std::cos(phi), std::sin(phi) * std::sin(th));
            addVertex(n * r, n, Vec2(u * 2.0f * PI * r, v * PI * r));
        }
    }
    for (int j = 0; j < rings; j++)
        for (int i = 0; i < seg; i++) {
            unsigned int a = start + j * (seg + 1) + i, b = a + seg + 1;
            tri(a, b, a + 1);
            tri(a + 1, b, b + 1);
        }
}

void MeshBuilder::torus(float R, float r, int segR, int segr) {
    unsigned int start = (unsigned int)verts.size();
    for (int i = 0; i <= segR; i++) {
        float u = (float)i / segR * 2.0f * PI;
        Vec3 c(std::cos(u) * R, 0, std::sin(u) * R);
        Vec3 out(std::cos(u), 0, std::sin(u));
        for (int j = 0; j <= segr; j++) {
            float v = (float)j / segr * 2.0f * PI;
            Vec3 n = out * std::cos(v) + Vec3(0, 1, 0) * std::sin(v);
            addVertex(c + n * r, n, Vec2(u * R, v * r));
        }
    }
    for (int i = 0; i < segR; i++)
        for (int j = 0; j < segr; j++) {
            unsigned int a = start + i * (segr + 1) + j, b = a + segr + 1;
            tri(a, b, a + 1);
            tri(a + 1, b, b + 1);
        }
}

void MeshBuilder::disk(float r, int seg) { annulus(0.0f, r, seg); }

void MeshBuilder::annulus(float rIn, float rOut, int seg) {
    unsigned int start = (unsigned int)verts.size();
    Vec3 n(0, 1, 0);
    for (int i = 0; i <= seg; i++) {
        float a = (float)i / seg * 2.0f * PI, c = std::cos(a), s = std::sin(a);
        addVertex(Vec3(c * rIn, 0, s * rIn), n, Vec2(c * rIn, s * rIn));
        addVertex(Vec3(c * rOut, 0, s * rOut), n, Vec2(c * rOut, s * rOut));
    }
    for (int i = 0; i < seg; i++) {
        unsigned int b = start + i * 2;
        tri(b, b + 1, b + 3);
        tri(b, b + 3, b + 2);
    }
}

void MeshBuilder::grid(float w, float d, int nx, int nz) {
    unsigned int start = (unsigned int)verts.size();
    for (int j = 0; j <= nz; j++)
        for (int i = 0; i <= nx; i++) {
            float x = -w * 0.5f + w * i / nx, z = -d * 0.5f + d * j / nz;
            addVertex(Vec3(x, 0, z), Vec3(0, 1, 0), Vec2(x, z));
        }
    for (int j = 0; j < nz; j++)
        for (int i = 0; i < nx; i++) {
            unsigned int a = start + j * (nx + 1) + i, b = a + nx + 1;
            tri(a, b, a + 1);
            tri(a + 1, b, b + 1);
        }
}

// ---------------------------------------------------------------------------
//  Paths
// ---------------------------------------------------------------------------
std::vector<PathFrame> buildPathFrames(const std::vector<Vec3>& path) {
    std::vector<PathFrame> f(path.size());
    float s = 0;
    for (size_t i = 0; i < path.size(); i++) {
        Vec3 t;
        if (i == 0) t = path[1] - path[0];
        else if (i + 1 == path.size()) t = path[i] - path[i - 1];
        else t = path[i + 1] - path[i - 1];
        t = normalize(t);
        if (i > 0) s += length(path[i] - path[i - 1]);
        // "parallel transport": keep the previous up vector as steady as possible
        Vec3 up = (i == 0) ? Vec3(0, 1, 0) : f[i - 1].up;
        up = up - t * dot(up, t);
        if (length(up) < 1e-4f) up = Vec3(1, 0, 0) - t * t.x;
        up = normalize(up);
        f[i].pos = path[i];
        f[i].tangent = t;
        f[i].up = up;
        f[i].side = normalize(cross(t, up));
        f[i].s = s;
    }
    return f;
}

std::vector<Vec3> roundedPath(const std::vector<Vec3>& c, float radius, float step) {
    // 1) build a dense list of points: straight parts + quadratic Bezier fillets
    std::vector<Vec3> dense;
    dense.push_back(c[0]);
    Vec3 cur = c[0];
    for (size_t i = 1; i + 1 < c.size(); i++) {
        Vec3 din = c[i] - c[i - 1], dout = c[i + 1] - c[i];
        float d = std::fmin(radius, std::fmin(length(din), length(dout)) * 0.45f);
        Vec3 A = c[i] - normalize(din) * d, B = c[i] + normalize(dout) * d;
        dense.push_back(A);
        for (int k = 1; k <= 24; k++) {
            float t = k / 24.0f;
            Vec3 p = A * ((1 - t) * (1 - t)) + c[i] * (2 * (1 - t) * t) + B * (t * t);
            dense.push_back(p);
        }
        cur = B;
    }
    dense.push_back(c.back());
    // 2) resample at an even spacing so tubes have uniform rings
    std::vector<Vec3> out;
    out.push_back(dense[0]);
    float carry = 0;
    for (size_t i = 1; i < dense.size(); i++) {
        Vec3 a = dense[i - 1], b = dense[i];
        float segLen = length(b - a);
        float pos = step - carry;
        while (pos <= segLen) {
            out.push_back(a + (b - a) * (pos / segLen));
            pos += step;
        }
        carry = segLen - (pos - step);
    }
    if (length(out.back() - dense.back()) > step * 0.2f) out.push_back(dense.back());
    return out;
}

void MeshBuilder::sweep(const std::vector<Vec3>& path, float r, int seg, float a0, float a1, bool inward) {
    std::vector<PathFrame> f = buildPathFrames(path);
    unsigned int start = (unsigned int)verts.size();
    for (size_t i = 0; i < f.size(); i++)
        for (int j = 0; j <= seg; j++) {
            float a = a0 + (a1 - a0) * j / seg;
            Vec3 radial = f[i].up * std::cos(a) + f[i].side * std::sin(a);
            addVertex(f[i].pos + radial * r, inward ? -radial : radial, Vec2(a * r, f[i].s));
        }
    for (size_t i = 0; i + 1 < f.size(); i++)
        for (int j = 0; j < seg; j++) {
            unsigned int a = start + (unsigned int)i * (seg + 1) + j, b = a + seg + 1;
            tri(a, b, a + 1);
            tri(a + 1, b, b + 1);
        }
}

void MeshBuilder::extrudeOutlineX(const std::vector<Vec2>& o, float x0, float x1, float baseY) {
    size_t n = o.size();
    std::vector<Vec3> segN(n - 1);
    for (size_t k = 0; k + 1 < n; k++) {
        float dz = o[k + 1].x - o[k].x, dy = o[k + 1].y - o[k].y;
        segN[k] = normalize(Vec3(0, dz, -dy));
    }
    const float smoothCos = std::cos(radians(32.0f));
    float v = 0;
    for (size_t k = 0; k + 1 < n; k++) {
        Vec3 nA = segN[k], nB = segN[k];
        if (k > 0 && dot(segN[k - 1], segN[k]) > smoothCos) nA = normalize(segN[k - 1] + segN[k]);
        if (k + 2 < n && dot(segN[k + 1], segN[k]) > smoothCos) nB = normalize(segN[k + 1] + segN[k]);
        float len = length(Vec3(0, o[k + 1].y - o[k].y, o[k + 1].x - o[k].x));
        unsigned int a = addVertex(Vec3(x0, o[k].y, o[k].x), nA, Vec2(x0, v));
        unsigned int b = addVertex(Vec3(x1, o[k].y, o[k].x), nA, Vec2(x1, v));
        unsigned int c = addVertex(Vec3(x1, o[k + 1].y, o[k + 1].x), nB, Vec2(x1, v + len));
        unsigned int d = addVertex(Vec3(x0, o[k + 1].y, o[k + 1].x), nB, Vec2(x0, v + len));
        tri(a, b, c);
        tri(a, c, d);
        v += len;
    }
    // side caps (the cut faces at x0 and x1)
    for (int side = 0; side < 2; side++) {
        float x = side == 0 ? x0 : x1;
        Vec3 nrm(side == 0 ? -1.0f : 1.0f, 0, 0);
        for (size_t k = 0; k + 1 < n; k++) {
            if (o[k + 1].x - o[k].x < 1e-4f) continue;
            unsigned int a = addVertex(Vec3(x, baseY, o[k].x), nrm, Vec2(o[k].x, baseY));
            unsigned int b = addVertex(Vec3(x, o[k].y, o[k].x), nrm, Vec2(o[k].x, o[k].y));
            unsigned int c = addVertex(Vec3(x, o[k + 1].y, o[k + 1].x), nrm, Vec2(o[k + 1].x, o[k + 1].y));
            unsigned int d = addVertex(Vec3(x, baseY, o[k + 1].x), nrm, Vec2(o[k + 1].x, baseY));
            tri(a, b, c);
            tri(a, c, d);
        }
    }
}

Mesh MeshBuilder::build(bool dynamic) const {
    Mesh m;
    m.upload(verts, idx, dynamic);
    return m;
}
