#pragma once
#include <glm/glm.hpp>

// Where each fixed object stands in the world (its ROOT matrix).
//
// Before Step 8 these were private to Scene.cpp. Now three different parts of
// the program need the same answers -- the scene (to draw), the camera views
// (to look at things) and picking (to test the mouse ray against things) --
// so they live in one place instead of being copied.
//
// Moving parts (rotor angle, gate, boat ...) are NOT here: they come from
// PlantState every frame. These roots never change.
namespace Placement {
    // Turbine `index` 0 or 1, standing on the ground, rotor facing the viewer.
    glm::mat4 turbineRoot(int index);
    // World position of turbine `index`'s rotor hub (its pivot).
    glm::vec3 turbineHub(int index);

    glm::mat4 wheelRoot();    // water-wheel axle centre (dam frame -> wheel mount)
    glm::mat4 solarRoot();    // centre of the solar array, on the ground
}
