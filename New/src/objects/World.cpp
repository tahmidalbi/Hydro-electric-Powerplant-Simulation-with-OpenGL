#include "World.h"
#include "Materials.h"
#include "Powerhouse.h"

using namespace DamDims;

// Where the intake gate sits: centre of the conduit, floor level, in the shaft.
static const Vec3 GATE_POS(INTAKE_X, CONDUIT_Y0, 1.7f);
static const float CZ0 = 2.7f;                          // conduit water starts just after the gate
static const float CX0 = CONDUIT_X0 + 0.1f, CX1 = CONDUIT_X1 - 0.1f;
static const int   NZ = 60, NX = 10;                    // resolution of the conduit water surface
static const float PIPE_TOP_Y = 39.0f;                  // penstock centre at its entrance
static const float RES_CUT_X0 = -30.0f, RES_CUT_X1 = 52.6f, RES_CUT_Z0 = -90.0f;
static const Vec3  DRAFT_OUTLET(62.5f, 3.0f, 83.0f);    // where the turbine water boils up in the river

// a flat rectangle with one normal (used for the cut faces of the water)
static void face(MeshBuilder& b, Vec3 a, Vec3 c, Vec3 d, Vec3 e, Vec3 n) {
    unsigned int k = (unsigned int)b.verts.size();
    b.addVertex(a, n, Vec2(a.x + a.z, a.y));
    b.addVertex(c, n, Vec2(c.x + c.z, c.y));
    b.addVertex(d, n, Vec2(d.x + d.z, d.y));
    b.addVertex(e, n, Vec2(e.x + e.z, e.y));
    b.tri(k, k + 1, k + 2);
    b.tri(k, k + 2, k + 3);
}

static Mesh waterPlane(float x0, float x1, float z0, float z1, float y) {
    MeshBuilder b;
    b.setTransform(translate(Vec3((x0 + x1) * 0.5f, y, (z0 + z1) * 0.5f)));
    b.grid(x1 - x0, z1 - z0, 6, 6);
    return b.build();
}

void World::build() {
    dam.build();
    terrain.build();
    powerhouse.build();
    yard.build();
    spill.build();
    intake.build();
    gate.build();
    debris.build();
    valve.build();
    turbine.build();
    generator.build();
    Vec3 terminals[3] = {generator.terminal(0), generator.terminal(1), generator.terminal(2)};
    transformer.build(terminals);
    line.build();

    // debris: its local frame (x = flow, y = up) is placed at the trash-rack cage.
    debris.parent = translate(Vec3(INTAKE_X, RESERVOIR_Y - 6.5f, IntakeDims::CAGE_FRONT_Z + 4.0f)) * rotateY(-PI * 0.5f);
    debris.sinkY = PIPE_TOP_Y - (RESERVOIR_Y - 6.5f);
    debris.endX = UPSTREAM_Z - (IntakeDims::CAGE_FRONT_Z + 4.0f);

    // reservoir surface in pieces, so the part next to the intake can be cut away
    resA = waterPlane(-700, 700, -900, RES_CUT_Z0, RESERVOIR_Y);
    resB = waterPlane(-700, RES_CUT_X0, RES_CUT_Z0, UPSTREAM_Z, RESERVOIR_Y);
    resC = waterPlane(RES_CUT_X1, 700, RES_CUT_Z0, UPSTREAM_Z, RESERVOIR_Y);
    resR = waterPlane(RES_CUT_X0, RES_CUT_X1, RES_CUT_Z0, UPSTREAM_Z, RESERVOIR_Y);
    river = waterPlane(-450, 450, 44, 700, RIVER_Y);

    const float yb = 15.0f, yt = RESERVOIR_Y;
    MeshBuilder b;
    face(b, Vec3(RES_CUT_X1, yb, UPSTREAM_Z), Vec3(RES_CUT_X1, yb, RES_CUT_Z0), Vec3(RES_CUT_X1, yt, RES_CUT_Z0), Vec3(RES_CUT_X1, yt, UPSTREAM_Z), Vec3(-1, 0, 0));
    cutFaceSide = b.build();
    b.clear();
    face(b, Vec3(RES_CUT_X0, yb, RES_CUT_Z0), Vec3(RES_CUT_X1, yb, RES_CUT_Z0), Vec3(RES_CUT_X1, yt, RES_CUT_Z0), Vec3(RES_CUT_X0, yt, RES_CUT_Z0), Vec3(0, 0, 1));
    cutFaceFar = b.build();
    b.clear();
    face(b, Vec3(RES_CUT_X0, yb, RES_CUT_Z0), Vec3(RES_CUT_X0, yb, UPSTREAM_Z), Vec3(RES_CUT_X0, yt, UPSTREAM_Z), Vec3(RES_CUT_X0, yt, RES_CUT_Z0), Vec3(1, 0, 0));
    cutFaceBack = b.build();
    b.clear();
    face(b, Vec3(RES_CUT_X1, CONDUIT_Y0, 1.2f), Vec3(RES_CUT_X1, CONDUIT_Y0, UPSTREAM_Z), Vec3(RES_CUT_X1, CONDUIT_Y1, UPSTREAM_Z), Vec3(RES_CUT_X1, CONDUIT_Y1, 1.2f), Vec3(-1, 0, 0));
    mouthFace = b.build();

    // light shafts for the under-water part (direction = sunlight)
    b.clear();
    Rng rs(3);
    Vec3 dir = normalize(Vec3(0.55f, -0.62f, -0.55f));
    for (int i = 0; i < 18; i++) {
        float x = INTAKE_X + rs.range(8, 34) * (i % 2 ? 1.0f : -1.0f), z = rs.range(-70, -20), w = rs.range(1.2f, 4.0f), len = rs.range(26, 44);
        Vec3 top(x, RESERVOIR_Y, z);
        Vec3 side = normalize(Vec3(1, 0, 1 + rs.range(-0.5f, 0.5f)));
        Vec3 a = top - side * w * 0.5f, c = top + side * w * 0.5f;
        unsigned int k = (unsigned int)b.verts.size();
        Vec3 n(0, 0, -1);
        b.addVertex(a, n, Vec2(0, 0));
        b.addVertex(c, n, Vec2(1, 0));
        b.addVertex(c + dir * len, n, Vec2(1, 1));
        b.addVertex(a + dir * len, n, Vec2(0, 1));
        b.tri(k, k + 1, k + 2);
        b.tri(k, k + 2, k + 3);
    }
    shafts = b.build();

    // conduit water: CPU recomputes the surface heights every frame
    topVerts.resize((NZ + 1) * (NX + 1));
    frontVerts.resize((NZ + 1) * 2);
    std::vector<unsigned int> idx, fidx;
    for (int j = 0; j < NX; j++)
        for (int i = 0; i < NZ; i++) {
            unsigned int a = j * (NZ + 1) + i, c = a + NZ + 1;
            idx.insert(idx.end(), {a, c, a + 1, a + 1, c, c + 1});
        }
    for (int i = 0; i < NZ; i++) { unsigned int k = i * 2; fidx.insert(fidx.end(), {k, k + 2, k + 3, k, k + 3, k + 1}); }
    rebuildConduitMesh();
    conduitTop.upload(topVerts, idx, true);
    conduitFront_.upload(frontVerts, fidx, true);

    spray.maxParticles = 3000;
    mist.maxParticles = 600;
    outflow.maxParticles = 1500;
    dust.softNoise = false;
    splash.softNoise = false;
    gateSpray.softNoise = false;
}

// ============================================================================
//  Story states
// ============================================================================
void World::setPreview() {
    spillOpen = true;
    spillGate = 1.0f;
    spill.fillInstantly();
    intakeOpen = true;
    intakeLift = gate.maxLift;
    conduitFront = conduitLength() + 3.0f;
    conduitLevel = 9.3f;
    pipeRelease = true;
    pipeFeeding = true;
    pipeFront = pipeLength();
    pipeTail = -1e3f;
    valveOpen = true;
    valveLift = 1.0f;
    casingFront = turbine.casingLength + 2.0f;
    vanesOpen = true;
    vaneOpening = 1.0f;
    draftFill = 1.0f;
    flowAge = 100.0f;
    turbineSection = false;
    rotorSpeed = Plant::VISUAL_OMEGA;
    excited = true;
    excitation = 1.0f;
    breakerClosed = true;
    breaker01 = 1.0f;
    gridFront = 1e4f;
    trafoXrayOn = false;
    trafoXray = 0;
    xray = 0;
    xrayOn = false;
    waterCut = false;
    nearSideHidden = false;
    cutLift = cutLiftTarget = 0;
    debris.reset();
    debrisRunning = false;
    spray.clear(); mist.clear(); outflow.clear(); splash.clear(); gateSpray.clear(); dust.clear();
}

void World::beginJourney() {
    // To follow the water from the start, the unit is shut down in the real
    // order: breaker opens, field switched off, guide vanes close, main inlet
    // valve closes, intake gate closes.  The waterway drains and the turbine
    // runs down while the camera flies to the reservoir.
    breakerClosed = false;
    excited = false;
    vanesOpen = false;
    valveOpen = false;
    intakeOpen = false;
    pipeRelease = false;
    xrayOn = false;
    trafoXrayOn = false;
    turbineSection = false;
    debris.reset();
    debrisRunning = false;
}

// helper: the waterway is empty and still (used when jumping to a scene)
static void emptyWaterway(World& w) {
    w.intakeOpen = false;
    w.intakeLift = 0;
    w.conduitFront = 0;
    w.conduitLevel = 0;
    w.pipeRelease = false;
    w.pipeFeeding = false;
    w.pipeFront = -5.0f;
    w.pipeTail = 0;
    w.valveOpen = false;
    w.valveLift = 0;
    w.casingFront = 0;
    w.vanesOpen = false;
    w.vaneOpening = 0;
    w.draftFill = 0;
    w.flowAge = 0;
    w.rotorSpeed = 0;
    w.excited = false;
    w.excitation = 0;
    w.breakerClosed = false;
    w.breaker01 = 0;
    w.gridFront = 0;
    w.spillOpen = false;
    w.spillGate = 0;
    w.spill.slugs.clear();
}

// trash that has already been caught (when you jump straight to a later scene)
static void settledTrash(World& w) {
    w.debris.reset();
    ParticleSystem dummy;
    for (int wave = 0; wave < 3; wave++) {
        w.debris.spawnWave(wave);
        for (int k = 0; k < 180; k++) w.debris.update(1.0f / 30.0f, k / 30.0f, dummy);
    }
    for (int k = 0; k < 300; k++) w.debris.update(1.0f / 30.0f, k / 30.0f, dummy);
    w.debrisRunning = false;
}

void World::prepareRack(bool instant) {
    waterCut = true;
    nearSideHidden = true;
    debris.reset();
    debrisRunning = true;
    debrisWave = 0;
    nextWaveAt = time;
    if (instant) {
        emptyWaterway(*this);
        cutLift = cutLiftTarget = 0;
    }
}

void World::prepareGate(bool instant) {
    waterCut = true;
    nearSideHidden = true;
    cutLiftTarget = 1.0f;
    if (instant) {
        emptyWaterway(*this);
        cutLift = 1.0f;
        settledTrash(*this);
    }
}

void World::preparePenstock(bool instant) {
    waterCut = true;
    nearSideHidden = true;
    cutLiftTarget = 1.0f;
    pipeRelease = true;
    if (instant) completeUpTo(ST_PENSTOCK);
}

// Jumping straight to a scene: do (instantly) every step that comes before it.
void World::completeUpTo(Stage s) {
    emptyWaterway(*this);
    waterCut = true;
    nearSideHidden = true;
    cutLift = cutLiftTarget = 1.0f;
    settledTrash(*this);
    xrayOn = false;
    trafoXrayOn = false;
    turbineSection = false;
    // reservoir -> intake gate open -> conduit full
    intakeOpen = true;
    intakeLift = gate.maxLift;
    conduitFront = conduitLength() + 3.0f;
    conduitLevel = 9.3f;
    pipeRelease = true;
    if (s <= ST_PENSTOCK) return;            // the penstock scene starts with an empty pipe
    pipeFeeding = true;                      // penstock full, waiting at the closed valve
    pipeFront = pipeLength();
    pipeTail = -1e3f;
    if (s <= ST_VALVE) return;
    valveOpen = true;                        // valve open, casing full
    valveLift = 1.0f;
    casingFront = turbine.casingLength + 2.0f;
    if (s <= ST_TURBINE) return;
    vanesOpen = true;                        // guide vanes open, runner at speed
    vaneOpening = 1.0f;
    draftFill = 1.0f;
    flowAge = 100.0f;
    rotorSpeed = Plant::VISUAL_OMEGA;
    if (s <= ST_GENERATOR) return;
    excited = true;                          // generator excited, 11 kV at the terminals
    excitation = 1.0f;
}

void World::setSpillway(bool open) {
    spillOpen = open;
    if (open) spill.open(); else spill.close();
}

void World::addTrash() {
    debris.spawnWave(1 + (debrisWave++) % 2);
}

void World::restartPenstockFlow() {
    pipeFeeding = false;
    pipeFront = -5.0f;
    pipeTail = 0;
}

// ============================================================================
//  Physics
// ============================================================================
float World::conduitLength() const { return BULKHEAD_Z - CZ0; }
bool World::conduitFull() const { return conduitFront >= conduitLength() && conduitLevel > 6.0f; }

float World::pipeSpeedAt(float s) const {
    float y = pipePoint(std::fmax(s, 0.0f)).y;
    return std::sqrt(36.0f + 2.0f * Plant::G * std::fmax(PIPE_TOP_Y - y, 0.0f));
}

float World::flow01() const {
    if (!casingFull() || valveLift < 0.9f) return 0.0f;
    return vaneOpening;
}

float World::voltageKV() const {
    return Plant::GEN_KV * excitation * std::fmin(1.0f, rpm() / Plant::RPM);
}

float World::powerMW() const {
    if (breaker01 < 0.5f || voltageKV() < 9.0f) return 0.0f;
    return Plant::powerMW(Plant::Q_UNIT * flow01(), Plant::HEAD);
}

float World::conduitProfile(float d) const {
    if (d > conduitFront || conduitLevel <= 0.005f) return 0.0f;
    float opening = std::fmin(intakeLift, 4.0f);
    float jet = opening * 0.9f * std::exp(-(d - 2.0f) * (d - 2.0f) / 6.0f) * (1.0f - smoothstepf(4.5f, 8.5f, conduitLevel));
    float h = conduitLevel + jet;
    h *= std::sqrt(clampf((conduitFront - d) / 3.0f, 0.0f, 1.0f));
    if (h > 0.1f) h += 0.08f * std::sin(d * 1.3f - time * 6.0f);
    return std::fmin(h, CONDUIT_Y1 - CONDUIT_Y0 - 0.05f);
}

void World::rebuildConduitMesh() {
    const float L = conduitLength();
    for (int j = 0; j <= NX; j++)
        for (int i = 0; i <= NZ; i++) {
            float d = L * i / NZ;
            float x = mixf(CX0, CX1, (float)j / NX);
            float h = conduitProfile(d), hn = conduitProfile(d + 0.25f);
            Vertex& v = topVerts[j * (NZ + 1) + i];
            v.pos = Vec3(x, CONDUIT_Y0 + 0.03f + h, CZ0 + d);
            v.normal = normalize(Vec3(0.0f, 1.0f, -(hn - h) / 0.25f));
            v.uv = Vec2(x - CX0, d);
        }
    for (int i = 0; i <= NZ; i++) {
        float d = L * i / NZ;
        float h = conduitProfile(d);
        frontVerts[i * 2].pos = Vec3(CX0, CONDUIT_Y0 + 0.03f, CZ0 + d);
        frontVerts[i * 2 + 1].pos = Vec3(CX0, CONDUIT_Y0 + 0.03f + h, CZ0 + d);
        frontVerts[i * 2].normal = frontVerts[i * 2 + 1].normal = Vec3(-1, 0, 0);
        frontVerts[i * 2].uv = Vec2(CZ0 + d, 0);
        frontVerts[i * 2 + 1].uv = Vec2(CZ0 + d, h);
    }
}

void World::update(float dt) {
    time += dt;

    // ---- spillway --------------------------------------------------------
    spillGate += clampf((spillOpen ? 1.0f : 0.0f) - spillGate, -dt * 0.35f, dt * 0.35f);
    spill.update(dt, spray, mist);

    // ---- intake gate -------------------------------------------------------
    intakeLift += clampf((intakeOpen ? gate.maxLift : 0.0f) - intakeLift, -dt * 2.0f, dt * 2.0f);

    // ---- conduit: Torricelli jet under the gate, v = sqrt(2 g h) -------------
    const float L = conduitLength();
    if (intakeLift > 0.05f) {
        float jet = std::sqrt(2.0f * Plant::G * (RESERVOIR_Y - PIPE_TOP_Y));
        conduitFront = std::fmin(conduitFront + dt * WATER_SLOW * (2.0f + 0.3f * jet * std::fmin(intakeLift, 3.0f) / 3.0f), L + 3.0f);
        float targetLevel = conduitFront >= L ? 9.3f : std::fmin(1.0f + intakeLift * 0.42f, 4.2f);
        conduitLevel += (targetLevel - conduitLevel) * std::fmin(dt * 0.9f * WATER_SLOW, 1.0f);
    } else {
        conduitLevel = std::fmax(conduitLevel - dt * 0.8f, 0.0f);
        if (conduitLevel <= 0.01f) conduitFront = 0;
    }

    // ---- penstock: fed by the full conduit, ends at the main inlet valve ------
    bool feedNow = conduitFull() && intakeLift > 2.0f && pipeRelease;
    if (feedNow && !pipeFeeding) {
        if (pipeFront < 0.0f || pipeTail >= pipeFront) pipeFront = 0.0f;
        pipeTail = -1e3f;
        pipeFeeding = true;
    }
    if (!feedNow && pipeFeeding) { pipeFeeding = false; pipeTail = 0.0f; }
    const float PL = pipeLength();
    if (pipeFeeding) {
        pipeFront = std::fmin(pipeFront + pipeSpeedAt(pipeFront) * dt * WATER_SLOW, PL);
    } else if (pipeFront >= 0.0f) {
        pipeTail += pipeSpeedAt(pipeTail) * dt * WATER_SLOW;
        if (pipeTail >= pipeFront) { pipeFront = -5.0f; pipeTail = 0.0f; }   // drained
    }

    // ---- main inlet valve -> spiral casing ------------------------------------
    float oldLift = valveLift;
    valveLift += clampf((valveOpen ? 1.0f : 0.0f) - valveLift, -dt * 0.25f, dt * 0.25f);
    wheelAngle += (valveLift - oldLift) * 40.0f;                   // the hand-wheel turns while it moves
    if (valveLift > 0.3f && pipeFull() && pipeFeeding)
        casingFront = std::fmin(casingFront + 7.0f * dt * WATER_SLOW * valveLift, turbine.casingLength + 2.0f);
    else if (!valveOpen || pipeFront < 0.0f)
        casingFront = std::fmax(casingFront - 6.0f * dt, 0.0f);    // casing drains

    // ---- guide vanes -> runner -> draft tube ------------------------------------
    vaneOpening += clampf((vanesOpen ? 1.0f : 0.0f) - vaneOpening, -dt * 0.35f, dt * 0.35f);
    float q = flow01();
    flowAge = q > 0.05f ? flowAge + dt : 0.0f;
    float targetSpeed = Plant::VISUAL_OMEGA * std::fmin(1.0f, q * 1.25f);
    rotorSpeed += (targetSpeed - rotorSpeed) * std::fmin(dt * 0.45f, 1.0f);
    rotorAngle += rotorSpeed * dt;
    draftFill += clampf((q > 0.05f ? 1.0f : 0.0f) - draftFill, -dt * 0.4f, dt * 0.4f);

    // ---- generator, transformer, breaker, line ----------------------------
    excitation += clampf((excited ? 1.0f : 0.0f) - excitation, -dt * 0.6f, dt * 0.6f);
    breaker01 += clampf((breakerClosed ? 1.0f : 0.0f) - breaker01, -dt * 3.0f, dt * 3.0f);
    if (breaker01 > 0.9f && voltageKV() > 9.0f) gridFront = std::fmin(gridFront + 140.0f * dt, line.lineLength + 60.0f);
    else gridFront = 0.0f;
    fanAngle += dt * 14.0f * voltage01();

    // ---- cut-away / x-ray animations -----------------------------------------
    cutLift += clampf(cutLiftTarget - cutLift, -dt * 0.45f, dt * 0.45f);
    xray += clampf((xrayOn ? 1.0f : 0.0f) - xray, -dt * 1.2f, dt * 1.2f);
    trafoXray += clampf((trafoXrayOn ? 1.0f : 0.0f) - trafoXray, -dt * 1.5f, dt * 1.5f);

    // ---- trash arriving at the rack ------------------------------------------
    if (debrisRunning && debrisWave < 3 && time >= nextWaveAt) {
        debris.spawnWave(debrisWave);
        debrisWave++;
        nextWaveAt = time + 2.6f;
    }
    debris.update(dt, time, splash);

    emitParticles(dt);
    rebuildConduitMesh();
    conduitTop.updateVertices(topVerts);
    conduitFront_.updateVertices(frontVerts);
}

void World::emitParticles(float dt) {
    // turbine water boiling up from the draft tube outlet into the river
    if (draftFill > 0.6f) {
        for (int k = 0; k < 6; k++) {
            Particle p;
            p.pos = DRAFT_OUTLET + Vec3(rng.range(-2.5f, 2.5f), 0, rng.range(-1.0f, 2.0f));
            p.vel = Vec3(rng.range(-1.5f, 1.5f), rng.range(1.0f, 3.5f), rng.range(2.0f, 6.0f));
            p.size = rng.range(0.3f, 0.7f);
            p.grow = 1.4f;
            p.maxLife = rng.range(0.7f, 1.4f);
            p.gravity = -9.81f;
            p.drag = 0.8f;
            p.color = Vec3(0.85f, 0.9f, 0.95f);
            p.alpha = 0.7f * draftFill;
            outflow.emit(p);
        }
    }
    // spray where the jet shoots out under the opening intake gate
    if (intakeLift > 0.05f && conduitLevel < 6.0f) {
        for (int k = 0; k < 8; k++) {
            Particle p;
            p.pos = Vec3(rng.range(CX0 + 0.5f, CX1 - 0.5f), CONDUIT_Y0 + rng.range(0.2f, std::fmin(intakeLift, 3.5f)), CZ0 + rng.range(0.0f, 1.5f));
            p.vel = Vec3(rng.range(-0.8f, 0.8f), rng.range(0.5f, 3.5f), rng.range(4.0f, 9.0f));
            p.size = rng.range(0.06f, 0.16f);
            p.maxLife = rng.range(0.4f, 0.9f);
            p.gravity = -9.81f;
            p.color = Vec3(0.88f, 0.94f, 1.0f);
            p.alpha = 0.8f;
            gateSpray.emit(p);
        }
    }
    // floating specks in the water when the camera is under the surface
    if (cameraUnderwater) {
        while (dust.particles.size() < 900) {
            Particle p;
            p.pos = Vec3(INTAKE_X + rng.range(-30, 30), rng.range(18, 55), rng.range(-75, -10));
            p.vel = Vec3(rng.range(-0.1f, 0.1f), rng.range(-0.05f, 0.08f), rng.range(0.05f, 0.25f));
            p.size = rng.range(0.025f, 0.07f);
            p.maxLife = rng.range(20.0f, 40.0f);
            p.life = rng.range(0.5f, 10.0f);
            p.gravity = 0;
            p.color = Vec3(0.75f, 0.9f, 0.95f);
            p.alpha = rng.range(0.4f, 0.9f);
            dust.emit(p);
        }
    }
    spray.update(dt);
    mist.update(dt);
    outflow.update(dt);
    splash.update(dt);
    gateSpray.update(dt);
    dust.update(dt);
}

// ============================================================================
//  Lighting: one daylight for the whole world, a blue under-water light when
//  the camera is below the reservoir surface.
// ============================================================================
void World::lighting(Lighting& L, const Vec3& c) {
    bool inCutRegion = c.x > RES_CUT_X0 && c.x < RES_CUT_X1 && c.z > RES_CUT_Z0;
    cameraUnderwater = c.y < RESERVOIR_Y && c.z < UPSTREAM_Z && !(waterCut && inCutRegion);
    L.waterLevel = RESERVOIR_Y;
    L.causticRect = Vec4(-700, -900, 700, UPSTREAM_Z + 0.4f);
    L.numPoint = 3;
    L.pointPos[0] = Vec3(59, 25, 55);
    L.pointColor[0] = Vec3(8, 7.4f, 6.2f);
    L.pointPos[1] = Vec3(59, 25, 72);
    L.pointColor[1] = Vec3(8, 7.4f, 6.2f);
    L.pointPos[2] = Vec3(53, 11, 64);                 // lamp in the turbine pit
    L.pointColor[2] = Vec3(4, 4.3f, 4.6f);
    if (cameraUnderwater) {
        L.sunDir = normalize(Vec3(-0.35f, 1.0f, 0.35f));
        L.sunColor = Vec3(1.3f, 1.75f, 1.85f);
        L.skyTop = Vec3(0.12f, 0.35f, 0.45f);
        L.skyHorizon = Vec3(0.06f, 0.2f, 0.27f);
        L.ground = Vec3(0.02f, 0.06f, 0.08f);
        L.ambient = 0.85f;
        L.fogColor = Vec3(0.03f, 0.16f, 0.23f);
        L.fogDensity = 0.017f;
        L.skyMode = 2;
        L.underwater = true;
        L.caustics = 1.6f;
        L.exposure = 0.95f;
    } else {
        L.sunDir = normalize(Vec3(-0.55f, 0.62f, 0.55f));
        L.sunColor = Vec3(2.4f, 2.25f, 2.05f);
        L.skyTop = Vec3(0.2f, 0.34f, 0.56f);
        L.skyHorizon = Vec3(0.56f, 0.62f, 0.68f);
        L.ground = Vec3(0.18f, 0.17f, 0.16f);
        L.ambient = 0.55f;
        L.fogColor = Vec3(0.55f, 0.6f, 0.66f);
        L.fogDensity = 0.0016f;
        L.caustics = 0.5f;
        L.exposure = 0.48f;
        // inside the powerhouse hall: a little less exposure (sun comes in through the cut-away)
        bool inHall = c.x > HouseDims::X0 && c.x < HouseDims::X1 && c.z > HouseDims::Z0 && c.z < HouseDims::Z1 && c.y < HouseDims::ROOF_Y;
        if (inHall) { L.exposure = 0.4f; L.ambient = 0.45f; }
    }
}

// ============================================================================
//  Drawing
// ============================================================================
// Simple visibility culling: don't draw what cannot be seen from the camera
// (detailed machines far away, the yard while we are inside the hall ...).
static bool nearHouse(const Vec3& c) { return length(c - Vec3(60, 12, 63)) < 160.0f; }
static bool insideHall(const Vec3& c) {
    return c.x > HouseDims::X0 && c.x < HouseDims::X1 && c.z > HouseDims::Z0 && c.z < HouseDims::Z1 && c.y < HouseDims::ROOF_Y;
}

void World::drawOpaque(Renderer& r) {
    Vec3 c = r.cam.finalEye();
    terrain.draw(r);
    dam.draw(r, spillGate, cutLift);
    intake.draw(r, !nearSideHidden);
    gate.draw(r, translate(GATE_POS), intakeLift);
    powerhouse.draw(r, rotorAngle);
    if (nearHouse(c)) {
        valve.draw(r, valveLift, wheelAngle);
        turbine.draw(r, rotorAngle, vaneOpening, turbineSection);
        generator.draw(r, rotorAngle, excitation, voltage01(), time);
    }
    if (!insideHall(c)) {
        transformer.draw(r, fanAngle, trafoXray);
        yard.draw(r, breaker01);
        line.draw(r);
    }
    if (length(c - Vec3(58, 50, -15)) < 220.0f) debris.draw(r);
}

void World::drawTransparent(Renderer& r) {
    WaterStyle lake;
    lake.mode = 0;
    lake.deep = Vec3(0.01f, 0.05f, 0.08f);
    lake.shallow = Vec3(0.03f, 0.16f, 0.22f);
    lake.alpha = 0.95f;
    lake.ripple = 0.35f;
    lake.rippleStrength = 0.8f;
    lake.surfaceY = RESERVOIR_Y;
    if (waterCut) {
        lake.alpha = 0.8f;
        lake.shallow = Vec3(0.06f, 0.3f, 0.42f);
    }
    r.drawWater(resA, Mat4(), lake);
    r.drawWater(resB, Mat4(), lake);
    r.drawWater(resC, Mat4(), lake);
    if (!waterCut) {
        r.drawWater(resR, Mat4(), lake);
    } else {
        WaterStyle cut;
        cut.mode = 3;
        cut.deep = Vec3(0.02f, 0.12f, 0.24f);
        cut.shallow = Vec3(0.05f, 0.38f, 0.58f);
        cut.alpha = 0.7f;
        cut.surfaceY = RESERVOIR_Y;
        r.drawWater(cutFaceSide, Mat4(), cut);
        r.drawWater(cutFaceFar, Mat4(), cut);
        r.drawWater(cutFaceBack, Mat4(), cut);
        r.drawWater(mouthFace, Mat4(), cut);
    }

    // water rushing through the conduit after the intake gate
    if (conduitFront > 0.1f && conduitLevel > 0.005f) {
        WaterStyle flow;
        flow.mode = 1;
        flow.deep = Vec3(0.03f, 0.16f, 0.25f);
        flow.shallow = Vec3(0.2f, 0.5f, 0.62f);
        flow.alpha = 0.75f;
        flow.foam = conduitLevel > 6.0f ? 0.25f : 0.55f;
        flow.flowSpeed = 9.0f;
        flow.ripple = 0.9f;
        flow.front = conduitFront;
        flow.sheetWidth = CX1 - CX0;
        r.drawWater(conduitTop, Mat4(), flow);
        WaterStyle ff;
        ff.mode = 3;
        ff.deep = Vec3(0.02f, 0.12f, 0.24f);
        ff.shallow = Vec3(0.05f, 0.38f, 0.58f);
        ff.alpha = 0.68f;
        ff.surfaceY = CONDUIT_Y0 + conduitLevel;
        r.drawWater(conduitFront_, Mat4(), ff);
    }

    // river: foamy where the spillway lands and where the turbine water comes out
    WaterStyle riv = lake;
    riv.alpha = 0.95f;
    riv.shallow = Vec3(0.05f, 0.22f, 0.26f);
    riv.flowSpeed = 0.6f;
    riv.ripple = 0.5f;
    riv.rippleStrength = 1.4f;
    riv.surfaceY = RIVER_Y;
    riv.foamSpot = Vec4(0.0f, 66.0f, 26.0f, spill.reachesBottom() ? 1.0f : 0.0f);
    riv.foamSpot2 = Vec4(DRAFT_OUTLET.x, DRAFT_OUTLET.z + 3.0f, 9.0f, 0.9f * draftFill);
    riv.foam = 0.12f;
    r.drawWater(river, Mat4(), riv);

    spill.draw(r, time);

    // penstock water (stops at the main inlet valve)
    if (pipeFront > 0.0f) {
        WaterStyle w;
        w.deep = Vec3(0.02f, 0.13f, 0.24f);
        w.shallow = Vec3(0.1f, 0.42f, 0.62f);
        w.alpha = 0.86f;
        w.foam = 0.5f;
        w.flowSpeed = pipeFull() && valveLift < 0.3f ? 0.3f : 6.0f + pipeSpeedAt(pipeFront) * 0.6f;
        w.ripple = 0.8f;
        w.front = pipeFront;
        w.tail = pipeFeeding ? -1e3f : pipeTail;
        w.noseLen = pipeFull() ? 0.0f : 2.5f;
        w.glow = xray * 1.4f;
        powerhouse.penstock.drawWater(r, w);
    }
    powerhouse.penstock.drawHolo(r, xray);

    // spiral casing water + glowing streamlines through the turbine
    Vec3 cam = r.cam.finalEye();
    float q = flow01();
    float v = voltage01();
    if (nearHouse(cam)) {
        turbine.drawWater(r, casingFront, turbineSection, 0.0f);
        turbine.drawFlowLines(r, q * 1.2f, 2.0f + 6.0f * q, flowAge * 9.0f, turbineSection);
        // electricity: magnetic field + current in the generator
        generator.drawGlow(r, rotorAngle, excitation * std::fmin(1.0f, rotorSpeed / Plant::VISUAL_OMEGA + 0.15f), v, frequency() / Plant::FREQ);
    }
    if (!insideHall(cam)) {                                  // bus ducts, yard, power line
        transformer.drawGlow(r, v, v, frequency() / Plant::FREQ, trafoXray);
        yard.drawGlow(r, v, v * breaker01, 6.0f);
        line.drawGlow(r, gridFront, v * breaker01, 40.0f);
    }

    spray.draw(r);
    mist.draw(r);
    outflow.draw(r);
    splash.draw(r);
    gateSpray.draw(r);
    if (cameraUnderwater) {
        dust.draw(r);
        Material beam;
        beam.pattern = PAT_BEAM;
        beam.emissive = Vec3(0.5f, 0.85f, 0.9f);
        beam.alpha = 0.14f;
        beam.additive = true;
        beam.castShadow = false;
        beam.twoSided = true;
        r.drawMesh(shafts, Mat4(), beam);
    }
}
