#pragma once
#include <string>

struct App;
class HudRenderer;

// STEP 8 - What the on-screen panel shows (the HudRenderer does the drawing).
//
//   top-left     plant status: view, shading, time, wind, gate, flow, boat, solar
//   top-right    wind compass: which way the wind blows, as seen from the camera
//   bottom-left  the selected object and the keys that control it
//   bottom-right hint line
//   F11          full list of keys in the middle of the screen
//   Backspace    hides everything for a clean screenshot
namespace HudOverlay {
    void draw(HudRenderer& hud, const App& app, const std::string& galleryDetail, float fps);
}
