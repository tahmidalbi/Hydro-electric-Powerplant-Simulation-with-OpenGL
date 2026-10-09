#pragma once
#include <glm/glm.hpp>

struct PlantState;
enum class Selectable;

// STEP 8 - Picking: which object is under the mouse?
//
// 1. Turn the mouse position into a RAY:
//      pixel -> normalised device coords (-1..1) -> undo Projection*View
//      (the inverse matrix) at the near and far planes -> origin + direction.
//    This works for perspective AND orthographic views.
// 2. Every selectable object has a box in its OWN local frame (the frame
//    its model matrix builds). Move the ray into that frame with the inverse
//    model matrix, then do a simple axis-aligned "slab" test. A turned or
//    moving object (the boat on the waves) is handled for free.
// 3. Keep the nearest hit in front of the camera.
//
// A picking ray only CHOOSES an object; it does not draw anything (it is
// not ray tracing -- that is Step 9).
struct Ray {
    glm::vec3 origin;
    glm::vec3 direction;   // unit length
};

namespace Picking {
    // px, py: cursor in FRAMEBUFFER pixels (top-left origin).
    Ray rayThroughPixel(double px, double py, int framebufferWidth, int framebufferHeight,
                        const glm::mat4& view, const glm::mat4& projection);

    // Ray against the box [boxMin, boxMax] given in the model's local frame.
    // On a hit, tHit = distance along the WORLD ray.
    bool rayHitsBox(const Ray& ray, const glm::mat4& model, const glm::vec3& boxMin, const glm::vec3& boxMax, float& tHit);

    // The local box and model matrix of a selectable object.
    void selectionBox(Selectable which, const PlantState& plant, glm::mat4& model, glm::vec3& boxMin, glm::vec3& boxMax);

    // Nearest selectable object hit by the ray, or Selectable::None.
    Selectable pick(const Ray& ray, const PlantState& plant);

    const char* name(Selectable which);
}
