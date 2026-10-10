// =============================================================================
//  Scene 3  (video 0:40 - 0:50)  -  INTAKE GATE
//  The camera glides on from the trash rack while the dam section in front of
//  the conduit lifts away (cut-away).  Inside: the closed steel gate holds the
//  reservoir back.  OPEN GATE: the hydraulic cylinders on the crest lift it,
//  water bursts out underneath and fills the conduit up to the penstock.
//  FOLLOW TO PENSTOCK is only enabled once the gate is fully open.
// =============================================================================
#pragma once
#include "Scene.h"
#include "../objects/World.h"

class Scene3_IntakeGate : public Scene {
public:
    explicit Scene3_IntakeGate(World& w) : world(w) {}
    std::string title() const override { return "INTAKE GATE"; }
    std::string caption() const override {
        return world.intakeOpen ? "THE GATE RISES - RESERVOIR PRESSURE PUSHES THE WATER THROUGH THE CONDUIT"
                                : "THE CLOSED GATE HOLDS BACK THE RESERVOIR.  PRESS OPEN GATE";
    }
    void enter(bool continuous) override;
    void update(float dt) override;
    void camera(Camera& cam) override;
    void lighting(Lighting& L, const Vec3& camPos) override { world.lighting(L, camPos); }
    Vec3 shadowCentre() const override { return Vec3(58, 42, 3); }
    float shadowRadius() const override { return 40.0f; }
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
    float openedAt = -1.0f;
};
