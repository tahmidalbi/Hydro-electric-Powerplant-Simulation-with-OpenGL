#pragma once
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include "../core/Shader.h"

// Draws flat 2D shapes and text on top of the 3D picture, in screen pixels.
//
// Usage per frame:  begin(width, height) -> rect/triangle/text/... -> end()
// Everything between begin and end is collected into one vertex list and
// drawn with ONE draw call at end() (blending on, depth test off).
class HudRenderer {
public:
    HudRenderer();
    ~HudRenderer();
    HudRenderer(const HudRenderer&) = delete;
    HudRenderer& operator=(const HudRenderer&) = delete;

    void begin(int width, int height);
    void end();

    void rect(float x, float y, float w, float h, const glm::vec4& color);
    void triangle(glm::vec2 a, glm::vec2 b, glm::vec2 c, const glm::vec4& color);
    // Text with the 5x7 bitmap font. `pixel` = screen pixels per font pixel.
    // A dark drop shadow keeps it readable on bright sky or water.
    void text(const std::string& s, float x, float y, float pixel, const glm::vec4& color, bool shadow = true);
    // Darkens the screen edges a little (a "vignette", like a camera lens).
    void vignette(float strength);

    int width() const { return screenW; }
    int height() const { return screenH; }

private:
    struct Vertex { glm::vec2 pixel; glm::vec4 color; };
    void quad(glm::vec2 a, glm::vec2 b, glm::vec2 c, glm::vec2 d, const glm::vec4& ca, const glm::vec4& cb,
              const glm::vec4& cc, const glm::vec4& cd);
    void glyphs(const std::string& s, float x, float y, float pixel, const glm::vec4& color);

    Shader shader;
    unsigned int vao = 0, vbo = 0;
    std::vector<Vertex> vertices;
    int screenW = 1, screenH = 1;
};
