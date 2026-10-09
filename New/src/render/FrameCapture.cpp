#include "FrameCapture.h"
#include <glad/gl.h>

void FrameCapture::create(Target& t) {
    if (!t.fbo) { glGenFramebuffers(1, &t.fbo); glGenTextures(1, &t.tex); }
    glBindTexture(GL_TEXTURE_2D, t.tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, W, H, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindFramebuffer(GL_FRAMEBUFFER, t.fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, t.tex, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

bool FrameCapture::init(int w, int h) {
    bool ok = blit.load("fullscreen.vert", "blit.frag");
    glGenVertexArrays(1, &vao);
    resize(w, h);
    return ok;
}

void FrameCapture::resize(int w, int h) {
    if (w == W && h == H) return;
    W = w;
    H = h;
    create(live);
    create(frozen);
}

void FrameCapture::captureLive() {
    // resolve the anti-aliased screen into a normal texture
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, live.fbo);
    glBlitFramebuffer(0, 0, W, H, 0, 0, W, H, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FrameCapture::freeze() {
    glBindFramebuffer(GL_READ_FRAMEBUFFER, live.fbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, frozen.fbo);
    glBlitFramebuffer(0, 0, W, H, 0, 0, W, H, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FrameCapture::drawFrozen(float alpha) {
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    blit.use();
    blit.set("uTex", 0);
    blit.set("uAlpha", alpha);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, frozen.tex);
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
}
