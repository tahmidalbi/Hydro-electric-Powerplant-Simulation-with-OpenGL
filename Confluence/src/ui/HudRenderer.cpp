#include "HudRenderer.h"
#include "BitmapFont.h"
#include <glad/gl.h>
#include <algorithm>
#include <cmath>
#include <cstddef>

HudRenderer::HudRenderer() : shader("shaders/hud.vert", "shaders/hud.frag") {
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, pixel));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));
    glBindVertexArray(0);
}

HudRenderer::~HudRenderer() {
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
}

void HudRenderer::begin(int w, int h) {
    screenW = std::max(1, w);
    screenH = std::max(1, h);
    vertices.clear();
}

void HudRenderer::end() {
    if (vertices.empty()) return;
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    // "Orphan" the old buffer and upload this frame's vertices.
    glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)(vertices.size() * sizeof(Vertex)), vertices.data(), GL_STREAM_DRAW);

    shader.use();
    shader.setVec2("uScreenSize", glm::vec2((float)screenW, (float)screenH));
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, (GLsizei)vertices.size());
    glBindVertexArray(0);
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
}

void HudRenderer::quad(glm::vec2 a, glm::vec2 b, glm::vec2 c, glm::vec2 d, const glm::vec4& ca, const glm::vec4& cb,
                       const glm::vec4& cc, const glm::vec4& cd) {
    vertices.push_back({a, ca}); vertices.push_back({b, cb}); vertices.push_back({c, cc});
    vertices.push_back({a, ca}); vertices.push_back({c, cc}); vertices.push_back({d, cd});
}

void HudRenderer::rect(float x, float y, float w, float h, const glm::vec4& color) {
    quad({x, y}, {x + w, y}, {x + w, y + h}, {x, y + h}, color, color, color, color);
}

void HudRenderer::triangle(glm::vec2 a, glm::vec2 b, glm::vec2 c, const glm::vec4& color) {
    vertices.push_back({a, color});
    vertices.push_back({b, color});
    vertices.push_back({c, color});
}

void HudRenderer::glyphs(const std::string& s, float x, float y, float pixel, const glm::vec4& color) {
    float penX = x;
    for (char ch : s) {
        for (int row = 0; row < BitmapFont::GLYPH_H; ++row) {
            const char* bits = BitmapFont::glyphRow(ch, row);
            if (!bits) break;
            // Merge neighbouring lit pixels of a row into one rectangle (fewer vertices).
            int col = 0;
            while (col < BitmapFont::GLYPH_W) {
                if (bits[col] != '#') { ++col; continue; }
                int start = col;
                while (col < BitmapFont::GLYPH_W && bits[col] == '#') ++col;
                rect(penX + start * pixel, y + row * pixel, (col - start) * pixel, pixel, color);
            }
        }
        penX += BitmapFont::ADVANCE * pixel;
    }
}

void HudRenderer::text(const std::string& s, float x, float y, float pixel, const glm::vec4& color, bool shadow) {
    if (shadow) glyphs(s, x + pixel * 0.6f, y + pixel * 0.6f, pixel, glm::vec4(0.0f, 0.0f, 0.0f, 0.55f * color.a));
    glyphs(s, x, y, pixel, color);
}

void HudRenderer::vignette(float strength) {
    // A 16 x 10 grid; each corner's darkness depends on its distance from the
    // centre, so the edges darken smoothly and the middle stays clear.
    const int NX = 16, NY = 10;
    auto alphaAt = [&](float u, float v) {
        float dx = (u - 0.5f) * 2.0f, dy = (v - 0.5f) * 2.0f * 0.85f;
        float r = std::sqrt(dx * dx + dy * dy);
        float t = std::clamp((r - 0.65f) / 0.75f, 0.0f, 1.0f);
        return strength * t * t * (3.0f - 2.0f * t);
    };
    for (int j = 0; j < NY; ++j) {
        for (int i = 0; i < NX; ++i) {
            float u0 = (float)i / NX, u1 = (float)(i + 1) / NX, v0 = (float)j / NY, v1 = (float)(j + 1) / NY;
            glm::vec2 a(u0 * screenW, v0 * screenH), b(u1 * screenW, v0 * screenH), c(u1 * screenW, v1 * screenH), d(u0 * screenW, v1 * screenH);
            quad(a, b, c, d, {0, 0, 0, alphaAt(u0, v0)}, {0, 0, 0, alphaAt(u1, v0)}, {0, 0, 0, alphaAt(u1, v1)}, {0, 0, 0, alphaAt(u0, v1)});
        }
    }
}
