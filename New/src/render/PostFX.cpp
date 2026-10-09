#include "PostFX.h"
#include <glad/gl.h>

void PostFX::create(Target& t, int w, int h) {
    if (!t.fbo) { glGenFramebuffers(1, &t.fbo); glGenTextures(1, &t.tex); }
    glBindTexture(GL_TEXTURE_2D, t.tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, w, h, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindFramebuffer(GL_FRAMEBUFFER, t.fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, t.tex, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

bool PostFX::init(int w, int h) {
    bool ok = bright.load("fullscreen.vert", "post_bright.frag");
    ok &= blur.load("fullscreen.vert", "post_blur.frag");
    ok &= composite.load("fullscreen.vert", "post_composite.frag");
    glGenVertexArrays(1, &vao);
    resize(w, h);
    return ok;
}

void PostFX::resize(int w, int h) {
    if (w == W && h == H) return;
    W = w;
    H = h;
    bw = w / 4 > 1 ? w / 4 : 1;
    bh = h / 4 > 1 ? h / 4 : 1;
    create(a, bw, bh);
    create(b, bw, bh);
}

void PostFX::apply(unsigned int sceneTex, float time) {
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glBindVertexArray(vao);
    glActiveTexture(GL_TEXTURE0);

    // 1) bright pass into a quarter-size texture
    glBindFramebuffer(GL_FRAMEBUFFER, a.fbo);
    glViewport(0, 0, bw, bh);
    bright.use();
    bright.set("uTex", 0);
    glBindTexture(GL_TEXTURE_2D, sceneTex);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    // 2) blur: horizontal then vertical, twice with a growing radius
    blur.use();
    blur.set("uTex", 0);
    for (int pass = 0; pass < 2; pass++) {
        float r = 1.0f + pass * 1.5f;
        glBindFramebuffer(GL_FRAMEBUFFER, b.fbo);
        blur.set("uDir", Vec2(r / bw, 0.0f));
        glBindTexture(GL_TEXTURE_2D, a.tex);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glBindFramebuffer(GL_FRAMEBUFFER, a.fbo);
        blur.set("uDir", Vec2(0.0f, r / bh));
        glBindTexture(GL_TEXTURE_2D, b.tex);
        glDrawArrays(GL_TRIANGLES, 0, 3);
    }

    // 3) final picture = scene + bloom, then vignette and grain
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, W, H);
    composite.use();
    composite.set("uScene", 0);
    composite.set("uBloom", 1);
    composite.set("uStrength", bloomStrength);
    composite.set("uTime", time);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, a.tex);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, sceneTex);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glEnable(GL_DEPTH_TEST);
}
