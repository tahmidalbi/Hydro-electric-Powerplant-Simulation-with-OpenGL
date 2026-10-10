// =============================================================================
//  World.h  -  ONE continuous world for the whole story.
//
//  Every scene of the program looks at this same world, so the water (and
//  later the electricity) really travels from place to place:
//
//     reservoir --> trash-rack cage --> intake gate --> conduit --> penstock
//       --> MAIN INLET VALVE --> spiral casing --> guide vanes --> RUNNER
//       --> draft tube --> river
//     runner --> shaft --> GENERATOR rotor  (mechanical energy)
//     generator --> bus ducts --> STEP-UP TRANSFORMER --> switchyard
//       --> CIRCUIT BREAKER --> transmission line  (electrical energy)
//
//  Cause and effect are simulated: nothing happens before its cause (the
//  casing fills only after the valve opens, the runner turns only when the
//  guide vanes let water through, voltage needs a spinning AND excited rotor,
//  the line is live only after the breaker closes).
//
//  The spillway (flood outlet) is a separate path straight to the river.
// =============================================================================
#pragma once
#include "Dam.h"
#include "Terrain.h"
#include "Powerhouse.h"
#include "Switchyard.h"
#include "SpillwayWater.h"
#include "IntakeStructure.h"
#include "IntakeGate.h"
#include "Debris.h"
#include "MainInletValve.h"
#include "TurbineUnit.h"
#include "Generator.h"
#include "Transformer.h"
#include "TransmissionLine.h"
#include "PlantData.h"
#include "../render/ParticleSystem.h"

// the stations of the story (= scene numbers), used to set the world up when
// you jump straight to a scene
enum Stage { ST_PREVIEW = 1, ST_RACK = 2, ST_GATE = 3, ST_PENSTOCK = 4, ST_RAYTRACE = 5, ST_VALVE = 6,
             ST_TURBINE = 7, ST_SHAFT = 8, ST_GENERATOR = 9, ST_TRANSFORMER = 10, ST_GRID = 11 };

class World {
public:
    Dam dam;
    Terrain terrain;
    Powerhouse powerhouse;
    Switchyard yard;
    SpillwayWater spill;
    IntakeStructure intake;
    IntakeGate gate;
    Debris debris;
    MainInletValve valve;
    TurbineUnit turbine;
    Generator generator;
    Transformer transformer;
    TransmissionLine line;
    ParticleSystem spray, mist, outflow, splash, gateSpray, dust;

    // ---- spillway (flood outlet) ----
    bool  spillOpen = true;
    float spillGate = 1.0f;

    // ---- intake gate + conduit ----
    bool  intakeOpen = true;
    float intakeLift = 0.0f;
    float conduitFront = 0.0f, conduitLevel = 0.0f;

    // ---- penstock ----
    bool  pipeRelease = true;        // false = water waits at the penstock entrance (story pause)
    bool  pipeFeeding = true;
    float pipeFront = 0.0f, pipeTail = 0.0f;
    float xray = 0.0f;
    bool  xrayOn = false;

    // ---- main inlet valve + turbine ----
    bool  valveOpen = true;
    float valveLift = 1.0f;          // 0 closed .. 1 open
    float wheelAngle = 0.0f;
    float casingFront = 0.0f;        // how far the water has filled the spiral casing (m)
    bool  vanesOpen = true;
    float vaneOpening = 1.0f;        // guide vanes 0 closed .. 1 fully open
    float draftFill = 0.0f;          // water leaving through the draft tube (0..1)
    float flowAge = 0.0f;            // seconds the water has been flowing through the runner
    bool  turbineSection = false;    // section view of the turbine
    float rotorAngle = 0.0f, rotorSpeed = 0.0f;

    // ---- generator + transformer + grid ----
    bool  excited = true;
    float excitation = 1.0f;         // field current 0..1
    bool  breakerClosed = true;
    float breaker01 = 1.0f;
    float gridFront = 0.0f;          // how far the energy has travelled along the line (m)
    float fanAngle = 0.0f;
    bool  trafoXrayOn = false;
    float trafoXray = 0.0f;

    // ---- cut-away ----
    bool  waterCut = false;
    bool  nearSideHidden = false;
    float cutLift = 0.0f, cutLiftTarget = 0.0f;
    bool  cameraUnderwater = false;

    // ---- trash ----
    bool  debrisRunning = false;
    int   debrisWave = 0;
    float nextWaveAt = 0.0f;

    float time = 0;

    void build();

    // story states
    void setPreview();               // the finished plant running
    void beginJourney();             // SHOW WATER MOVEMENT: everything shuts down, the water journey restarts
    void prepareRack(bool instant);
    void prepareGate(bool instant);
    void preparePenstock(bool instant);
    void completeUpTo(Stage s);      // jumped straight to a later scene: do every step before it
    void setSpillway(bool open);
    void setIntakeGate(bool open) { intakeOpen = open; }
    void addTrash();
    void restartPenstockFlow();

    void update(float dt);
    void lighting(Lighting& L, const Vec3& camPos);
    void drawOpaque(Renderer& r);
    void drawTransparent(Renderer& r);

    // ---- helpers / measured values for the scenes ----
    float pipeLength() const { return powerhouse.penstock.length; }
    Vec3  pipePoint(float s) const { return powerhouse.penstock.pointAt(s); }
    float pipeSpeedAt(float s) const;           // v = sqrt(v0^2 + 2 g h)
    bool  pipeFull() const { return pipeFront >= pipeLength() - 0.05f; }
    bool  conduitFull() const;
    bool  gateFullyOpen() const { return intakeLift >= gate.maxLift - 0.01f; }
    float conduitLength() const;
    bool  casingFull() const { return casingFront >= turbine.casingLength - 0.1f; }
    float flow01() const;                       // share of full turbine flow (0..1)
    float rpm() const { return Plant::rpmFromVisual(rotorSpeed); }
    float frequency() const { return rpm() * Plant::POLES / 120.0f; }
    float voltageKV() const;
    float powerMW() const;
    float voltage01() const { return voltageKV() / Plant::GEN_KV; }
    static constexpr float WATER_SLOW = 0.6f;

private:
    Mesh resA, resB, resC, resR, cutFaceSide, cutFaceFar, cutFaceBack, mouthFace, river, shafts;
    Mesh conduitTop, conduitFront_;
    std::vector<Vertex> topVerts, frontVerts;
    Rng rng{99};
    float conduitProfile(float d) const;
    void rebuildConduitMesh();
    void emitParticles(float dt);
};
