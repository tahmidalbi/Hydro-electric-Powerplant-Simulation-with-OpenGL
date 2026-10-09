// =============================================================================
//  Scene 7  (video 1:20 - 1:50)  -  FRANCIS TURBINE
//  Top view: water swirls round the spiral casing, through the fixed stay
//  vanes and the 24 movable guide vanes, into the runner.  OPEN GUIDE VANES:
//  the vanes turn open, the water pushes the 15 curved runner blades and the
//  runner speeds up to 150 rpm.  SECTION VIEW cuts the casing in half and
//  shows the streamlines going through the runner and down the draft tube.
//  FOLLOW UP THE SHAFT is enabled once the runner is at full speed.
// =============================================================================
#pragma once
#include "Scene.h"
#include "../objects/World.h"

class Scene7_Turbine : public Scene {
public:
    explicit Scene7_Turbine(World& w) : world(w) {}
    std::string title() const override { return "FRANCIS TURBINE"; }
    std::string caption() const override {
        return world.vanesOpen ? "THE GUIDE VANES OPEN - WATER PUSHES THE CURVED BLADES AND THE RUNNER TURNS"
                               : "WATER FILLS THE SPIRAL CASING. THE GUIDE VANES ARE STILL CLOSED";
    }
    void enter(bool continuous) override;
    void update(float dt) override;
    void camera(Camera& cam) override;
    void lighting(Lighting& L, const Vec3& camPos) override { world.lighting(L, camPos); }
    Vec3 shadowCentre() const override { return Vec3(62, 7, 64); }
    float shadowRadius() const override { return 18.0f; }
    void drawOpaque(Renderer& r) override { world.drawOpaque(r); }
    void drawTransparent(Renderer& r) override { world.drawTransparent(r); }
    void hud(Ui& ui, const Mat4& viewProj) override;
    void telemetry(std::vector<Telemetry>& out) override;
    void buttons(std::vector<Button>& out) override;
    void onButton(int id) override;
    void autoPlay() override;
    bool finished() const override { return next; }

private:
    World& world;
    bool next = false;
    float section = 0;          // animated 0 = top view, 1 = section view
    bool ready() const { return world.rpm() >= 140.0f; }
};
