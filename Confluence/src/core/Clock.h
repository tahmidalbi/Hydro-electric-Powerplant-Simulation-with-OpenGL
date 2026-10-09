#pragma once

// STEP 1 - Time.
//
// The program has two clocks:
//   real time       - what the wall clock says. The camera and the inspection
//                     lamp use it, so they still move while the plant is paused.
//   simulation time - what the plant "feels". It stops when paused and can run
//                     slower or faster than real time (time scale).
//
// Every frame:
//   realDelta       = currentRealTime - previousRealTime      (clamped)
//   simulationDelta = paused ? 0 : realDelta * timeScale
//
// Moving parts then do   angle += angularSpeed * simulationDelta
// so they travel the same distance in ten seconds at 30 FPS or at 144 FPS.
//
// The Clock never asks GLFW for the time itself: main.cpp passes the time in.
// That keeps this class pure C++ and lets the self-test drive it with fake times.
class Clock {
public:
    // A frame longer than this (window dragged, debugger stop, slow load) is
    // treated as this long, so nothing jumps after a stall.
    static constexpr float MAX_DELTA_SECONDS = 0.1f;
    static constexpr float MIN_TIME_SCALE = 0.125f;
    static constexpr float MAX_TIME_SCALE = 8.0f;

    // Call exactly once per frame with the current real time in seconds.
    void tick(double nowSeconds);

    float realDelta() const { return realDeltaSeconds; }
    float simulationDelta() const { return simulationDeltaSeconds; }

    bool isPaused() const { return paused; }
    float getTimeScale() const { return timeScale; }

    void togglePause() { paused = !paused; }
    void setPaused(bool value) { paused = value; }
    void slower();   // halves the time scale
    void faster();   // doubles the time scale
    void resetTimeScale() { timeScale = 1.0f; }

private:
    double previousRealTime = -1.0;   // < 0 means "no frame yet"
    float realDeltaSeconds = 0.0f;
    float simulationDeltaSeconds = 0.0f;
    bool paused = false;
    float timeScale = 1.0f;
};
