#include "SelfTest.h"
#include "../core/Camera.h"
#include "../core/Clock.h"
#include "../core/SceneState.h"
#include "../core/Transform.h"
#include "../input/Picking.h"
#include "../objects/Layout.h"
#include "../objects/Placement.h"
#include "../objects/SolarArray.h"
#include "../objects/Terrain.h"
#include "../systems/Animation.h"
#include "../systems/BoatSystem.h"
#include "../systems/CameraDirector.h"
#include "../systems/Effects.h"
#include "../systems/Environment.h"
#include "../systems/HydroSystem.h"
#include "../systems/Particles.h"
#include "../systems/Waves.h"
#include "../systems/WindSystem.h"
#include "../ui/DemoGallery.h"
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>
#include <cstdio>
#include <string>
#include <vector>

namespace {

int failures = 0;
int checks = 0;

void check(bool ok, const std::string& name, const std::string& detail) {
    ++checks;
    if (!ok) ++failures;
    std::printf("  [%s] %-46s %s\n", ok ? "PASS" : "FAIL", name.c_str(), detail.c_str());
}

std::string fmt(const char* format, double a, double b = 0.0, double c = 0.0) {
    char buffer[160];
    std::snprintf(buffer, sizeof(buffer), format, a, b, c);
    return buffer;
}

float maxDifference(const glm::mat4& a, const glm::mat4& b) {
    float worst = 0.0f;
    for (int c = 0; c < 4; ++c)
        for (int r = 0; r < 4; ++r)
            worst = std::fmax(worst, std::fabs(a[c][r] - b[c][r]));
    return worst;
}

// Smallest difference between two angles, ignoring whole turns.
float angleDifference(float a, float b) {
    const float twoPi = 6.28318530718f;
    float d = std::fmod(std::fabs(a - b), twoPi);
    return std::fmin(d, twoPi - d);
}

// Runs `seconds` of real time through Clock + Animation using a frame-time
// pattern, exactly as the main loop does. Returns the final plant state.
PlantState simulate(float seconds, float frameA, float frameB) {
    Clock clock;
    PlantState plant;
    Animation::reset(plant);
    double now = 0.0;
    clock.tick(now);
    bool useA = true;
    while (now < seconds - 1e-9) {
        double step = std::fmin(useA ? frameA : frameB, seconds - now);
        now += step;
        useA = !useA;
        clock.tick(now);
        Animation::update(plant, clock.simulationDelta());
    }
    return plant;
}

// ---------------------------------------------------------------------------
void testStep1Time() {
    std::printf("\nSTEP 1 - time and state\n");

    // Frame-rate independence: 10 s at 30, 60, 144 FPS and an uneven pattern.
    PlantState at30  = simulate(10.0f, 1.0f / 30.0f,  1.0f / 30.0f);
    PlantState at60  = simulate(10.0f, 1.0f / 60.0f,  1.0f / 60.0f);
    PlantState at144 = simulate(10.0f, 1.0f / 144.0f, 1.0f / 144.0f);
    PlantState uneven = simulate(10.0f, 0.005f, 0.028f);
    float worst = 0.0f;
    for (const PlantState* p : {&at60, &at144, &uneven})
        for (int i = 0; i < 2; ++i)
            worst = std::fmax(worst, angleDifference(p->rotorAngleRad[i], at30.rotorAngleRad[i]));
    check(worst < 1e-3f, "same rotor angle after 10 s at any FPS",
          fmt("30/60/144/uneven FPS differ by at most %.6f rad", worst));
    check(std::fabs(at144.simulationTimeSeconds - 10.0) < 1e-4, "simulation time = real time at x1",
          fmt("%.5f s simulated in 10 s", at144.simulationTimeSeconds));

    // Pause stops the rotor; resume continues without a jump.
    Clock clock;
    PlantState plant;
    Animation::reset(plant);
    double now = 0.0;
    const double dt = 1.0 / 60.0;
    clock.tick(now);
    for (int i = 0; i < 120; ++i) { now += dt; clock.tick(now); Animation::update(plant, clock.simulationDelta()); }
    float beforePause = plant.rotorAngleRad[0];
    clock.setPaused(true);
    for (int i = 0; i < 180; ++i) { now += dt; clock.tick(now); Animation::update(plant, clock.simulationDelta()); }
    float afterPause = plant.rotorAngleRad[0];
    check(afterPause == beforePause && clock.realDelta() > 0.0f, "pause freezes the rotor (real time continues)",
          fmt("angle before %.5f, after 3 s paused %.5f", beforePause, afterPause));
    clock.setPaused(false);
    now += dt; clock.tick(now); Animation::update(plant, clock.simulationDelta());
    float firstStep = angleDifference(plant.rotorAngleRad[0], afterPause);
    float expectedStep = plant.rotorSpeedRadPerSecond[0] * (float)dt;
    check(std::fabs(firstStep - expectedStep) < 1e-5f, "resume continues without a jump",
          fmt("first step %.5f rad, one normal frame = %.5f rad", firstStep, expectedStep));

    // A 2-second stall is clamped.
    Clock stall;
    stall.tick(5.0);
    stall.tick(7.0);
    check(stall.realDelta() <= Clock::MAX_DELTA_SECONDS + 1e-6f, "a 2 s stall is clamped",
          fmt("2.0 s frame counted as %.3f s", stall.realDelta()));

    // Time scale x2 doubles the travel.
    Clock fast;
    fast.faster();
    PlantState fastPlant;
    Animation::reset(fastPlant);
    double t = 0.0;
    fast.tick(t);
    for (int i = 0; i < 300; ++i) { t += dt; fast.tick(t); Animation::update(fastPlant, fast.simulationDelta()); }
    check(std::fabs(fastPlant.simulationTimeSeconds - 10.0) < 1e-3, "time scale x2: 5 s real = 10 s simulated",
          fmt("%.4f s simulated", fastPlant.simulationTimeSeconds));

    // Reset returns the documented start pose.
    Animation::reset(fastPlant);
    check(fastPlant.simulationTimeSeconds == 0.0 && fastPlant.rotorAngleRad[0] == 0.35f && fastPlant.rotorAngleRad[1] == 1.35f,
          "reset restores the start pose", "t = 0, rotors at 0.35 and 1.35 rad");
}

// ---------------------------------------------------------------------------
void testStep2Transforms() {
    using namespace Transform;
    std::printf("\nSTEP 2 - transformations (hand-written matrices vs glm)\n");

    float worst = 0.0f;
    const glm::mat4 I(1.0f);
    for (float a : {-2.1f, -0.3f, 0.0f, 0.7f, 1.9f, 3.0f}) {
        worst = std::fmax(worst, maxDifference(rotateX(a), glm::rotate(I, a, {1, 0, 0})));
        worst = std::fmax(worst, maxDifference(rotateY(a), glm::rotate(I, a, {0, 1, 0})));
        worst = std::fmax(worst, maxDifference(rotateZ(a), glm::rotate(I, a, {0, 0, 1})));
    }
    worst = std::fmax(worst, maxDifference(translate({3, -2, 7}), glm::translate(I, {3, -2, 7})));
    worst = std::fmax(worst, maxDifference(scale({2, 0.5f, 4}), glm::scale(I, {2, 0.5f, 4})));
    check(worst < 1e-5f, "translate/scale/rotateX/Y/Z match glm", fmt("largest entry difference %.2e", worst));

    // Matrix order (the gallery's bar, in its exhibit frame): T*R*S keeps the
    // bar's centre on top of its post; S*R*T does not.
    glm::vec3 post(2.0f, 3.0f, 0.0f);
    glm::mat4 T = translate(post), R = rotateY(2.5f), S = scale({3.0f, 0.5f, 0.5f});
    glm::vec3 centreCorrect = glm::vec3(T * R * S * glm::vec4(0, 0, 0, 1));
    glm::vec3 centreWrong   = glm::vec3(S * R * T * glm::vec4(0, 0, 0, 1));
    check(glm::length(centreCorrect - post) < 1e-5f && glm::length(centreWrong - post) > 1.0f,
          "T*R*S spins in place, S*R*T flies off",
          fmt("centre offset: correct %.4f, wrong %.2f units", glm::length(centreCorrect - post), glm::length(centreWrong - post)));

    // Pivot: the hinge point does not move; other points keep their distance to it.
    glm::vec3 hinge(8.5f, 3.0f, -8.0f), corner(10.6f, 6.2f, -8.0f);
    glm::mat4 P = rotateAboutPivot(hinge, rotateX(0.6f));
    glm::vec3 hingeAfter = glm::vec3(P * glm::vec4(hinge, 1.0f));
    glm::vec3 cornerAfter = glm::vec3(P * glm::vec4(corner, 1.0f));
    check(glm::length(hingeAfter - hinge) < 1e-5f && std::fabs(glm::length(cornerAfter - hinge) - glm::length(corner - hinge)) < 1e-4f,
          "rotation about a pivot keeps the pivot fixed",
          fmt("pivot moved %.2e, corner radius change %.2e", glm::length(hingeAfter - hinge),
              std::fabs(glm::length(cornerAfter - hinge) - glm::length(corner - hinge))));

    // Hierarchy: moving the parent root moves the child by exactly the same amount.
    glm::mat4 root = translate({-28.5f, 4.0f, -65.0f}) * rotateY(0.8f);
    glm::mat4 hubLocal = translate({0.0f, 12.35f, 1.2f});
    glm::vec3 offset(2.5f, 0.0f, -1.0f);
    glm::vec3 hubBefore = glm::vec3(root * hubLocal * glm::vec4(0, 0, 0, 1));
    glm::vec3 hubAfter  = glm::vec3(translate(offset) * root * hubLocal * glm::vec4(0, 0, 0, 1));
    check(glm::length((hubAfter - hubBefore) - offset) < 1e-4f, "moving the root carries its children",
          fmt("hub moved (%.2f, %.2f, %.2f)", hubAfter.x - hubBefore.x, hubAfter.y - hubBefore.y, hubAfter.z - hubBefore.z));

    // Normal matrix under non-uniform scale: normal must stay perpendicular to the surface.
    glm::mat4 M = scale({3.0f, 1.0f, 1.0f});
    glm::vec3 tangent(1.0f, 1.0f, 0.0f), normal = glm::normalize(glm::vec3(1.0f, -1.0f, 0.0f));
    glm::vec3 tangentWorld = glm::vec3(M * glm::vec4(tangent, 0.0f));
    float correctDot = glm::dot(tangentWorld, glm::normalize(normalMatrix(M) * normal));
    float wrongDot   = glm::dot(tangentWorld, glm::normalize(glm::mat3(M) * normal));
    check(std::fabs(correctDot) < 1e-5f && std::fabs(wrongDot) > 0.5f, "(M^-1)^T keeps normals perpendicular",
          fmt("normal.tangent: inverse-transpose %.2e, plain model matrix %.3f", correctDot, wrongDot));

    // Shear: the base stays, the top leans.  Reflection: mirrors and flips winding.
    glm::mat4 H = shearXByY(0.45f);
    glm::vec3 base = glm::vec3(H * glm::vec4(1, 0, 0, 1)), top = glm::vec3(H * glm::vec4(1, 5, 0, 1));
    check(base.x == 1.0f && std::fabs(top.x - 3.25f) < 1e-5f, "shear leans the top, keeps the base",
          fmt("base x %.2f -> %.2f, top x 1.00 -> %.2f", 1.0, base.x, top.x));
    glm::mat4 F = reflectAcrossPlaneX(20.5f);
    glm::vec3 mirrored = glm::vec3(F * glm::vec4(18.0f, 2.0f, -8.0f, 1.0f));
    float det = glm::determinant(glm::mat3(F));
    check(std::fabs(mirrored.x - 23.0f) < 1e-4f && det < 0.0f, "reflection mirrors (determinant -1)",
          fmt("x 18.0 -> %.2f, det = %.1f (negative = winding flipped)", mirrored.x, det));
}

// ---------------------------------------------------------------------------
// The specular term of lighting_common.glsl, for the lamp only (Phong form).
float lampSpecular(const glm::vec3& point, const glm::vec3& lamp, const glm::vec3& eye, float shininess) {
    glm::vec3 N(0.0f, 1.0f, 0.0f);
    glm::vec3 L = glm::normalize(lamp - point);
    glm::vec3 V = glm::normalize(eye - point);
    glm::vec3 R = glm::reflect(-L, N);
    return std::pow(std::fmax(glm::dot(R, V), 0.0f), shininess);
}

void testStep3And4Shading() {
    std::printf("\nSTEPS 3-4 - lighting and Gouraud vs Phong (CPU copy of the shader formula)\n");

    const glm::vec3 eye = DemoGallery::cameraEye();
    const glm::vec3 lamp = DemoGallery::lampStartPosition();
    const glm::vec3 c = DemoGallery::waterPatchCentre();
    const float h = DemoGallery::waterPatchSize() * 0.5f;
    const float shininess = 150.0f;   // Materials::GALLERY_WATER

    // LOW detail patch = 2 triangles, corners only. gridData() splits the quad
    // along the diagonal (+x,-z) -- (-x,+z), and the centre lies on it.
    float cornerA = lampSpecular(c + glm::vec3(h, 0, -h), lamp, eye, shininess);
    float cornerB = lampSpecular(c + glm::vec3(-h, 0, h), lamp, eye, shininess);
    float gouraudCentre = 0.5f * (cornerA + cornerB);           // blended corner values
    float phongCentre = lampSpecular(c, lamp, eye, shininess);  // evaluated right there

    // Highest specular anywhere on the patch, for each method (21 x 21 samples).
    float phongPeak = 0.0f, gouraudPeak = 0.0f;
    float corners[4] = {lampSpecular(c + glm::vec3(-h, 0, -h), lamp, eye, shininess),
                        lampSpecular(c + glm::vec3( h, 0, -h), lamp, eye, shininess),
                        lampSpecular(c + glm::vec3(-h, 0,  h), lamp, eye, shininess),
                        lampSpecular(c + glm::vec3( h, 0,  h), lamp, eye, shininess)};
    for (float corner : corners) gouraudPeak = std::fmax(gouraudPeak, corner);   // linear blend never exceeds its corners
    for (int i = 0; i <= 20; ++i)
        for (int j = 0; j <= 20; ++j)
            phongPeak = std::fmax(phongPeak, lampSpecular(c + glm::vec3(-h + 2 * h * i / 20.0f, 0, -h + 2 * h * j / 20.0f), lamp, eye, shininess));

    check(phongCentre > 0.9f && gouraudCentre < 0.01f, "Gouraud misses the highlight Phong finds",
          fmt("patch centre: Phong %.3f, Gouraud %.5f", phongCentre, gouraudCentre));
    check(phongPeak > 50.0f * gouraudPeak, "brightest highlight on the coarse patch",
          fmt("Phong %.3f vs Gouraud %.5f", phongPeak, gouraudPeak));

    // Point-light attenuation falls off with distance (Step 3).
    auto attenuation = [](float d) { return 1.0f / (1.0f + 0.045f * d + 0.0075f * d * d); };
    check(attenuation(0) == 1.0f && attenuation(9) < 0.6f && attenuation(17) < attenuation(9),
          "point light gets weaker with distance", fmt("d=0: %.2f, d=9: %.2f, d=17: %.2f", attenuation(0), attenuation(9), attenuation(17)));
}

// Runs the plant (and optionally the particles) for `seconds` at 60 FPS.
void run(PlantState& plant, float seconds, ParticleSystem* particles = nullptr, EffectsState* fx = nullptr) {
    const float dt = 1.0f / 60.0f;
    for (float t = 0.0f; t < seconds; t += dt) {
        Animation::update(plant, dt);
        if (particles && fx) Effects::update(*particles, *fx, plant, dt);
    }
}

constexpr float RPM = 6.28318530718f / 60.0f;   // 1 rpm in rad/s

// ---------------------------------------------------------------------------
void testStep5Wind() {
    std::printf("\nSTEP 5 - wind, turbines, clouds\n");

    float calm = WindSystem::targetRotorSpeed(2.0f), cutIn = WindSystem::targetRotorSpeed(3.0f);
    float rated = WindSystem::targetRotorSpeed(12.0f), strong = WindSystem::targetRotorSpeed(20.0f);
    float storm = WindSystem::targetRotorSpeed(26.0f);
    check(calm == 0.0f && std::fabs(cutIn - 6 * RPM) < 1e-4f && std::fabs(rated - 16 * RPM) < 1e-4f &&
          std::fabs(strong - rated) < 1e-6f && storm == 0.0f,
          "rotor curve: cut-in, rated, cut-out",
          fmt("2 m/s: %.0f rpm, 3: %.0f rpm, 12-20: %.0f rpm, 26 (storm): 0", calm / RPM, cutIn / RPM, rated / RPM));

    // A sudden wind change must not make the rotor jump.
    PlantState plant;
    Animation::reset(plant);
    plant.windTargetMetersPerSecond = 20.0f;   // gust from 9 to 20 m/s
    float worstStep = 0.0f, previousSpeed = plant.rotorSpeedRadPerSecond[0];
    for (int i = 0; i < 60 * 12; ++i) {
        Animation::update(plant, 1.0f / 60.0f);
        worstStep = std::fmax(worstStep, std::fabs(plant.rotorSpeedRadPerSecond[0] - previousSpeed));
        previousSpeed = plant.rotorSpeedRadPerSecond[0];
    }
    check(worstStep < 0.01f && std::fabs(plant.rotorSpeedRadPerSecond[0] - rated) < 0.05f * rated,
          "wind change: rotor speeds up smoothly",
          fmt("largest change in one frame %.4f rad/s; after 12 s %.1f of %.0f rpm", worstStep,
              plant.rotorSpeedRadPerSecond[0] / RPM, rated / RPM));
    check(std::fabs(plant.rotorSpeedRadPerSecond[1] - rated * WindSystem::SECOND_TURBINE_FACTOR) < 0.05f * rated,
          "the two turbines are independent", fmt("turbine 2 turns at %.1f rpm (factor %.2f)",
                                                  plant.rotorSpeedRadPerSecond[1] / RPM, WindSystem::SECOND_TURBINE_FACTOR));

    // Clouds: stay inside the wrap band, and the wrap happens far outside the view.
    float minX = 1e9f, maxX = -1e9f, biggestJumpInView = 0.0f, previous = WindSystem::cloudX(110.0f, 0.0f, 1.0f);
    for (float drift = 0.5f; drift < 5000.0f; drift += 0.5f) {
        float x = WindSystem::cloudX(110.0f, drift, 1.0f);
        minX = std::fmin(minX, x);
        maxX = std::fmax(maxX, x);
        if (std::fabs(x) < 300.0f) biggestJumpInView = std::fmax(biggestJumpInView, std::fabs(x - previous));
        previous = x;
    }
    check(minX >= -WindSystem::CLOUD_WRAP && maxX < WindSystem::CLOUD_WRAP && biggestJumpInView < 1.0f,
          "clouds drift and wrap out of sight",
          fmt("x stays in [%.0f, %.0f]; largest step while |x| < 300: %.2f m", minX, maxX, biggestJumpInView));
}

// ---------------------------------------------------------------------------
void testStep6Hydro() {
    std::printf("\nSTEP 6 - gate, flow, wheel\n");

    PlantState plant;
    Animation::reset(plant);
    float start = plant.gateOpening;
    HydroSystem::toggleGate(plant);   // 0.45 -> open
    float worstStep = 0.0f, previous = plant.gateOpening;
    float at4 = 0.0f;
    for (int i = 0; i < 60 * 6; ++i) {
        Animation::update(plant, 1.0f / 60.0f);
        worstStep = std::fmax(worstStep, plant.gateOpening - previous);
        previous = plant.gateOpening;
        if (i == 60 * 4) at4 = plant.gateOpening;
    }
    check(worstStep <= HydroSystem::GATE_SPEED_PER_SECOND / 60.0f + 1e-6f && at4 < 1.0f && plant.gateOpening > 0.9999f,
          "gate travels at its limited speed",
          fmt("%.2f -> 1.00 at 12.5%%/s: at 4 s %.2f, at 6 s %.2f", start, at4, plant.gateOpening));

    for (int i = 0; i < 100; ++i) HydroSystem::nudgeTarget(plant, 0.1f);
    run(plant, 2.0f);
    bool inSlot = plant.gateTarget <= 1.0f && plant.gateOpening <= 1.0f;
    for (int i = 0; i < 100; ++i) HydroSystem::nudgeTarget(plant, -0.1f);
    run(plant, 10.0f);
    inSlot = inSlot && plant.gateTarget >= 0.0f && plant.gateOpening >= 0.0f;
    check(inSlot && plant.gateOpening == 0.0f, "gate stays inside its slot (0..1)", "pushed far past both ends: clamped");

    // Wheel follows the flow: open fully (and let the wheel settle), then shut.
    HydroSystem::toggleGate(plant);
    run(plant, 40.0f);
    float fullSpeed = plant.wheelSpeedRadPerSecond;
    HydroSystem::toggleGate(plant);
    float before = plant.wheelSpeedRadPerSecond;
    bool neverSpeedsUp = true;
    for (int i = 0; i < 60 * 25; ++i) {
        Animation::update(plant, 1.0f / 60.0f);
        neverSpeedsUp = neverSpeedsUp && plant.wheelSpeedRadPerSecond <= before + 1e-6f;
        before = plant.wheelSpeedRadPerSecond;
    }
    check(std::fabs(fullSpeed - HydroSystem::WHEEL_MAX_SPEED) < 0.02f && neverSpeedsUp && plant.wheelSpeedRadPerSecond < 0.02f,
          "wheel speed follows the gate",
          fmt("open: %.1f rpm; shut: slows smoothly to %.2f rpm", fullSpeed / RPM, plant.wheelSpeedRadPerSecond / RPM));

    // Automatic demonstration covers the whole range.
    plant.gateAutoDemo = true;
    float lo = 1.0f, hi = 0.0f;
    for (int i = 0; i < 60 * 45; ++i) {
        Animation::update(plant, 1.0f / 60.0f);
        lo = std::fmin(lo, plant.gateOpening);
        hi = std::fmax(hi, plant.gateOpening);
    }
    check(lo < 0.05f && hi > 0.95f, "J: automatic gate demonstration", fmt("gate went from %.2f to %.2f by itself", lo, hi));

    // Spray: plenty while open, none left a while after shutting.
    ParticleSystem particles;
    EffectsState fx;
    Animation::reset(plant);
    HydroSystem::toggleGate(plant);
    run(plant, 12.0f, &particles, &fx);
    int sprayOpen = 0;
    for (const Particle& p : particles.slots()) sprayOpen += (p.alive() && !p.flat) ? 1 : 0;
    HydroSystem::toggleGate(plant);
    run(plant, 18.0f, &particles, &fx);
    int sprayShut = 0;
    for (const Particle& p : particles.slots()) sprayShut += (p.alive() && !p.flat) ? 1 : 0;
    check(sprayOpen > 500 && sprayShut == 0, "spray follows the flow", fmt("gate open: %.0f spray particles; 18 s after shutting: %.0f",
                                                                            sprayOpen, sprayShut));
}

// ---------------------------------------------------------------------------
void testStep7Boat() {
    std::printf("\nSTEP 7 - waves and boat\n");

    // The CPU normal must match the slope of the CPU height (finite differences).
    float worst = 0.0f;
    for (int i = 0; i < 50; ++i) {
        float x = 20.0f + i * 3.1f, z = -150.0f - i * 1.7f, t = i * 0.37f, e = 0.01f;
        float dx = (Waves::height(x + e, z, t, 9.0f) - Waves::height(x - e, z, t, 9.0f)) / (2 * e);
        float dz = (Waves::height(x, z + e, t, 9.0f) - Waves::height(x, z - e, t, 9.0f)) / (2 * e);
        glm::vec3 numeric = glm::normalize(glm::vec3(-dx, 1.0f, -dz));
        worst = std::fmax(worst, glm::length(numeric - Waves::normal(x, z, t, 9.0f)));
    }
    check(worst < 2e-3f, "wave normal = slope of the wave height", fmt("largest difference over 50 points: %.5f", worst));

    float calmScale = Waves::amplitudeScale(1.0f), galeScale = Waves::amplitudeScale(25.0f);
    check(galeScale > 3.0f * calmScale, "stronger wind -> taller waves", fmt("scale %.2f in calm air, %.2f in a gale", calmScale, galeScale));

    // The patrol route lies on open water all the way round.
    int onWater = 0;
    for (int i = 0; i < 360; ++i) {
        float a = 6.28318f * i / 360.0f;
        glm::vec2 p = BoatSystem::patrolPoint(a), next = BoatSystem::patrolPoint(a - 0.01f);
        float heading = std::atan2(next.x - p.x, next.y - p.y);
        onWater += BoatSystem::hullFits(p, heading) ? 1 : 0;
    }
    check(onWater == 360, "patrol route is on open water", fmt("%.0f of 360 route points fit the hull", onWater));

    // Patrol for two minutes: keeps moving, never beaches, floats on the waves.
    PlantState plant;
    Animation::reset(plant);
    float travelled = 0.0f, worstFloat = 0.0f;
    bool alwaysOnWater = true;
    glm::vec2 last = plant.boatXZ;
    for (int i = 0; i < 60 * 120; ++i) {
        Animation::update(plant, 1.0f / 60.0f);
        travelled += glm::length(plant.boatXZ - last);
        last = plant.boatXZ;
        alwaysOnWater = alwaysOnWater && BoatSystem::isNavigable(plant.boatXZ.x, plant.boatXZ.y);
        float surface = Waves::height(plant.boatXZ.x, plant.boatXZ.y, (float)plant.simulationTimeSeconds, plant.windMetersPerSecond);
        worstFloat = std::fmax(worstFloat, std::fabs(plant.boatWaterHeight - surface));
    }
    check(alwaysOnWater && travelled > 400.0f && !plant.boatBlockedByShore, "patrol: 2 minutes on the water",
          fmt("travelled %.0f m, never left navigable water", travelled));
    check(worstFloat < 0.25f, "the boat rides the drawn waves", fmt("boat height vs wave height under it: within %.3f m", worstFloat));

    // Manual: full throttle, rudder slightly left, for 3 minutes. It must
    // stop at the shore instead of driving onto land.
    Animation::reset(plant);
    plant.boatMode = BoatMode::Manual;
    plant.boatThrottleInput = 1.0f;
    plant.boatSteerInput = 0.15f;
    bool everBlocked = false;
    alwaysOnWater = true;
    for (int i = 0; i < 60 * 180; ++i) {
        Animation::update(plant, 1.0f / 60.0f);
        everBlocked = everBlocked || plant.boatBlockedByShore;
        alwaysOnWater = alwaysOnWater && BoatSystem::hullFits(plant.boatXZ, plant.boatHeadingRad);
    }
    check(alwaysOnWater && everBlocked, "manual: the shore stops the boat", "full throttle for 3 minutes: stopped at the shore, hull never on land");

    float minTilt, maxTilt;
    SolarArray::tiltLimits(minTilt, maxTilt);
    plant.solarTiltDeg = 90.0f;
    run(plant, 0.1f);
    float high = plant.solarTiltDeg;
    plant.solarTiltDeg = -20.0f;
    run(plant, 0.1f);
    float lowest = std::fmin(SolarArray::groundClearance(minTilt), SolarArray::groundClearance(maxTilt));
    check(high == maxTilt && plant.solarTiltDeg == minTilt && maxTilt >= 30.0f && lowest >= 0.35f,
          "solar tilt limits keep the panels off the slope",
          fmt("range %.0f..%.0f deg (includes the 30 deg start); edge clearance at the limits >= %.2f m", minTilt, maxTilt, lowest));
}

// ---------------------------------------------------------------------------
void testStep8Controls() {
    std::printf("\nSTEP 8 - picking, projection, camera views\n");

    PlantState plant;
    Animation::reset(plant);
    Camera cam;
    cam.lookFrom(Layout::CAMERA_EYE, Layout::CAMERA_TARGET);
    glm::vec3 eye = cam.getPosition();

    Ray toTurbine{eye, glm::normalize(Placement::turbineHub(0) - eye)};
    glm::vec3 boatPos(plant.boatXZ.x, plant.boatWaterHeight + 0.6f, plant.boatXZ.y);
    Ray toBoat{eye, glm::normalize(boatPos - eye)};
    Ray toSky{eye, glm::vec3(0.0f, 1.0f, 0.0f)};
    Selectable a = Picking::pick(toTurbine, plant), b = Picking::pick(toBoat, plant), c = Picking::pick(toSky, plant);
    check(a == Selectable::Turbine1 && b == Selectable::Boat && c == Selectable::None, "picking finds the object under the ray",
          std::string("hub -> ") + Picking::name(a) + ", boat -> " + Picking::name(b) + ", sky -> " + Picking::name(c));

    // The centre pixel's ray points at the camera target, in both projections.
    bool ok = true;
    for (bool ortho : {false, true}) {
        glm::mat4 projection = cam.projection(1216.0f / 900.0f, Layout::VFOV_DEG, ortho);
        Ray r = Picking::rayThroughPixel(608.0, 450.0, 1216, 900, cam.getViewMatrix(), projection);
        glm::vec3 toTarget = glm::normalize(cam.target - eye);
        ok = ok && glm::dot(r.direction, toTarget) > 0.9999f;
    }
    check(ok, "mouse ray works in perspective and ortho", "centre pixel's ray points at the camera target in both");

    // A glide arrives exactly at the preset, and passes through the middle smoothly.
    CameraDirector director;
    CameraViews::goTo(director, cam, CameraView::Turbine, plant, false);
    float startRadius = cam.radius;
    for (int i = 0; i < 60; ++i) CameraViews::update(director, cam, plant, 1.0f / 60.0f);   // 1 s of 1.8 s
    float midRadius = cam.radius;
    for (int i = 0; i < 120; ++i) CameraViews::update(director, cam, plant, 1.0f / 60.0f);
    Camera expected;
    CameraDirector instant;
    CameraViews::goTo(instant, expected, CameraView::Turbine, plant, true);
    check(!director.gliding && glm::length(cam.target - expected.target) < 1e-3f && std::fabs(cam.radius - expected.radius) < 1e-2f &&
          midRadius < startRadius && midRadius > expected.radius,
          "camera glides to a view and stops there", fmt("distance %.0f m -> %.0f m (after 1 s) -> %.1f m", startRadius, midRadius, cam.radius));

    // Camera limit: an eye pushed under the hill is lifted back above it.
    Camera low;
    low.lookFrom(Layout::CAMERA_EYE, Layout::CAMERA_TARGET);
    low.pitch = -60.0f;
    CameraDirector idle;
    CameraViews::update(idle, low, plant, 1.0f / 60.0f);
    glm::vec3 lowEye = low.getPosition();
    check(lowEye.y > terrainHeight(lowEye.x, lowEye.z), "camera never goes under the ground",
          fmt("eye %.1f m above the terrain", lowEye.y - terrainHeight(lowEye.x, lowEye.z)));

    // Particle pool: bounded, frozen when paused.
    ParticleSystem particles;
    EffectsState fx;
    Animation::reset(plant);
    plant.gateOpening = plant.gateTarget = 1.0f;
    plant.boatMode = BoatMode::Manual;
    plant.boatThrottleInput = 1.0f;
    run(plant, 30.0f, &particles, &fx);
    int alive = particles.aliveCount();
    std::vector<glm::vec3> before;
    for (const Particle& p : particles.slots()) before.push_back(p.position);
    Effects::update(particles, fx, plant, 0.0f);   // a paused frame
    bool frozen = true;
    for (size_t i = 0; i < before.size(); ++i) frozen = frozen && particles.slots()[i].position == before[i];
    check(alive <= ParticleSystem::CAPACITY && alive > 0 && frozen && particles.aliveCount() == alive,
          "particle pool is bounded and pauses", fmt("%.0f alive of %.0f slots; a paused frame changes nothing", alive, ParticleSystem::CAPACITY));
}

}

int SelfTest::runAll() {
    failures = 0;
    checks = 0;
    std::printf("Confluence self-test (Steps 1-8)\n");
    testStep1Time();
    testStep2Transforms();
    testStep3And4Shading();
    testStep5Wind();
    testStep6Hydro();
    testStep7Boat();
    testStep8Controls();
    std::printf("\n%d of %d checks passed.%s\n", checks - failures, checks, failures ? "  SOME CHECKS FAILED." : "");
    return failures;
}
