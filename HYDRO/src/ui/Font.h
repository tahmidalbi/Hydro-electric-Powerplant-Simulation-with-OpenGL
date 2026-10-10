// =============================================================================
//  Font.h  -  a hand-made 5x7 pixel font (no font files, no libraries).
//  The glyphs are drawn as text pictures in Font.cpp and packed into one
//  small OpenGL texture at start-up.
// =============================================================================
#pragma once

namespace Font {
    const int CELL_W = 6;          // 5 pixels + 1 pixel gap
    const int CELL_H = 8;          // 7 pixels + 1 pixel gap
    const int FIRST = 32;          // ' '
    const int COUNT = 96;          // ASCII 32..127
    unsigned int createTexture();  // returns an OpenGL texture id
}
