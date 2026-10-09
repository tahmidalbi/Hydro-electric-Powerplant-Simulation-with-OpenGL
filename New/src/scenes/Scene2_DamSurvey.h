// =============================================================================
//  Scene 2  (video 0:10 - 0:20)  -  DAM SURVEY
//  Daylight view of the whole dam with the gates closed.  Engineering
//  dimension lines are drawn over it (height, reservoir level, head, crest
//  length) like the measuring overlay in the video.  Then the story dives
//  under the water to the intake.
// =============================================================================
#pragma once
#include "Scene.h"
#include "../objects/DamSite.h"

class Scene2_DamSurvey : public Scene {
public:
    explicit Scene2_DamSurvey(DamSite& s) : site(s) {}
    std::string title() const override { return "DAM SURVEY"; }
    std::string caption() const override { return "THE RESERVOIR STORES WATER HIGH UP - THIS HEIGHT (HEAD) IS THE ENERGY WE WILL USE"; }
    void build() override {}
    void enter() override;
    void update(float dt) override;
    void camera(Camera& cam) override;
    void lighting(Lighting& L) override;
    Vec3 shadowCentre() const override { return Vec3(-10, 30, 20); }
    float shadowRadius() const override { return 170.0f; }
    void drawOpaque(Renderer& r) override { site.drawOpaque(r); }
    void drawTransparent(Renderer& r) override { site.drawTransparent(r); }
    void hud(Ui& ui, const Mat4& viewProj) override;
    void telemetry(std::vector<Telemetry>& out) override;
    void buttons(std::vector<Button>& out) override;
    void onButton(int id) override { if (id == 1) skip = true; }
    bool finished() const override { return skip || t > 10.5f; }

private:
    DamSite& site;
    bool skip = false;
};
