// =============================================================================
//  Scene 8  (video 1:50 - 2:00)  -  THE SHAFT  (automatic)
//  A "blueprint hologram" shot: the camera rises along the turning steel
//  shaft from the runner, past the guide bearing and the coupling flange,
//  through the generator floor, to the generator rotor.  Scanning rings and
//  labels show how the mechanical energy is carried upwards.
// =============================================================================
#pragma once
#include "Scene.h"
#include "../objects/World.h"

class Scene8_Shaft : public Scene {
public:
    explicit Scene8_Shaft(World& w) : world(w) {}
    std::string title() const override { return "THE SHAFT"; }
    std::string caption() const override { return "THE SPINNING SHAFT CARRIES 86 MW OF MECHANICAL ENERGY UP TO THE GENERATOR"; }
    void build() override;
    void enter(bool continuous) override;
    void update(float dt) override { t += dt; world.update(dt); }
    void camera(Camera& cam) override;
    void lighting(Lighting& L, const Vec3& camPos) override;
    Vec3 shadowCentre() const override { return Vec3(62, 11, 64); }
    float shadowRadius() const override { return 18.0f; }
    void drawOpaque(Renderer& r) override { world.drawOpaque(r); }
    void drawTransparent(Renderer& r) override;
    void hud(Ui& ui, const Mat4& viewProj) override;
    void telemetry(std::vector<Telemetry>& out) override;
    bool finished() const override { return t > DURATION; }

private:
    World& world;
    Mesh rings;
    float camY = 8;
    static constexpr float DURATION = 11.0f;
};
