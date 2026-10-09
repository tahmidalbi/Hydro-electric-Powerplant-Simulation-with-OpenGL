// =============================================================================
//  Scene 5  (video 1:00 - 1:10)  -  RAY-TRACED PIPE  (GPU RAY TRACING)
//  The camera has moved in close to the penstock; the picture dissolves into
//  the sci-fi "hologram" shot: a glass section of the penstock in a dark
//  engineering space.  Water races through it as glowing streams.
//  This whole picture is made by the RAY TRACER in shaders/raytrace.frag:
//  refraction through the glass and water, ray-traced reflections on the
//  floor, shadow rays, and volume ray-marching of the glowing water.
//  UP / DOWN change the head (height of the water) -> v = sqrt(2 g h).
//  FOLLOW TO MAIN INLET VALVE dissolves back into the powerhouse.
// =============================================================================
#pragma once
#include "Scene.h"

class Scene5_RayTracedPipe : public Scene {
public:
    std::string title() const override { return "RAY-TRACED PIPE"; }
    std::string caption() const override { return "RAY-TRACED VIEW INSIDE THE PENSTOCK - THE WATER NOW MOVES AT TURBINE SPEED"; }
    void build() override;
    void enter(bool continuous) override;
    void update(float dt) override;
    void camera(Camera& cam) override;
    void lighting(Lighting& L, const Vec3& camPos) override;
    void drawOpaque(Renderer& r) override;
    bool rayTraced() const override { return true; }
    bool dissolveOnEnter() const override { return true; }
    void hud(Ui& ui, const Mat4& viewProj) override;
    void telemetry(std::vector<Telemetry>& out) override;
    void buttons(std::vector<Button>& out) override;
    void onButton(int id) override;
    void autoPlay() override { if (t > 9.0f) restart = true; }
    bool finished() const override { return restart; }

private:
    Shader rt;
    float head = 53.0f;          // metres of water above the turbine
    float flowVisual = 0;        // smoothed streak speed
    float flowOn = 0;
    float travelled = 0;         // camera position along the pipe
    bool restart = false;
    Vec3 pipeA{0, 6.5f, 0}, pipeDir;
    float speed() const { return std::sqrt(2.0f * 9.81f * head); }
};
