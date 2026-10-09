#include "ParticleSystem.h"
#include <glad/gl.h>

void ParticleSystem::update(float dt) {
    for (size_t i = 0; i < particles.size();) {
        Particle& p = particles[i];
        p.life += dt;
        if (p.life >= p.maxLife) {                 // dead: swap with the last one and remove
            particles[i] = particles.back();
            particles.pop_back();
            continue;
        }
        p.vel.y += p.gravity * dt;                 // gravity
        p.vel *= 1.0f / (1.0f + p.drag * dt);      // air / water resistance
        p.pos += p.vel * dt;                       // move
        p.size += p.grow * dt;
        i++;
    }
}

void ParticleSystem::draw(Renderer& r) {
    if (particles.empty() || r.inShadowPass()) return;
    if (!vao) {
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        const int stride = 9 * sizeof(float);   // pos3 uv2 color4
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, stride, (void*)(5 * sizeof(float)));
    }
    // build two triangles per particle, facing the camera
    buffer.clear();
    Vec3 right = r.camRight, up = r.camUp;
    for (const Particle& p : particles) {
        float k = p.life / p.maxLife;
        // fade in quickly, fade out slowly
        float a = p.alpha * std::fmin(1.0f, k * 8.0f) * (1.0f - k);
        Vec3 R = right * p.size, U = up * p.size;
        Vec3 c[4] = {p.pos - R - U, p.pos + R - U, p.pos + R + U, p.pos - R + U};
        float uv[4][2] = {{0, 0}, {1, 0}, {1, 1}, {0, 1}};
        int order[6] = {0, 1, 2, 0, 2, 3};
        for (int o : order) {
            buffer.insert(buffer.end(), {c[o].x, c[o].y, c[o].z, uv[o][0], uv[o][1], p.color.x, p.color.y, p.color.z, a});
        }
    }
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, buffer.size() * sizeof(float), buffer.data(), GL_STREAM_DRAW);

    r.particle.use();
    r.particle.set("uSoftNoise", softNoise ? 1 : 0);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, additive ? GL_ONE : GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);
    glDrawArrays(GL_TRIANGLES, 0, (int)(buffer.size() / 9));
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}
