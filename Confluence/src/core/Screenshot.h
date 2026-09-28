#pragma once
#include <functional>
#include <string>

// Renders one frame into an offscreen 4x-MSAA framebuffer of the given size
// and writes it to a 24-bit BMP. `render(w, h)` must draw the whole frame.
bool captureToBmp(int width, int height, const std::function<void(int, int)>& render, const std::string& path);
