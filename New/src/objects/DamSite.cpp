#include "DamSite.h"
#include "Materials.h"

using namespace DamDims;

void DamSite::build() {
    dam.build();
    terrain.build();
    powerhouse.build();
    yard.build();
    spill.build();

    MeshBuilder b;
    b.setTransform(translate(Vec3(0, RESERVOIR_Y, -450.0f)));
    b.grid(1400.0f, 896.0f, 8, 8);                // ends at z = -2 (dam face)
    reservoir = b.build();
    b.clear();
    b.setTransform(translate(Vec3(0, RIVER_Y, 372.0f)));
    b.grid(900.0f, 656.0f, 8, 8);                 // from the dam toe (z = 44) downstream
    river = b.build();

    spray.maxParticles = 5000;
    mist.maxParticles = 1500;
    outflow.maxParticles = 1500;
}

void DamSite::reset(bool running) {
    spray.clear();
    mist.clear();
    outflow.clear();
    gatesOpen = running;
    gateOpen = running ? 1.0f : 0.0f;
    if (running) {
        spill.fillInstantly();
        pipeFront = powerhouse.penstock.length + 10.0f;
        pipeTail = -1e3f;
        pipeFeeding = true;
        rotorSpeed = 2.4f;
    } else {
        spill.slugs.clear();
        pipeFront = 0;
        pipeTail = 0;
        pipeFeeding = false;
        rotorSpeed = 0;
    }
}

void DamSite::setWaterRunning(bool on) {
    gatesOpen = on;
    if (on) {
        spill.open();
        if (!pipeFeeding) { pipeFront = 0; pipeTail = -1e3f; pipeFeeding = true; }
    } else {
        spill.close();
        if (pipeFeeding) { pipeFeeding = false; pipeTail = 0; }
    }
}

// water in the penstock speeds up as it drops: v = sqrt(2 g h)
float DamSite::pipeSpeedAt(float s) const {
    float y = powerhouse.penstock.pointAt(std::fmax(s, 0.0f)).y;
    return std::sqrt(4.0f + 2.0f * 9.81f * std::fmax(38.0f - y, 0.0f));
}

void DamSite::update(float dt) {
    time += dt;
    // gates move slowly towards their target (hydraulic hoists take time)
    float target = gatesOpen ? 1.0f : 0.0f;
    gateOpen += clampf(target - gateOpen, -dt * 0.35f, dt * 0.35f);

    spill.update(dt, spray, mist);

    // penstock water front / tail
    const float slow = 0.45f;
    float L = powerhouse.penstock.length;
    pipeFront = std::fmin(pipeFront + pipeSpeedAt(pipeFront) * dt * slow, L + 10.0f);
    if (!pipeFeeding) pipeTail = std::fmin(pipeTail + pipeSpeedAt(pipeTail) * dt * slow, L + 10.0f);

    // turbine spins while water passes through it
    float turbineS = 0;
    for (const PathFrame& f : powerhouse.penstock.frames) if (f.pos.z <= powerhouse.turbineZ) turbineS = f.s;
    bool waterAtTurbine = pipeFront > turbineS && (pipeFeeding || pipeTail < turbineS);
    float targetSpeed = waterAtTurbine ? 2.4f : 0.0f;
    rotorSpeed += (targetSpeed - rotorSpeed) * std::fmin(dt * 0.6f, 1.0f);
    rotorAngle += rotorSpeed * dt;

    // white water leaving the draft tube into the river
    bool outflowing = pipeFront >= L - 0.5f && (pipeFeeding || pipeTail < L - 0.5f);
    if (outflowing) {
        for (int k = 0; k < 6; k++) {
            Particle p;
            float a = rng.range(0, 2 * PI);
            p.pos = Vec3(58, 6.5f, 80) + Vec3(std::cos(a) * 1.4f, std::sin(a) * 1.4f, 0);
            p.vel = Vec3(rng.range(-1.5f, 1.5f), rng.range(0.5f, 3.0f), rng.range(5.0f, 10.0f));
            p.size = rng.range(0.3f, 0.7f);
            p.grow = 1.4f;
            p.maxLife = rng.range(0.7f, 1.4f);
            p.gravity = -9.81f;
            p.drag = 0.8f;
            p.color = Vec3(0.85f, 0.9f, 0.95f);
            p.alpha = 0.7f;
            outflow.emit(p);
        }
    }
    spray.update(dt);
    mist.update(dt);
    outflow.update(dt);
}

void DamSite::drawOpaque(Renderer& r) {
    terrain.draw(r);
    dam.draw(r, gateOpen);
    powerhouse.draw(r, rotorAngle);
    yard.draw(r);
}

void DamSite::drawTransparent(Renderer& r) {
    // reservoir: deep, calm water
    WaterStyle lake;
    lake.mode = 0;
    lake.deep = Vec3(0.01f, 0.05f, 0.08f);
    lake.shallow = Vec3(0.03f, 0.16f, 0.22f);
    lake.alpha = 0.97f;
    lake.ripple = 0.35f;
    lake.rippleStrength = 0.8f;
    r.drawWater(reservoir, Mat4(), lake);

    // river below the dam: flowing, foamy where the spillway and turbine outflow land
    WaterStyle riv = lake;
    riv.shallow = Vec3(0.05f, 0.22f, 0.26f);
    riv.flowSpeed = 0.6f;
    riv.ripple = 0.5f;
    riv.rippleStrength = 1.4f;
    bool spillLanding = spill.reachesBottom();
    riv.foamSpot = Vec4(0.0f, 66.0f, 26.0f, spillLanding ? 1.0f : 0.0f);
    bool turbineOut = pipeFront >= powerhouse.penstock.length - 0.5f && (pipeFeeding || pipeTail < powerhouse.penstock.length);
    riv.foamSpot2 = Vec4(58.0f, 86.0f, 9.0f, turbineOut ? 0.9f : 0.0f);
    riv.foam = 0.12f;
    r.drawWater(river, Mat4(), riv);

    // water in the chutes and in the penstock
    spill.draw(r, time);
    WaterStyle pipeW;
    pipeW.deep = Vec3(0.02f, 0.12f, 0.2f);
    pipeW.shallow = Vec3(0.1f, 0.42f, 0.6f);
    pipeW.alpha = 0.88f;
    pipeW.foam = 0.5f;
    pipeW.flowSpeed = 12.0f;
    pipeW.ripple = 0.8f;
    pipeW.front = pipeFront;
    pipeW.tail = pipeFeeding ? -1e3f : pipeTail;
    pipeW.noseLen = 2.0f;
    powerhouse.penstock.drawWater(r, pipeW);

    spray.draw(r);
    mist.draw(r);
    outflow.draw(r);
}
