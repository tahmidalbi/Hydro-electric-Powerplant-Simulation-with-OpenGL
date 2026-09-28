#include "Screenshot.h"
#include <glad/gl.h>
#include <cstdint>
#include <cstdio>
#include <iostream>
#include <vector>

namespace {

bool writeBmp(const std::string& path, int w, int h, const std::vector<unsigned char>& bgr) {
    const int rowSize = (w * 3 + 3) & ~3;
    const uint32_t dataSize = (uint32_t)rowSize * (uint32_t)h;
    unsigned char header[54] = {0};
    auto put32 = [&](int off, uint32_t v) { for (int i = 0; i < 4; ++i) header[off + i] = (v >> (8 * i)) & 0xFF; };
    auto put16 = [&](int off, uint16_t v) { header[off] = v & 0xFF; header[off + 1] = (v >> 8) & 0xFF; };
    header[0] = 'B'; header[1] = 'M';
    put32(2, 54 + dataSize);
    put32(10, 54);
    put32(14, 40);
    put32(18, (uint32_t)w);
    put32(22, (uint32_t)h);   // positive height: bottom-up rows, same order glReadPixels returns
    put16(26, 1);
    put16(28, 24);
    put32(34, dataSize);

    FILE* f = std::fopen(path.c_str(), "wb");
    if (!f) return false;
    std::fwrite(header, 1, sizeof(header), f);
    std::vector<unsigned char> pad((size_t)(rowSize - w * 3), 0);
    for (int y = 0; y < h; ++y) {
        std::fwrite(&bgr[(size_t)y * w * 3], 1, (size_t)w * 3, f);
        if (!pad.empty()) std::fwrite(pad.data(), 1, pad.size(), f);
    }
    std::fclose(f);
    return true;
}

}

bool captureToBmp(int w, int h, const std::function<void(int, int)>& render, const std::string& path) {
    GLuint fbo[2], rbo[3];
    glGenFramebuffers(2, fbo);
    glGenRenderbuffers(3, rbo);

    glBindRenderbuffer(GL_RENDERBUFFER, rbo[0]);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_RGBA8, w, h);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo[1]);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH_COMPONENT24, w, h);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo[0]);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rbo[0]);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo[1]);

    bool ok = glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
    if (ok) {
        render(w, h);

        // Resolve the multisampled image into a plain renderbuffer, then read it back.
        glBindRenderbuffer(GL_RENDERBUFFER, rbo[2]);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, w, h);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo[1]);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rbo[2]);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo[0]);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo[1]);
        glBlitFramebuffer(0, 0, w, h, 0, 0, w, h, GL_COLOR_BUFFER_BIT, GL_NEAREST);

        glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo[1]);
        std::vector<unsigned char> pixels((size_t)w * h * 3);
        glPixelStorei(GL_PACK_ALIGNMENT, 1);
        glReadPixels(0, 0, w, h, GL_BGR, GL_UNSIGNED_BYTE, pixels.data());
        ok = writeBmp(path, w, h, pixels);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteRenderbuffers(3, rbo);
    glDeleteFramebuffers(2, fbo);
    if (!ok) std::cerr << "Screenshot failed: " << path << std::endl;
    return ok;
}
