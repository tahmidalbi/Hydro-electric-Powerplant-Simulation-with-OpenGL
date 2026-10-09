#pragma once
#include <glm/glm.hpp>

// Spherical-coordinate orbit camera: mouse-drag changes yaw/pitch, scroll
// changes radius. Specular highlights are view-dependent, so orbiting is
// how you prove the Phong shader is actually per-fragment.
class Camera {
public:
    glm::mat4 getViewMatrix() const;
    glm::vec3 getPosition() const;

    // STEP 8: perspective (things shrink with distance) or orthographic
    // (parallel lines stay parallel; sizes can be compared). In orthographic
    // mode the visible height equals what perspective shows at the target,
    // so switching keeps the target the same size on screen.
    glm::mat4 projection(float aspect, float verticalFovDeg, bool orthographic) const;

    // Places the camera at `eye` orbiting around `target`.
    void lookFrom(const glm::vec3& eye, const glm::vec3& target);
    void orbit(float dYawDeg, float dPitchDeg);
    void zoom(float dRadius);

    glm::vec3 target{0.0f};
    float radius = 30.0f, yaw = 90.0f, pitch = 20.0f;

    static constexpr float NEAR_PLANE = 0.3f;
    static constexpr float FAR_PLANE = 1500.0f;
};
