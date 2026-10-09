#include "Mesh.h"
#include <glad/gl.h>
#include <glm/gtc/matrix_inverse.hpp>
#include <algorithm>
#include <cmath>
#include <cstddef>

namespace {

constexpr float PI = 3.14159265358979323846f;

// Flat-shaded quad whose normal is forced to point away from `inside`.
void addQuad(MeshData& d, glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 e, glm::vec3 inside) {
    glm::vec3 n = glm::cross(b - a, c - a);
    if (glm::length(n) < 1e-9f) n = glm::cross(c - a, e - a);
    n = glm::normalize(n);
    if (glm::dot(n, (a + b + c + e) * 0.25f - inside) < 0.0f) n = -n;
    unsigned int base = (unsigned int)d.vertices.size();
    d.vertices.push_back({a, n});
    d.vertices.push_back({b, n});
    d.vertices.push_back({c, n});
    d.vertices.push_back({e, n});
    d.indices.insert(d.indices.end(), {base, base + 1, base + 2, base, base + 2, base + 3});
}

void addDisc(MeshData& d, float y, float r, int segments, float ny) {
    unsigned int center = (unsigned int)d.vertices.size();
    d.vertices.push_back({{0.0f, y, 0.0f}, {0.0f, ny, 0.0f}});
    for (int i = 0; i <= segments; ++i) {
        float a = 2.0f * PI * i / segments;
        d.vertices.push_back({{std::cos(a) * r, y, std::sin(a) * r}, {0.0f, ny, 0.0f}});
    }
    for (int i = 0; i < segments; ++i)
        d.indices.insert(d.indices.end(), {center, center + 1 + i, center + 2 + i});
}

}

// Grows capacity geometrically (at least doubling). Reserving exactly
// "size + n" on every append would reallocate and copy the whole array each
// time -- quadratic work. The forest alone appends thousands of trees, which
// used to take ~10 s at start-up.
template <typename T>
static void reserveForAppend(std::vector<T>& v, size_t extra) {
    size_t needed = v.size() + extra;
    if (needed > v.capacity()) v.reserve(std::max(needed, v.capacity() * 2));
}

void MeshData::append(const MeshData& other, const glm::mat4& model, const glm::vec3& color) {
    glm::mat3 normalMatrix = glm::mat3(glm::transpose(glm::inverse(model)));
    unsigned int base = (unsigned int)vertices.size();
    reserveForAppend(vertices, other.vertices.size());
    for (const Vertex& v : other.vertices) {
        Vertex t;
        t.position = glm::vec3(model * glm::vec4(v.position, 1.0f));
        t.normal = glm::normalize(normalMatrix * v.normal);
        t.color = v.color * color;
        vertices.push_back(t);
    }
    reserveForAppend(indices, other.indices.size());
    for (unsigned int i : other.indices) indices.push_back(base + i);
}

Mesh::Mesh(const MeshData& data) {
    indexCount = (int)data.indices.size();

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, data.vertices.size() * sizeof(Vertex), data.vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, data.indices.size() * sizeof(unsigned int), data.indices.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));

    glBindVertexArray(0);
}

void Mesh::release() {
    if (VAO) glDeleteVertexArrays(1, &VAO);
    if (VBO) glDeleteBuffers(1, &VBO);
    if (EBO) glDeleteBuffers(1, &EBO);
    VAO = VBO = EBO = 0;
    indexCount = 0;
}

Mesh::~Mesh() { release(); }

Mesh::Mesh(Mesh&& other) noexcept
    : VAO(other.VAO), VBO(other.VBO), EBO(other.EBO), indexCount(other.indexCount) {
    other.VAO = other.VBO = other.EBO = 0;
    other.indexCount = 0;
}

Mesh& Mesh::operator=(Mesh&& other) noexcept {
    if (this != &other) {
        release();
        VAO = other.VAO; VBO = other.VBO; EBO = other.EBO; indexCount = other.indexCount;
        other.VAO = other.VBO = other.EBO = 0;
        other.indexCount = 0;
    }
    return *this;
}

void Mesh::draw() const {
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

// ---------------------------------------------------------------------
// Primitive factories
// ---------------------------------------------------------------------

MeshData cubeData() {
    MeshData d;
    glm::vec3 p[8] = {{-0.5f, -0.5f, -0.5f}, {0.5f, -0.5f, -0.5f}, {0.5f, 0.5f, -0.5f}, {-0.5f, 0.5f, -0.5f},
                      {-0.5f, -0.5f,  0.5f}, {0.5f, -0.5f,  0.5f}, {0.5f, 0.5f,  0.5f}, {-0.5f, 0.5f,  0.5f}};
    glm::vec3 c(0.0f);
    addQuad(d, p[4], p[5], p[6], p[7], c);
    addQuad(d, p[1], p[0], p[3], p[2], c);
    addQuad(d, p[5], p[1], p[2], p[6], c);
    addQuad(d, p[0], p[4], p[7], p[3], c);
    addQuad(d, p[7], p[6], p[2], p[3], c);
    addQuad(d, p[0], p[1], p[5], p[4], c);
    return d;
}

MeshData wedgeData(float topFraction) {
    MeshData d;
    float zt = -0.5f + glm::clamp(topFraction, 0.02f, 1.0f);
    glm::vec3 b0(-0.5f, -0.5f, -0.5f), b1(0.5f, -0.5f, -0.5f), b2(0.5f, -0.5f, 0.5f), b3(-0.5f, -0.5f, 0.5f);
    glm::vec3 t0(-0.5f, 0.5f, -0.5f), t1(0.5f, 0.5f, -0.5f), t2(0.5f, 0.5f, zt), t3(-0.5f, 0.5f, zt);
    glm::vec3 c = (b0 + b1 + b2 + b3 + t0 + t1 + t2 + t3) * 0.125f;
    addQuad(d, b0, b1, t1, t0, c);   // vertical back face
    addQuad(d, b0, b1, b2, b3, c);   // base
    addQuad(d, t0, t1, t2, t3, c);   // crest
    addQuad(d, b3, b2, t2, t3, c);   // sloped front face
    addQuad(d, b0, b3, t3, t0, c);   // -X end
    addQuad(d, b1, b2, t2, t1, c);   // +X end
    return d;
}

MeshData cylinderData(int segments, float topScale) {
    MeshData d;
    const float r0 = 0.5f, r1 = 0.5f * topScale;
    for (int i = 0; i <= segments; ++i) {
        float a = 2.0f * PI * i / segments;
        float c = std::cos(a), s = std::sin(a);
        glm::vec3 n = glm::normalize(glm::vec3(c, r0 - r1, s));
        d.vertices.push_back({{c * r0, -0.5f, s * r0}, n});
        d.vertices.push_back({{c * r1,  0.5f, s * r1}, n});
    }
    for (int i = 0; i < segments; ++i) {
        unsigned int b = (unsigned int)(i * 2);
        d.indices.insert(d.indices.end(), {b, b + 1, b + 2, b + 1, b + 3, b + 2});
    }
    addDisc(d,  0.5f, r1, segments,  1.0f);
    addDisc(d, -0.5f, r0, segments, -1.0f);
    return d;
}

MeshData sphereData(int stacks, int slices) {
    MeshData d;
    const float r = 0.5f;
    for (int i = 0; i <= stacks; ++i) {
        float phi = PI * i / stacks;
        float y = std::cos(phi) * r, ring = std::sin(phi) * r;
        for (int j = 0; j <= slices; ++j) {
            float theta = 2.0f * PI * j / slices;
            glm::vec3 pos(std::cos(theta) * ring, y, std::sin(theta) * ring);
            d.vertices.push_back({pos, glm::normalize(pos)});
        }
    }
    for (int i = 0; i < stacks; ++i) {
        for (int j = 0; j < slices; ++j) {
            unsigned int a = (unsigned int)(i * (slices + 1) + j);
            unsigned int b = a + (unsigned int)(slices + 1);
            // Counter-clockwise seen from outside (computeSmoothNormals relies on it).
            d.indices.insert(d.indices.end(), {a, a + 1, b, b, a + 1, b + 1});
        }
    }
    return d;
}

MeshData coneData(int segments) {
    MeshData d;
    const float r = 0.5f;
    for (int i = 0; i < segments; ++i) {
        float a0 = 2.0f * PI * i / segments, a1 = 2.0f * PI * (i + 1) / segments, am = 0.5f * (a0 + a1);
        // Apex duplicated per face with the mid-angle normal, so the tip shades smoothly.
        unsigned int base = (unsigned int)d.vertices.size();
        d.vertices.push_back({{std::cos(a0) * r, -0.5f, std::sin(a0) * r}, glm::normalize(glm::vec3(std::cos(a0), r, std::sin(a0)))});
        d.vertices.push_back({{std::cos(a1) * r, -0.5f, std::sin(a1) * r}, glm::normalize(glm::vec3(std::cos(a1), r, std::sin(a1)))});
        d.vertices.push_back({{0.0f, 0.5f, 0.0f}, glm::normalize(glm::vec3(std::cos(am), r, std::sin(am)))});
        d.indices.insert(d.indices.end(), {base, base + 2, base + 1});
    }
    addDisc(d, -0.5f, r, segments, -1.0f);
    return d;
}

MeshData quadData() {
    MeshData d;
    glm::vec3 n(0.0f, 0.0f, 1.0f);
    d.vertices = {{{-0.5f, -0.5f, 0.0f}, n}, {{0.5f, -0.5f, 0.0f}, n}, {{0.5f, 0.5f, 0.0f}, n}, {{-0.5f, 0.5f, 0.0f}, n}};
    d.indices = {0, 1, 2, 0, 2, 3};
    return d;
}

MeshData gridData(int subdivisions) {
    MeshData d;
    int N = subdivisions < 1 ? 1 : subdivisions;
    for (int zi = 0; zi <= N; ++zi)
        for (int xi = 0; xi <= N; ++xi)
            d.vertices.push_back({{(float)xi / N - 0.5f, 0.0f, (float)zi / N - 0.5f}, {0.0f, 1.0f, 0.0f}});
    for (int zi = 0; zi < N; ++zi) {
        for (int xi = 0; xi < N; ++xi) {
            unsigned int a = (unsigned int)(zi * (N + 1) + xi);
            unsigned int c = a + (unsigned int)(N + 1);
            d.indices.insert(d.indices.end(), {a, c, a + 1, a + 1, c, c + 1});
        }
    }
    return d;
}

void computeSmoothNormals(std::vector<Vertex>& v, const std::vector<unsigned int>& idx) {
    for (Vertex& vert : v) vert.normal = glm::vec3(0.0f);
    for (size_t i = 0; i + 2 < idx.size(); i += 3) {
        glm::vec3 a = v[idx[i]].position, b = v[idx[i + 1]].position, c = v[idx[i + 2]].position;
        glm::vec3 faceN = glm::cross(b - a, c - a);   // |faceN| = 2 * area: area-weighted sum
        v[idx[i]].normal += faceN;
        v[idx[i + 1]].normal += faceN;
        v[idx[i + 2]].normal += faceN;
    }
    for (Vertex& vert : v) {
        float len = glm::length(vert.normal);
        vert.normal = len > 1e-12f ? vert.normal / len : glm::vec3(0.0f, 1.0f, 0.0f);
    }
}
