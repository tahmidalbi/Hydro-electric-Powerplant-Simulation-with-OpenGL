#pragma once
#include <string>

// A tiny 5 x 7 pixel font, drawn by hand in BitmapFont.cpp as ASCII art
// ('#' = pixel on). No font library, no texture file: each lit pixel becomes
// a small rectangle in the HUD's vertex buffer.
//
// Supports A-Z (lower case is shown as upper case), 0-9, space and
// . , : ; % / - + ( ) [ ] < > = ! ? ' _ * | # ^ ~ (~ draws a degree sign).
namespace BitmapFont {
    constexpr int GLYPH_W = 5, GLYPH_H = 7;
    constexpr int ADVANCE = 6;        // glyph width + 1 pixel gap
    constexpr int LINE_HEIGHT = 10;   // glyph height + 3 pixels gap

    // Row `row` (0 = top) of character `c`: 5 chars of '#'/'.'; nullptr if unknown.
    const char* glyphRow(char c, int row);

    // Width in font pixels of a single line of text.
    inline int textWidth(const std::string& text) { return (int)text.size() * ADVANCE; }
}
