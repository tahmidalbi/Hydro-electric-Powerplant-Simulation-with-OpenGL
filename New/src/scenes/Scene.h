// =============================================================================
//  Scene.h  -  the common shape of every scene (= one shot of the video).
//
//  The App calls, every frame:
//      update(dt)      -> animation / simple physics
//      camera(cam)     -> the scripted "director's" camera for this moment
//      lighting(L)     -> sun, sky, fog for this scene
//      drawOpaque(r)   -> solid objects (also used for the shadow map)
//      drawTransparent(r) -> water, glass, particles (after the solid ones)
//      hud(...)        -> labels drawn on top of the picture
//  Buttons: buttons() lists them, onButton(id) reacts to a click / key.
// =============================================================================
#pragma once
#include <string>
#include <vector>
#include "../render/Renderer.h"
#include "../ui/Ui.h"

struct Button {
    std::string label;
    int id = 0;
    std::string key;          // keyboard shortcut shown on the button, e.g. "SPACE"
    bool primary = false;     // highlighted (the suggested next action)
};

struct Telemetry {
    std::string label, value;
};

class Scene {
public:
    virtual ~Scene() {}
    float t = 0;                      // seconds since this scene started

    virtual std::string title() const = 0;
    virtual std::string caption() const = 0;
    virtual void build() = 0;         // create meshes once at start-up
    virtual void enter() = 0;         // reset the story of this scene
    virtual void update(float dt) = 0;
    virtual void camera(Camera& cam) = 0;
    virtual void lighting(Lighting& L) = 0;
    virtual Vec3 shadowCentre() const { return Vec3(0, 0, 0); }
    virtual float shadowRadius() const { return 60.0f; }
    virtual void drawOpaque(Renderer& r) = 0;
    virtual void drawTransparent(Renderer& r) {}
    virtual bool rayTraced() const { return false; }
    virtual void hud(Ui& ui, const Mat4& viewProj) {}
    virtual void telemetry(std::vector<Telemetry>& out) {}
    virtual void buttons(std::vector<Button>& out) {}
    virtual void onButton(int id) {}
    virtual void autoPlay() {}                     // presses the buttons like the video does
    virtual bool finished() const { return false; }// true = the story continues to the next scene
};

// Projects a world point to window pixels. Returns false if it is behind the camera.
inline bool projectToScreen(const Mat4& viewProj, const Vec3& p, int w, int h, float& sx, float& sy) {
    Vec4 c = viewProj * Vec4(p, 1.0f);
    if (c.w <= 0.01f) return false;
    sx = (c.x / c.w * 0.5f + 0.5f) * w;
    sy = (1.0f - (c.y / c.w * 0.5f + 0.5f)) * h;
    return true;
}
