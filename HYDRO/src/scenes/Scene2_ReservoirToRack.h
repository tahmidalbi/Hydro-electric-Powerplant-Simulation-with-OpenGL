// =============================================================================
//  Scene 2  (video 0:20 - 0:40)  -  RESERVOIR -> TRASH RACK
//  Part A (automatic): the camera flies on from the preview, over the dam
//  crest, out over the reservoir and dives under the surface to the intake:
//  light shafts, caustics, floating dust, the steel trash-rack cage.
//  Part B: a cut-away side view of the same rack.  Branches, logs, plastic
//  and straw drift in with the water and stop at the bars; small leaves slip
//  through into the intake.
//  Buttons (part B): ADD MORE TRASH, FOLLOW TO INTAKE GATE.
// =============================================================================
#pragma once
#include "Scene.h"
#include "../objects/World.h"

class Scene2_ReservoirToRack : public Scene {
public:
    explicit Scene2_ReservoirToRack(World& w) : world(w) {}
    std::string title() const override { return inRack() ? "TRASH RACK" : "INTO THE RESERVOIR"; }
    std::string caption() const override {
        return inRack() ? "BRANCHES, LOGS AND PLASTIC STOP AT THE BARS - ONLY CLEAN WATER ENTERS THE INTAKE"
                        : "FOLLOWING THE WATER: FROM THE RESERVOIR DOWN TO THE INTAKE AT THE FOOT OF THE CRANE";
    }
    void enter(bool continuous) override;
    void update(float dt) override;
    void camera(Camera& cam) override;
    void lighting(Lighting& L, const Vec3& camPos) override { world.lighting(L, camPos); }
    Vec3 shadowCentre() const override { return inRack() ? Vec3(58, 45, -12) : Vec3(55, 40, -20); }
    float shadowRadius() const override { return inRack() ? 45.0f : 70.0f; }
    void drawOpaque(Renderer& r) override { world.drawOpaque(r); }
    void drawTransparent(Renderer& r) override { world.drawTransparent(r); }
    void hud(Ui& ui, const Mat4& viewProj) override;
    void telemetry(std::vector<Telemetry>& out) override;
    void buttons(std::vector<Button>& out) override;
    void onButton(int id) override;
    void autoPlay() override { if (inRack() && t - APPROACH > 13.0f) next = true; }
    bool finished() const override { return next; }

private:
    World& world;
    bool next = false;
    Vec3 camEye;
    static constexpr float APPROACH = 13.0f;   // seconds of the automatic flight
    bool inRack() const { return t >= APPROACH; }
};
