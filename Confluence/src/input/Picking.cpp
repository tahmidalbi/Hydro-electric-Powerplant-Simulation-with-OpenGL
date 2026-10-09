#include "Picking.h"
#include "../core/SceneState.h"
#include "../objects/Dam.h"
#include "../objects/Layout.h"
#include "../objects/Placement.h"
#include "../objects/SolarArray.h"
#include "../systems/BoatSystem.h"
#include <algorithm>
#include <cmath>

Ray Picking::rayThroughPixel(double px, double py, int w, int h, const glm::mat4& view, const glm::mat4& projection) {
    // Pixel -> normalised device coordinates. Window y grows DOWN, NDC y grows UP.
    float x = 2.0f * (float)px / (float)w - 1.0f;
    float y = 1.0f - 2.0f * (float)py / (float)h;

    glm::mat4 inverse = glm::inverse(projection * view);
    glm::vec4 nearPoint = inverse * glm::vec4(x, y, -1.0f, 1.0f);
    glm::vec4 farPoint  = inverse * glm::vec4(x, y,  1.0f, 1.0f);
    glm::vec3 a = glm::vec3(nearPoint) / nearPoint.w;   // divide by w: undo the perspective divide
    glm::vec3 b = glm::vec3(farPoint) / farPoint.w;
    return {a, glm::normalize(b - a)};
}

bool Picking::rayHitsBox(const Ray& ray, const glm::mat4& model, const glm::vec3& lo, const glm::vec3& hi, float& tHit) {
    // Into the box's own frame. The direction is NOT re-normalised, so a
    // parameter t means the same point on the ray in both frames.
    glm::mat4 toLocal = glm::inverse(model);
    glm::vec3 o = glm::vec3(toLocal * glm::vec4(ray.origin, 1.0f));      // a point: w = 1
    glm::vec3 d = glm::vec3(toLocal * glm::vec4(ray.direction, 0.0f));   // a direction: w = 0

    // Slab test: for each axis, the ray is inside the slab between tNear and tFar.
    // The box is hit if all three intervals overlap.
    float tEnter = -1e30f, tExit = 1e30f;
    for (int axis = 0; axis < 3; ++axis) {
        if (std::fabs(d[axis]) < 1e-9f) {                 // parallel to this slab
            if (o[axis] < lo[axis] || o[axis] > hi[axis]) return false;
            continue;
        }
        float t1 = (lo[axis] - o[axis]) / d[axis];
        float t2 = (hi[axis] - o[axis]) / d[axis];
        if (t1 > t2) std::swap(t1, t2);
        tEnter = std::max(tEnter, t1);
        tExit = std::min(tExit, t2);
        if (tEnter > tExit) return false;
    }
    if (tExit < 0.0f) return false;                       // the box is behind the camera
    tHit = tEnter > 0.0f ? tEnter : tExit;                // (camera inside the box: use the exit)
    return true;
}

void Picking::selectionBox(Selectable which, const PlantState& plant, glm::mat4& model, glm::vec3& lo, glm::vec3& hi) {
    switch (which) {
        case Selectable::Turbine1:
        case Selectable::Turbine2:
            model = Placement::turbineRoot(which == Selectable::Turbine1 ? 0 : 1);
            lo = {-6.5f, 0.0f, -2.5f};  hi = {6.5f, 18.7f, 3.2f};        // tower + rotor disc
            return;
        case Selectable::Gate:
            model = Layout::damRoot();
            lo = {Dam::spillwayStart(), 11.0f, -1.5f};  hi = {Dam::spillwayEnd(), 22.5f, 3.5f};
            return;
        case Selectable::Wheel:
            model = Placement::wheelRoot();
            lo = {-4.3f, -4.3f, -1.6f};  hi = {4.3f, 4.3f, 1.6f};
            return;
        case Selectable::Solar:
            model = SolarArray::panelMatrix(Placement::solarRoot(), plant.solarTiltDeg);
            lo = {-7.2f, -0.2f, -0.6f};  hi = {7.2f, 4.5f, 0.3f};        // the tilted panel plane
            return;
        case Selectable::Boat:
            model = BoatSystem::modelMatrix(plant);
            lo = {-1.0f, -0.3f, -2.2f};  hi = {1.0f, 1.6f, 3.2f};
            return;
        default:
            model = glm::mat4(1.0f);
            lo = hi = glm::vec3(0.0f);
            return;
    }
}

Selectable Picking::pick(const Ray& ray, const PlantState& plant) {
    Selectable best = Selectable::None;
    float bestT = 1e30f;
    for (int i = 1; i < (int)Selectable::Count; ++i) {
        Selectable s = (Selectable)i;
        glm::mat4 model;
        glm::vec3 lo, hi;
        selectionBox(s, plant, model, lo, hi);
        float t;
        if (rayHitsBox(ray, model, lo, hi, t) && t < bestT) { bestT = t; best = s; }
    }
    return best;
}

const char* Picking::name(Selectable which) {
    switch (which) {
        case Selectable::None:     return "NOTHING";
        case Selectable::Turbine1: return "WIND TURBINE 1";
        case Selectable::Turbine2: return "WIND TURBINE 2";
        case Selectable::Gate:     return "SPILLWAY GATES";
        case Selectable::Wheel:    return "WATER WHEEL";
        case Selectable::Solar:    return "SOLAR TRACKER";
        case Selectable::Boat:     return "BOAT";
        default:                   return "?";
    }
}
