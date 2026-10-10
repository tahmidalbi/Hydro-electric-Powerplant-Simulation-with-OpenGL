// =============================================================================
//  Scene 4  (video 0:50 - 1:00)  -  PENSTOCK
//  The water leaves the conduit and rushes down the steel penstock on the
//  dam face.  Its speed follows free fall: v = sqrt(v0^2 + 2 g h), so it
//  accelerates as it drops.  Half way down the pipe switches to the x-ray
//  hologram view.  At the bottom, inside the powerhouse, it is stopped by
//  the closed MAIN INLET VALVE in front of the turbine.
//  Buttons: X-RAY VIEW, REPLAY FLOW, FOLLOW TO RAY-TRACED PIPE.
// =============================================================================
#pragma once
#include "Scene.h"
#include "../objects/World.h"

class Scene4_Penstock : public Scene {
public:
    explicit Scene4_Penstock(World& w) : world(w) {}
    std::string title() const override { return "PENSTOCK"; }
    std::string caption() const override { return "GRAVITY PULLS THE WATER 33 M DOWN THE PIPE - FASTER AND FASTER (V = SQRT(2GH))"; }
    void enter(bool continuous) override;
    void update(float dt) override;
    void camera(Camera& cam) override;
    void lighting(Lighting& L, const Vec3& camPos) override { world.lighting(L, camPos); }
    Vec3 shadowCentre() const override { return Vec3(55, 22, 40); }
    float shadowRadius() const override { return 60.0f; }
    void drawOpaque(Renderer& r) override { world.drawOpaque(r); }
    void drawTransparent(Renderer& r) override { world.drawTransparent(r); }
    void hud(Ui& ui, const Mat4& viewProj) override;
    void telemetry(std::vector<Telemetry>& out) override;
    void buttons(std::vector<Button>& out) override;
    void onButton(int id) override;
    void autoPlay() override { if (arrivedTime > 3.0f) onButton(3); }
    bool finished() const override { return leaving && leaveT > 1.8f; }

private:
    World& world;
    bool xrayManual = false;
    float arrivedTime = 0;
    bool leaving = false;
    float leaveT = 0;
    Vec3 lastEye, lastTarget;
};
