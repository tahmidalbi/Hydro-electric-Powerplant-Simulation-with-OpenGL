// =============================================================================
//  Scene 5  (video 0:40 - 0:50)  -  INTAKE GATE
//  Cut-away of the intake: reservoir water on the left, the steel gate in
//  the middle, the dry conduit on the right.  Press OPEN GATE: the hydraulic
//  cylinders lift the gate, water bursts out underneath, races along the
//  floor and fills the conduit.  Then the story follows it into the penstock.
// =============================================================================
#pragma once
#include "Scene.h"
#include "../objects/IntakeGate.h"
#include "../render/ParticleSystem.h"

class Scene5_IntakeGate : public Scene {
public:
    std::string title() const override { return "INTAKE GATE"; }
    std::string caption() const override {
        return gateTarget ? "THE GATE RISES - RESERVOIR PRESSURE PUSHES THE WATER INTO THE CONDUIT"
                          : "THE GATE HOLDS BACK THE RESERVOIR.  PRESS OPEN GATE TO LET THE WATER IN";
    }
    void build() override;
    void enter() override;
    void update(float dt) override;
    void camera(Camera& cam) override;
    void lighting(Lighting& L) override;
    Vec3 shadowCentre() const override { return Vec3(5, 8, 0); }
    float shadowRadius() const override { return 38.0f; }
    void drawOpaque(Renderer& r) override;
    void drawTransparent(Renderer& r) override;
    void telemetry(std::vector<Telemetry>& out) override;
    void buttons(std::vector<Button>& out) override;
    void onButton(int id) override;
    void autoPlay() override { if (t > 1.0f && !gateTarget && !everOpened) onButton(1); }
    bool finished() const override { return next || (everOpened && filledTime > 3.0f); }

private:
    IntakeGate gate;
    Mesh structure, floorTiles, darkBlock, resTop, resFront, flowTop, flowFront;
    ParticleSystem spray;
    std::vector<Vertex> topVerts, frontVerts;
    Rng rng{31};
    bool gateTarget = false, everOpened = false, next = false;
    float lift = 0;            // metres the gate is raised
    float front = 0;           // how far the water has run along the conduit (x)
    float level = 0;           // water depth in the conduit
    float filledTime = 0;
    void rebuildFlowMesh();
    float profile(float x) const;
};
