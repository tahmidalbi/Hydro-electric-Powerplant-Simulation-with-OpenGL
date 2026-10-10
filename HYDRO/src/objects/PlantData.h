// =============================================================================
//  PlantData.h  -  ONE data sheet for the whole power plant, so every number
//  shown on screen (speed, flow, power, rpm, voltage ...) agrees.
//
//      P = rho * g * Q * H * efficiency
//        = 1000 * 9.81 * 180 * 53 * 0.92  =  86 MW   (one turbine unit)
//      f = poles * rpm / 120  =  40 * 150 / 120  =  50 Hz
// =============================================================================
#pragma once
#include <cmath>

namespace Plant {
    const float G = 9.81f, RHO = 1000.0f;
    const float RESERVOIR_Y = 56.0f, TAILWATER_Y = 3.0f;
    const float HEAD = RESERVOIR_Y - TAILWATER_Y;    // 53 m gross head
    const float Q_UNIT = 180.0f;                     // m3/s through one turbine at full gate
    const float EFFICIENCY = 0.92f;
    const float RPM = 150.0f;                        // turbine + generator speed
    const int   POLES = 40;                          // generator rotor poles
    const float FREQ = 50.0f;                        // Hz
    const float GEN_KV = 11.0f;                      // generator voltage
    const float GRID_KV = 220.0f;                    // after the step-up transformer
    const float RATED_MW = 86.0f;

    inline float powerMW(float q, float h) { return RHO * G * q * h * EFFICIENCY / 1e6f; }
    inline float freeFallSpeed(float h) { return std::sqrt(2.0f * G * h); }
    inline float torqueMNm(float mw, float rpm) { return rpm > 1.0f ? mw * 1e6f / (rpm * 2.0f * 3.14159265f / 60.0f) / 1e6f : 0.0f; }

    // the 3D rotor turns slower than the real 150 rpm so the eye can follow it
    const float VISUAL_OMEGA = 2.4f;                 // rad/s on screen at full speed
    inline float rpmFromVisual(float w) { return w / VISUAL_OMEGA * RPM; }
}
