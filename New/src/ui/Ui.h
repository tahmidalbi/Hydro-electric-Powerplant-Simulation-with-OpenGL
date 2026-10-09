// =============================================================================
//  Ui.h  -  draws 2D things on top of the 3D picture: rectangles, lines,
//  text.  Coordinates are pixels, (0,0) = top-left corner of the window.
//  Everything is collected in one vertex list and drawn with a single call.
// =============================================================================
#pragma once
#include <string>
#include <vector>
#include "../core/Shader.h"

class Ui {
public:
    int W = 1280, H = 720;

    bool init();
    void begin(int w, int h);
    void end();                                     // sends everything to the GPU

    void rect(float x, float y, float w, float h, const Vec4& color);
    void line(float x0, float y0, float x1, float y1, float thick, const Vec4& color);
    void outline(float x, float y, float w, float h, float thick, const Vec4& color);
    void corners(float x, float y, float w, float h, float len, float thick, const Vec4& color);
    void text(float x, float y, const std::string& s, float scale, const Vec4& color);
    float textWidth(const std::string& s, float scale) const;

private:
    Shader shader;
    unsigned int vao = 0, vbo = 0, fontTex = 0;
    std::vector<float> buf;
    void vertex(float x, float y, float u, float v, const Vec4& c);
};
