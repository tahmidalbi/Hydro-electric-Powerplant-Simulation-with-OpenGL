// =============================================================================
//  Scene 6  (video 1:10 - 1:20)  -  MAIN INLET VALVE
//  Back in the powerhouse pit: the full penstock ends at the closed main
//  inlet valve.  OPEN VALVE: the hand-wheel turns, the heavy gate rises into
//  the bonnet and the water rushes into the spiral casing.
//  FOLLOW TO TURBINE is enabled once the casing is full.
// =============================================================================
#pragma once
#include "Scene.h"
#include "../objects/World.h"

class Scene6_InletValve : public Scene {
public:
    explicit Scene6_InletValve(World& w) : world(w) {}
    std::string title() const override { return "MAIN INLET VALVE"; }
    std::string caption() const override {
        return world.valveOpen ? "THE VALVE GATE RISES - WATER RUSHES INTO THE SPIRAL CASING OF THE TURBINE"
                               : "THE PENSTOCK IS FULL. THE CLOSED MAIN INLET VALVE HOLDS 5.9 BAR OF WATER BACK";
    }
    void enter(bool continuous) override;
    void update(float dt) override { t += dt; world.update(dt); }
    void camera(Camera& cam) override;
    void lighting(Lighting& L, const Vec3& camPos) override { world.lighting(L, camPos); }
    Vec3 shadowCentre() const override { return Vec3(60, 8, 62); }
    float shadowRadius() const override { return 28.0f; }
    void drawOpaque(Renderer& r) override { world.drawOpaque(r); }
    void drawTransparent(Renderer& r) override { world.drawTransparent(r); }
    bool dissolveOnEnter() const override { return true; }   // we come from the ray-traced view
    void hud(Ui& ui, const Mat4& viewProj) override;
    void telemetry(std::vector<Telemetry>& out) override;
    void buttons(std::vector<Button>& out) override;
    void onButton(int id) override;
    void autoPlay() override;
    bool finished() const override { return next; }

private:
    World& world;
    bool next = false;
    bool ready() const { return world.valveOpen && world.casingFull(); }
};
