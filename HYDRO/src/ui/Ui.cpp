#include "Ui.h"
#include "Font.h"
#include <cmath>
#include <glad/gl.h>

bool Ui::init() {
    bool ok = shader.load("ui.vert", "ui.frag");
    fontTex = Font::createTexture();
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    const int stride = 8 * sizeof(float);       // pos2 uv2 color4
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, stride, (void*)(4 * sizeof(float)));
    return ok;
}

void Ui::begin(int w, int h) { W = w; H = h; buf.clear(); }

void Ui::vertex(float x, float y, float u, float v, const Vec4& c) {
    buf.insert(buf.end(), {x, y, u, v, c.x, c.y, c.z, c.w});
}

void Ui::rect(float x, float y, float w, float h, const Vec4& c) {
    vertex(x, y, -1, -1, c);     vertex(x + w, y, -1, -1, c); vertex(x + w, y + h, -1, -1, c);
    vertex(x, y, -1, -1, c);     vertex(x + w, y + h, -1, -1, c); vertex(x, y + h, -1, -1, c);
}

void Ui::line(float x0, float y0, float x1, float y1, float t, const Vec4& c) {
    float dx = x1 - x0, dy = y1 - y0;
    float len = std::sqrt(dx * dx + dy * dy);
    if (len < 1e-3f) return;
    float nx = -dy / len * t * 0.5f, ny = dx / len * t * 0.5f;   // perpendicular offset
    vertex(x0 + nx, y0 + ny, -1, -1, c); vertex(x1 + nx, y1 + ny, -1, -1, c); vertex(x1 - nx, y1 - ny, -1, -1, c);
    vertex(x0 + nx, y0 + ny, -1, -1, c); vertex(x1 - nx, y1 - ny, -1, -1, c); vertex(x0 - nx, y0 - ny, -1, -1, c);
}

void Ui::outline(float x, float y, float w, float h, float t, const Vec4& c) {
    rect(x, y, w, t, c);
    rect(x, y + h - t, w, t, c);
    rect(x, y, t, h, c);
    rect(x + w - t, y, t, h, c);
}

// Sci-fi corner brackets  ┌ ┐ └ ┘
void Ui::corners(float x, float y, float w, float h, float l, float t, const Vec4& c) {
    rect(x, y, l, t, c);             rect(x, y, t, l, c);
    rect(x + w - l, y, l, t, c);     rect(x + w - t, y, t, l, c);
    rect(x, y + h - t, l, t, c);     rect(x, y + h - l, t, l, c);
    rect(x + w - l, y + h - t, l, t, c); rect(x + w - t, y + h - l, t, l, c);
}

float Ui::textWidth(const std::string& s, float scale) const {
    return s.size() * Font::CELL_W * scale;
}

void Ui::text(float x, float y, const std::string& s, float scale, const Vec4& c) {
    const float texW = (float)(Font::COUNT * Font::CELL_W);
    float cw = Font::CELL_W * scale, ch = Font::CELL_H * scale;
    // start on a whole pixel, and keep the texture coordinates a little inside the
    // glyph's cell: otherwise the last pixel column can pick up the first column of
    // the NEXT glyph in the atlas (a stray bar after K, L, E ...)
    x = std::floor(x + 0.5f);
    y = std::floor(y + 0.5f);
    const float inset = 0.05f / texW;
    for (char raw : s) {
        char ch_ = (raw >= 'a' && raw <= 'z') ? (char)(raw - 32) : raw;   // only capitals are drawn
        int cell = ch_ - Font::FIRST;
        if (cell > 0 && cell < Font::COUNT) {
            float u0 = cell * Font::CELL_W / texW + inset, u1 = (cell * Font::CELL_W + Font::CELL_W) / texW - inset;
            vertex(x, y, u0, 0, c);      vertex(x + cw, y, u1, 0, c);  vertex(x + cw, y + ch, u1, 1, c);
            vertex(x, y, u0, 0, c);      vertex(x + cw, y + ch, u1, 1, c); vertex(x, y + ch, u0, 1, c);
        }
        x += cw;
    }
}

void Ui::end() {
    if (buf.empty()) return;
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    shader.use();
    shader.set("uScreen", Vec2((float)W, (float)H));
    shader.set("uFont", 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, fontTex);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, buf.size() * sizeof(float), buf.data(), GL_STREAM_DRAW);
    glDrawArrays(GL_TRIANGLES, 0, (int)(buf.size() / 8));
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    buf.clear();
}
