#include "ParticleRenderer.h"
#include "../systems/Particles.h"
#include <glad/gl.h>
#include <algorithm>
#include <cstddef>

ParticleRenderer::ParticleRenderer() : shader("shaders/particles.vert", "shaders/particles.frag") {
    const int capacity = ParticleSystem::CAPACITY;

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);
    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)(capacity * 4 * sizeof(GpuVertex)), nullptr, GL_STREAM_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GpuVertex), (void*)offsetof(GpuVertex, centre));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(GpuVertex), (void*)offsetof(GpuVertex, corner));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(GpuVertex), (void*)offsetof(GpuVertex, color));
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(GpuVertex), (void*)offsetof(GpuVertex, sizeFlatSeed));

    // The index pattern never changes: quad i uses vertices 4i .. 4i+3.
    std::vector<unsigned int> indices;
    indices.reserve((size_t)capacity * 6);
    for (unsigned int i = 0; i < (unsigned int)capacity; ++i) {
        unsigned int b = i * 4;
        indices.insert(indices.end(), {b, b + 1, b + 2, b, b + 2, b + 3});
    }
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)(indices.size() * sizeof(unsigned int)), indices.data(), GL_STATIC_DRAW);
    glBindVertexArray(0);

    vertices.reserve((size_t)capacity * 4);
    order.reserve(capacity);
}

ParticleRenderer::~ParticleRenderer() {
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);
    glDeleteVertexArrays(1, &vao);
}

void ParticleRenderer::draw(const ParticleSystem& system, const glm::mat4& view, const glm::mat4& projection,
                            const glm::vec3& cameraPos, const glm::vec3& lightColor,
                            const glm::vec3& fogColor, float fogDensity) {
    const std::vector<Particle>& pool = system.slots();

    // 1. Live particles, far -> near.
    order.clear();
    for (int i = 0; i < (int)pool.size(); ++i)
        if (pool[i].alive()) order.push_back(i);
    if (order.empty()) return;
    std::sort(order.begin(), order.end(), [&](int a, int b) {
        glm::vec3 da = pool[a].position - cameraPos, db = pool[b].position - cameraPos;
        return glm::dot(da, da) > glm::dot(db, db);
    });

    // 2. Four vertices per particle.
    static const glm::vec2 CORNERS[4] = {{-1, -1}, {1, -1}, {1, 1}, {-1, 1}};
    vertices.clear();
    for (int i : order) {
        const Particle& p = pool[i];
        glm::vec4 color(p.color, p.currentOpacity());
        glm::vec3 sizeFlatSeed(p.currentSize() * 0.5f, p.flat ? 1.0f : 0.0f, p.seed);
        for (const glm::vec2& c : CORNERS) vertices.push_back({p.position, c, color, sizeFlatSeed});
    }
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, (GLsizeiptr)(vertices.size() * sizeof(GpuVertex)), vertices.data());

    // The camera's right and up directions in world space are the first two
    // rows of the view matrix's rotation part.
    glm::vec3 right(view[0][0], view[1][0], view[2][0]);
    glm::vec3 up(view[0][1], view[1][1], view[2][1]);

    shader.use();
    shader.setMat4("uView", view);
    shader.setMat4("uProjection", projection);
    shader.setVec3("uCameraRight", right);
    shader.setVec3("uCameraUp", up);
    shader.setVec3("uLightColor", lightColor);
    shader.setVec3("uViewPos", cameraPos);
    shader.setVec3("uFogColor", fogColor);
    shader.setFloat("uFogDensity", fogDensity);

    // 3. Blended, depth-tested, not depth-written.
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, (GLsizei)(order.size() * 6), GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}
