// =============================================================================
//  Scene 11  (video 2:50 - 3:00)  -  SWITCHYARD + POWER LINE + SUMMARY
//  In the switchyard the 220 kV passes a disconnector, the CIRCUIT BREAKER,
//  current transformers and surge arresters.  CLOSE BREAKER connects the
//  plant to the grid: the energy runs out along the transmission line and the
//  camera flies with it down the valley.  At the end a summary shows the
//  whole energy chain.  REPLAY JOURNEY starts again from the preview.
// =============================================================================
#pragma once
#include "Scene.h"
#include "../objects/World.h"

class Scene11_Grid : public Scene {
public:
    explicit Scene11_Grid(World& w) : world(w) {}
    std::string title() const override { return "TO THE GRID"; }
    std::string caption() const override {
        return world.breakerClosed ? "THE BREAKER IS CLOSED - 86 MW FLOW INTO THE 220 KV GRID"
                                   : "THE CIRCUIT BREAKER IS OPEN - THE PLANT IS NOT YET CONNECTED";
    }
    void enter(bool continuous) override;
    void update(float dt) override;
    void camera(Camera& cam) override;
    void lighting(Lighting& L, const Vec3& camPos) override { world.lighting(L, camPos); }
    Vec3 shadowCentre() const override { return shadowAt; }
    float shadowRadius() const override { return 60.0f; }
    void drawOpaque(Renderer& r) override { world.drawOpaque(r); }
    void drawTransparent(Renderer& r) override { world.drawTransparent(r); }
    void hud(Ui& ui, const Mat4& viewProj) override;
    void telemetry(std::vector<Telemetry>& out) override;
    void buttons(std::vector<Button>& out) override;
    void onButton(int id) override;
    void autoPlay() override;
    bool finished() const override { return replay; }

private:
    World& world;
    bool replay = false;
    float closedAt = -1.0f;      // scene time when the breaker was closed
    Vec3 shadowAt{100, 8, 70};
    float flightS() const;       // how far along the line the camera is
    void drawSummary(Ui& ui, float alpha);
};
