// =============================================================================
//  Scene 3  (video 0:20 - 0:30)  -  UNDER THE RESERVOIR: THE INTAKE
//  We are under water in front of the dam face.  Sunlight comes down in
//  shafts, caustics dance on the concrete, dust floats in the water.  The
//  camera swims towards the intake opening and passes between the bars of
//  the trash rack into the tunnel.
// =============================================================================
#pragma once
#include "Scene.h"
#include "../objects/TrashRack.h"
#include "../render/ParticleSystem.h"

class Scene3_Underwater : public Scene {
public:
    std::string title() const override { return "INTAKE - UNDER WATER"; }
    std::string caption() const override { return "16 M UNDER THE SURFACE THE WATER ENTERS THE DAM THROUGH A STEEL TRASH RACK"; }
    void build() override;
    void enter() override;
    void update(float dt) override;
    void camera(Camera& cam) override;
    void lighting(Lighting& L) override;
    Vec3 shadowCentre() const override { return Vec3(0, 22, 0); }
    float shadowRadius() const override { return 32.0f; }
    void drawOpaque(Renderer& r) override;
    void drawTransparent(Renderer& r) override;
    void telemetry(std::vector<Telemetry>& out) override;
    void buttons(std::vector<Button>& out) override;
    void onButton(int id) override { if (id == 1) skip = true; }
    bool finished() const override { return skip || t > 10.5f; }

private:
    Mesh wall, column, tunnel, hood, rocks, surface, shafts, glowEnd, seabed;
    TrashRack rack;
    ParticleSystem dust;
    Rng rng{5};
    Vec3 camPos;
    bool skip = false;
    static constexpr float SURFACE_Y = 40.0f;
};
