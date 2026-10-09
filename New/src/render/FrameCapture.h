// =============================================================================
//  FrameCapture.h  -  keeps a copy of the last rendered 3D picture so a scene
//  change can CROSS-DISSOLVE: the old picture is drawn on top of the new one
//  and slowly made transparent.
//
//  Every frame the (multisampled) screen is resolved into 'live'.  When a
//  dissolve starts, 'live' is copied into 'frozen', which is then faded out.
// =============================================================================
#pragma once
#include "../core/Shader.h"

class FrameCapture {
public:
    bool init(int w, int h);
    void resize(int w, int h);
    void captureLive();             // call after the 3D scene is drawn, before the HUD
    void freeze();                  // start a dissolve from the last captured picture
    void drawFrozen(float alpha);   // draw the frozen picture on top
    unsigned int liveTexture() const { return live.tex; }
private:
    struct Target { unsigned int fbo = 0, tex = 0; };
    Target live, frozen;
    Shader blit;
    unsigned int vao = 0;
    int W = 0, H = 0;
    void create(Target& t);
};
