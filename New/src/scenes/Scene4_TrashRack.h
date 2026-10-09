// =============================================================================
//  Scene 4  (video 0:30 - 0:40)  -  TRASH RACK FILTRATION (cut-away view)
//  A cut through the intake channel: water flows left to right through a
//  cage of steel bars.  A drift-wood branch, logs, plastic and straw hit the
//  bars and stay there; only small leaves slip through.
//  Buttons: THROW MORE TRASH, RAKE CLEAN, GO TO INTAKE GATE.
// =============================================================================
#pragma once
#include "Scene.h"
#include "../objects/TrashRack.h"
#include "../objects/Debris.h"

class Scene4_TrashRack : public Scene {
public:
    std::string title() const override { return "TRASH RACK FILTRATION"; }
    std::string caption() const override { return "BRANCHES, LOGS AND PLASTIC ARE STOPPED BY THE BARS - ONLY CLEAN WATER GOES ON TO THE TURBINE"; }
    void build() override;
    void enter() override;
    void update(float dt) override;
    void camera(Camera& cam) override;
    void lighting(Lighting& L) override;
    Vec3 shadowCentre() const override { return Vec3(0, 4, 0); }
    float shadowRadius() const override { return 30.0f; }
    void drawOpaque(Renderer& r) override;
    void drawTransparent(Renderer& r) override;
    void hud(Ui& ui, const Mat4& viewProj) override;
    void telemetry(std::vector<Telemetry>& out) override;
    void buttons(std::vector<Button>& out) override;
    void onButton(int id) override;
    void autoPlay() override { if (t > 13.0f) next = true; }
    bool finished() const override { return next; }

private:
    Mesh floorSlab, soil, backWall, upper, motors, motorShafts, railing, waterTop, waterFront;
    TrashRack cage;
    Debris debris;
    ParticleSystem splash;
    int wave = 0;
    float nextWaveAt = 0;
    bool next = false;
    static constexpr float WATER_Y = 6.5f;
};
