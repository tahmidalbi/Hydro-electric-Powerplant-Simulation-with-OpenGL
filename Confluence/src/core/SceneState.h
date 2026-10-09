#pragma once
#include <glm/glm.hpp>
#include "../objects/Layout.h"

// STEP 1 - State.
//
// "State" = the numbers that change while the program runs. Geometry (the
// meshes) is built once at start-up; drawing only READS these numbers to place
// the parts. Only the update step (systems/*.cpp) and the input step
// (input/Input.cpp) WRITE them. A draw call never advances animation.

enum class BoatMode { Patrol = 0, Manual = 1 };

// ---------------------------------------------------------------------------
// Moving parts of the plant (advanced by simulation time).
// ---------------------------------------------------------------------------
struct PlantState {
    double simulationTimeSeconds = 0.0;

    // ---- STEP 5: wind, turbines, clouds -----------------------------------
    // Q/E set the TARGET wind; the actual wind eases towards it, so the rotors
    // never jump. Units: metres per second.
    float windTargetMetersPerSecond = 9.0f;
    float windMetersPerSecond = 9.0f;
    float rotorAngleRad[2] = {0.35f, 1.35f};       // start pose = the old fixed pose
    float rotorSpeedRadPerSecond[2] = {0.0f, 0.0f}; // set from the wind by Animation::reset
    float cloudDriftMeters = 0.0f;                  // how far the clouds have travelled

    // ---- STEP 6: gate, flow, water wheel -----------------------------------
    // 0 = shut, 1 = fully raised. The gate moves from `gateOpening` towards
    // `gateTarget` at a limited speed (real gates are slow).
    float gateTarget = Layout::GATE_OPENING;
    float gateOpening = Layout::GATE_OPENING;
    bool gateAutoDemo = false;                       // J: cycles the gate by itself
    float wheelAngleRad = 0.3f;
    float wheelSpeedRadPerSecond = 0.0f;             // set from the flow by Animation::reset

    // ---- STEP 7: boat on the reservoir --------------------------------------
    BoatMode boatMode = BoatMode::Patrol;
    glm::vec2 boatXZ{Layout::BOAT_POS.x, Layout::BOAT_POS.z};
    float boatHeadingRad = 1.5708f;                  // 0 = bow towards +z; pi/2 = towards +x
    float boatSpeed = 0.0f;                          // m/s along the bow (negative = astern)
    float boatThrottleInput = 0.0f;                  // W/S held: -1 .. 1 (manual mode)
    float boatSteerInput = 0.0f;                     // A/D held: -1 .. 1 (manual mode)
    bool boatBlockedByShore = false;                 // true while the next step would beach it
    // Results of floating on the waves (written by BoatSystem, read by drawing):
    float boatWaterHeight = Layout::LAKE_LEVEL;
    float boatPitchRad = 0.0f;
    float boatRollRad = 0.0f;

    // ---- STEP 7: solar tracker -------------------------------------------------
    float solarTiltDeg = 30.0f;                      // Z/C: panels turn about their hinge
};

// ---------------------------------------------------------------------------
// How the frame is drawn (changed by keys, never by time).
// ---------------------------------------------------------------------------
enum class ShadingMode { Flat = 0, Gouraud = 1, Phong = 2 };

// Which lighting terms are added up. The values are bit masks the shader
// tests with  (uTermMask & 1) ambient, (& 2) diffuse, (& 4) specular.
enum class LightingTerms { All = 7, AmbientOnly = 1, DiffuseOnly = 2, SpecularOnly = 4 };

// Which of the three lights are switched on.
enum class LightSet { All = 0, SunOnly = 1, FillOnly = 2, LampOnly = 3 };

enum class GalleryDetail { Low = 0, Medium = 1, High = 2 };

// STEP 8: the objects a right-click (or Tab) can select.
enum class Selectable { None = 0, Turbine1, Turbine2, Gate, Wheel, Solar, Boat, Count };

struct ViewSettings {
    ShadingMode shading = ShadingMode::Phong;
    LightingTerms terms = LightingTerms::All;
    LightSet lights = LightSet::All;
    bool useBlinn = false;        // B: Blinn-Phong half vector instead of Phong reflection vector
    bool wireframe = false;       // X: draw triangle edges only
    bool showNormals = false;     // N: colour = normal * 0.5 + 0.5

    bool galleryActive = false;                  // F6: graphics comparison gallery
    GalleryDetail galleryDetail = GalleryDetail::Low;   // + / -
    bool galleryWrongMatrixOrder = false;        // O: S*R*T instead of T*R*S
    bool galleryWrongNormalMatrix = false;       // M: model matrix instead of (M^-1)^T for normals

    // STEP 8
    bool orthographic = false;                   // P: parallel projection
    Selectable selected = Selectable::None;      // right-click / Tab
    bool hudVisible = true;                      // Backspace: clean screenshot view
    bool helpVisible = false;                    // F11: full key list on screen
};

// ---------------------------------------------------------------------------
// The movable inspection lamp (Step 3). One position per view, so moving the
// lamp in the gallery does not lose its place in the main scene.
// ---------------------------------------------------------------------------
struct LampState {
    glm::vec3 scenePosition{0.0f};
    glm::vec3 galleryPosition{0.0f};
};

struct SceneState {
    PlantState plant;
    ViewSettings view;
    LampState lamp;

    glm::vec3& activeLampPosition() { return view.galleryActive ? lamp.galleryPosition : lamp.scenePosition; }
    const glm::vec3& activeLampPosition() const { return view.galleryActive ? lamp.galleryPosition : lamp.scenePosition; }
};
