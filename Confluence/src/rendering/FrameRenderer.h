#pragma once
#include <string>
#include "ParticleRenderer.h"
#include "../core/MeshLibrary.h"
#include "../core/Shader.h"
#include "../objects/Scene.h"
#include "../objects/Sky.h"
#include "../ui/DemoGallery.h"
#include "../ui/HudRenderer.h"

struct App;

// Draws one frame. Owns everything that lives on the GPU: the shader
// programs, the shared primitive meshes, the plant scene, the gallery, the
// particle and HUD renderers.
//
// Order of a frame:
//   sky -> opaque scene (+ its transparent falls) -> lamp bulb
//       -> particles (soft, sorted far to near) -> HUD on top
//
// It only READS the App -- the render step never changes the simulation.
class FrameRenderer {
public:
    FrameRenderer();   // needs a current OpenGL context

    // realTimeSeconds: wall-clock time, used only for the selection glow's pulse.
    void render(int width, int height, const App& app, float fps, float realTimeSeconds);

    std::string galleryDetailSummary(const App& app) const;

private:
    // Two lighting programs, one shared lighting formula (lighting_common.glsl):
    Shader smoothShader;   // phong.vert + phong.frag            -> Gouraud and Phong
    Shader flatShader;     // flat.vert + flat.geom + flat.frag  -> strict Flat
    MeshLibrary meshes;
    Scene scene;
    Sky sky;
    DemoGallery gallery;
    ParticleRenderer particleRenderer;
    HudRenderer hud;
};
