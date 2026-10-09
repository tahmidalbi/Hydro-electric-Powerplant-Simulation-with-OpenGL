// =============================================================================
//  Scene 6  (video 0:50 - 1:00)  -  THE PENSTOCK
//  The water leaves the intake and rushes down the steep steel pipe.  Speed
//  follows free fall: v = sqrt(2 g h), so the front accelerates as it drops.
//  Half way down the pipe switches to an "x-ray" hologram view, and the water
//  reaches the turbine's spiral casing at the bottom.
// =============================================================================
#pragma once
#include "Scene.h"
#include "../objects/CutawayPipe.h"

class Scene6_Penstock : public Scene {
public:
    std::string title() const override { return "PENSTOCK"; }
    std::string caption() const override { return "GRAVITY PULLS THE WATER 34 M DOWN THE PIPE - IT GETS FASTER AND FASTER (V = SQRT(2GH))"; }
    void build() override;
    void enter() override;
    void update(float dt) override;
    void camera(Camera& cam) override;
    void lighting(Lighting& L) override;
    Vec3 shadowCentre() const override { return Vec3(4, 15, -4); }
    float shadowRadius() const override { return 75.0f; }
    void drawOpaque(Renderer& r) override;
    void drawTransparent(Renderer& r) override;
    void hud(Ui& ui, const Mat4& viewProj) override;
    void telemetry(std::vector<Telemetry>& out) override;
    void buttons(std::vector<Button>& out) override;
    void onButton(int id) override;
    bool finished() const override { return next || arrivedTime > 2.5f; }

private:
    CutawayPipe pipe;
    Mesh damFace, buttresses, ground, intakeBlock, saddles, casing, casingRibs, walkways, rails, reservoir, machine;
    float front = 0;
    float xray = 0;
    bool xrayManual = false, xrayOn = false;
    float arrivedTime = 0;
    bool next = false;
    float speedAt(float s) const;
    static constexpr float SLOW_MOTION = 0.6f;
};
