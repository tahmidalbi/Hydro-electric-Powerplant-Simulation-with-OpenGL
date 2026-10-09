// =============================================================================
//  Scene 1  (video 0:00 - 0:10)  -  SPILLWAY RELEASE
//  Water falls from the reservoir down the three spillway chutes, sprays into
//  the river; the camera follows it down to the cut-away powerhouse where the
//  turbine and generator spin.
//  Buttons: STOP / START WATER  and  FOLLOW THE WATER (next scene).
// =============================================================================
#pragma once
#include "Scene.h"
#include "../objects/DamSite.h"

class Scene1_Spillway : public Scene {
public:
    explicit Scene1_Spillway(DamSite& s) : site(s) {}
    std::string title() const override { return "SPILLWAY RELEASE"; }
    std::string caption() const override {
        return site.gatesOpen ? "RESERVOIR WATER FALLS 50 M DOWN THE SPILLWAY - GRAVITY TURNS HEIGHT INTO SPEED"
                              : "GATES CLOSED - THE CHUTES DRAIN.  NOW SEND THE WATER TO THE POWER INTAKE";
    }
    void build() override {}
    void enter() override;
    void update(float dt) override;
    void camera(Camera& cam) override;
    void lighting(Lighting& L) override;
    Vec3 shadowCentre() const override { return Vec3(25, 25, 40); }
    float shadowRadius() const override { return 130.0f; }
    void drawOpaque(Renderer& r) override { site.drawOpaque(r); }
    void drawTransparent(Renderer& r) override { site.drawTransparent(r); }
    void telemetry(std::vector<Telemetry>& out) override;
    void buttons(std::vector<Button>& out) override;
    void onButton(int id) override;
    void autoPlay() override { if (t > 10.0f) next = true; }
    bool finished() const override { return next; }

    enum { BTN_TOGGLE = 1, BTN_NEXT = 2 };

private:
    DamSite& site;
    bool next = false;
};
