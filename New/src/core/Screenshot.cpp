#include "Screenshot.h"
#include <glad/gl.h>
#include <vector>
#include <cstdio>
#include <cstdint>

static void put16(std::vector<uint8_t>& v, uint16_t x) { v.push_back(x & 255); v.push_back(x >> 8); }
static void put32(std::vector<uint8_t>& v, uint32_t x) { for (int i = 0; i < 4; i++) v.push_back((x >> (8 * i)) & 255); }

bool saveScreenshotBMP(const std::string& file, int w, int h) {
    int rowSize = (w * 3 + 3) & ~3;                    // BMP rows are padded to 4 bytes
    std::vector<uint8_t> pixels(rowSize * h);
    glPixelStorei(GL_PACK_ALIGNMENT, 4);
    glReadBuffer(GL_BACK);
    glReadPixels(0, 0, w, h, GL_BGR, GL_UNSIGNED_BYTE, pixels.data());   // bottom row first = BMP order

    std::vector<uint8_t> hdr;
    hdr.push_back('B'); hdr.push_back('M');
    put32(hdr, 54 + (uint32_t)pixels.size());   // file size
    put32(hdr, 0);
    put32(hdr, 54);                             // pixel data offset
    put32(hdr, 40);                             // info header size
    put32(hdr, (uint32_t)w);
    put32(hdr, (uint32_t)h);
    put16(hdr, 1);                              // planes
    put16(hdr, 24);                             // bits per pixel
    put32(hdr, 0);                              // no compression
    put32(hdr, (uint32_t)pixels.size());
    put32(hdr, 2835); put32(hdr, 2835);         // 72 dpi
    put32(hdr, 0); put32(hdr, 0);

    FILE* f = std::fopen(file.c_str(), "wb");
    if (!f) return false;
    std::fwrite(hdr.data(), 1, hdr.size(), f);
    std::fwrite(pixels.data(), 1, pixels.size(), f);
    std::fclose(f);
    return true;
}
