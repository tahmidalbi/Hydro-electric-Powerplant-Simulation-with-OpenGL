// =============================================================================
//  PostFX.h  -  cinematic finishing done on the whole picture after the 3D
//  scene is drawn:
//    1. BLOOM: bright parts (lamps, glowing current, sun glints) are cut out,
//       blurred twice (horizontal + vertical Gaussian) at a quarter of the
//       screen size, and added back -> soft glow halos.
//    2. VIGNETTE: the corners get slightly darker, like a film camera.
//    3. FILM GRAIN: a tiny bit of animated noise.
//  Press V to switch it off and compare.
// =============================================================================
#pragma once
#include "../core/Shader.h"

class PostFX {
public:
    bool enabled = true;
    float bloomStrength = 0.55f;

    bool init(int w, int h);
    void resize(int w, int h);
    void apply(unsigned int sceneTexture, float time);   // writes the final image to the screen

private:
    struct Target { unsigned int fbo = 0, tex = 0; };
    Target a, b;
    Shader bright, blur, composite;
    unsigned int vao = 0;
    int W = 0, H = 0, bw = 0, bh = 0;
    void create(Target& t, int w, int h);
};
