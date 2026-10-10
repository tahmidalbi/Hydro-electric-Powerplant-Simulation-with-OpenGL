// =============================================================================
//  Scene 9  (video 2:00 - 2:30)  -  GENERATOR
//  Cut-away of the generator in the bright powerhouse hall: 40 rotor poles
//  spin inside the copper stator coils.  EXCITE ROTOR sends DC current into
//  the field coils: the poles become electromagnets (blue field lines), the
//  turning field induces three-phase AC in the stator (orange current) and
//  the oscilloscope shows the three sine waves at 50 Hz.
//  FOLLOW THE POWER is enabled once the generator gives full voltage.
// =============================================================================
#pragma once
#include "Scene.h"
#include "../objects/World.h"

class Scene9_Generator : public Scene {
public:
    explicit Scene9_Generator(World& w) : world(w) {}
    std::string title() const override { return "GENERATOR"; }
    std::string caption() const override {
        return world.excited ? "THE SPINNING MAGNETIC FIELD INDUCES THREE-PHASE CURRENT IN THE COPPER COILS"
                             : "THE ROTOR SPINS, BUT WITHOUT FIELD CURRENT IT IS NOT YET A MAGNET";
    }
    void enter(bool continuous) override;
    void update(float dt) override { t += dt; world.update(dt); }
    void camera(Camera& cam) override;
    void lighting(Lighting& L, const Vec3& camPos) override { world.lighting(L, camPos); }
    Vec3 shadowCentre() const override { return Vec3(62, 17, 64); }
    float shadowRadius() const override { return 20.0f; }
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
    bool ready() const { return world.voltageKV() >= 10.5f; }
};
