// =============================================================================
//  Scene.h  -  the common shape of every scene (= one station of the story).
//
//  All scenes look at the SAME World, so the water travels from scene to
//  scene.  When the story moves on, the next scene starts its camera exactly
//  where the previous one stopped (fromEye / fromTarget) -> one continuous
//  flight, no cut.
//
//  The App calls, every frame:
//      update(dt)         -> animation / simple physics
//      camera(cam)        -> the scripted "director's" camera for this moment
//      lighting(L)        -> sun, sky, fog for this moment
//      drawOpaque(r)      -> solid objects (also used for the shadow map)
//      drawTransparent(r) -> water, glass, particles (after the solid ones)
//      hud(...)           -> labels drawn on top of the picture
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
    bool enabled = true;      // disabled buttons are grey and cannot be pressed
    std::string hint;         // shown under a disabled button: what to do first
};

struct Telemetry {
    std::string label, value;
};

class Scene {
public:
    virtual ~Scene() {}
    float t = 0;                      // seconds since this scene started (scaled by the speed control)
    Vec3 fromEye{0, 50, 100}, fromTarget{0, 30, 0};   // camera at the moment we arrived
    bool dissolveRequest = false;     // the scene asks the App for a cross-dissolve now

    virtual std::string title() const = 0;
    virtual std::string caption() const = 0;
    virtual void build() {}
    // continuous = true: we arrived by following the story (camera flies on from fromEye)
    // continuous = false: we jumped here with a number key (set everything up directly)
    virtual void enter(bool continuous) = 0;
    virtual void update(float dt) = 0;
    virtual void camera(Camera& cam) = 0;
    virtual void lighting(Lighting& L, const Vec3& camPos) = 0;
    virtual Vec3 shadowCentre() const { return Vec3(0, 0, 0); }
    virtual float shadowRadius() const { return 60.0f; }
    virtual void drawOpaque(Renderer& r) = 0;
    virtual void drawTransparent(Renderer& r) {}
    virtual bool rayTraced() const { return false; }
    virtual bool dissolveOnEnter() const { return false; }   // cross-fade into this scene
    virtual void hud(Ui& ui, const Mat4& viewProj) {}
    virtual void telemetry(std::vector<Telemetry>& out) {}
    virtual void buttons(std::vector<Button>& out) {}
    virtual void onButton(int id) {}
    virtual void autoPlay() {}                     // presses the buttons by itself (AUTO mode)
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

// A small label with a leader line pointing at a 3D point (used by several scenes).
inline void pointerLabel(Ui& ui, const Mat4& vp, const Vec3& p, float dx, float dy, const std::string& txt) {
    const Vec4 cyan(0.45f, 0.88f, 1.0f, 0.9f), white(1, 1, 1, 0.95f);
    float x, y;
    if (!projectToScreen(vp, p, ui.W, ui.H, x, y)) return;
    if (x < 0 || x > ui.W || y < 30 || y > ui.H - 30) return;
    float w = ui.textWidth(txt, 1.5f) + 14;
    float lx = dx > 0 ? x + dx : x + dx - w;
    lx = clampf(lx, 10.0f, ui.W - w - 10.0f);
    float ly = y + dy;
    // keep the label out of the HUD panels: title (top left) and telemetry + speed chips (top right)
    auto hits = [&](float x0, float x1, float yBottom) { return lx < x1 && lx + w > x0 && ly - 12 < yBottom; };
    if (hits(ui.W - 385.0f, ui.W, 248.0f)) ly = 248.0f + 16.0f;
    if (hits(0.0f, 740.0f, 136.0f)) ly = 136.0f + 16.0f;
    ui.rect(x - 4, y - 4, 8, 8, cyan);
    ui.line(x, y, dx > 0 ? lx : lx + w, ly, 1.5f, cyan);
    ui.rect(lx, ly - 12, w, 22, Vec4(0.02f, 0.06f, 0.1f, 0.75f));
    ui.text(lx + 7, ly - 6, txt, 1.5f, white);
}
