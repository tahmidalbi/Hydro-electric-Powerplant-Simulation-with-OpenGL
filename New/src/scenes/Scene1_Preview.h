// =============================================================================
//  Scene 1  (video 0:00 - 0:10)  -  PREVIEW: THE FINISHED PLANT
//  The power plant running: water falls down the spillway, the cut-away
//  powerhouse shows the turbine and generator spinning.  The camera follows
//  the falling water down to the powerhouse.
//  Buttons: STOP / START WATER, SHOW WATER MOVEMENT (starts the journey).
// =============================================================================
#pragma once
#include "Scene.h"
#include "../objects/World.h"

class Scene1_Preview : public Scene {
public:
    explicit Scene1_Preview(World& w) : world(w) {}
    std::string title() const override { return "PREVIEW - THE FINISHED PLANT"; }
    std::string caption() const override {
        return world.spillOpen ? "THE RUNNING PLANT: WATER FALLS 50 M DOWN THE SPILLWAY AND SPINS THE TURBINE"
                               : "WATER STOPPED - GATES CLOSED, THE CHUTES AND THE PENSTOCK DRAIN";
    }
    void enter(bool continuous) override;
    void update(float dt) override;
    void camera(Camera& cam) override;
    void lighting(Lighting& L, const Vec3& camPos) override { world.lighting(L, camPos); }
    Vec3 shadowCentre() const override { return Vec3(25, 25, 40); }
    float shadowRadius() const override { return 130.0f; }
    void drawOpaque(Renderer& r) override { world.drawOpaque(r); }
    void drawTransparent(Renderer& r) override { world.drawTransparent(r); }
    void telemetry(std::vector<Telemetry>& out) override;
    void buttons(std::vector<Button>& out) override;
    void onButton(int id) override;
    void autoPlay() override { if (t > 10.0f) next = true; }
    bool finished() const override { return next; }

    enum { BTN_TOGGLE = 1, BTN_SHOW = 2 };

private:
    World& world;
    bool next = false;
};
