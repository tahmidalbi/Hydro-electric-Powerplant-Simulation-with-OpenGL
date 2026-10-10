// =============================================================================
//  Scene 10  (video 2:30 - 2:50)  -  BUS DUCTS + STEP-UP TRANSFORMER
//  The camera follows the power: up out of the hall, over the roof and down
//  to the yard, where three bus ducts bring the 11 kV current through the
//  wall to the step-up transformer.  X-RAY shows the iron core and the
//  low- and high-voltage windings inside the tank (ratio 1 : 20).
// =============================================================================
#pragma once
#include "Scene.h"
#include "../objects/World.h"

class Scene10_Transformer : public Scene {
public:
    explicit Scene10_Transformer(World& w) : world(w) {}
    std::string title() const override { return "STEP-UP TRANSFORMER"; }
    std::string caption() const override { return "11 KV FROM THE GENERATOR IS RAISED TO 220 KV - LESS CURRENT, LESS LOSS ON THE WAY"; }
    void enter(bool continuous) override;
    void update(float dt) override { t += dt; world.update(dt); }
    void camera(Camera& cam) override;
    void lighting(Lighting& L, const Vec3& camPos) override { world.lighting(L, camPos); }
    Vec3 shadowCentre() const override { return Vec3(80, 10, 62); }
    float shadowRadius() const override { return 32.0f; }
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
};
