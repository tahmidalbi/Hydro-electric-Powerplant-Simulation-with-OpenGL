#include "Clock.h"
#include <algorithm>

void Clock::tick(double nowSeconds) {
    if (previousRealTime < 0.0) previousRealTime = nowSeconds;   // first frame: delta 0

    float delta = (float)(nowSeconds - previousRealTime);
    previousRealTime = nowSeconds;

    // Clamp: never negative, never longer than MAX_DELTA_SECONDS.
    realDeltaSeconds = std::clamp(delta, 0.0f, MAX_DELTA_SECONDS);
    simulationDeltaSeconds = paused ? 0.0f : realDeltaSeconds * timeScale;
}

void Clock::slower() { timeScale = std::max(MIN_TIME_SCALE, timeScale * 0.5f); }
void Clock::faster() { timeScale = std::min(MAX_TIME_SCALE, timeScale * 2.0f); }
