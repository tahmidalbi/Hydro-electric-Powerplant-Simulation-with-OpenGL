#include "Camera.h"
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <cmath>

glm::vec3 Camera::getPosition() const {
    float yawR = glm::radians(yaw);
    float pitchR = glm::radians(pitch);
    glm::vec3 offset;
    offset.x = radius * std::cos(pitchR) * std::cos(yawR);
    offset.y = radius * std::sin(pitchR);
    offset.z = radius * std::cos(pitchR) * std::sin(yawR);
    return target + offset;
}

glm::mat4 Camera::getViewMatrix() const {
    return glm::lookAt(getPosition(), target, glm::vec3(0.0f, 1.0f, 0.0f));
}

glm::mat4 Camera::projection(float aspect, float verticalFovDeg, bool orthographic) const {
    float halfFov = glm::radians(verticalFovDeg) * 0.5f;
    if (!orthographic)
        return glm::perspective(2.0f * halfFov, aspect, NEAR_PLANE, FAR_PLANE);
    float halfHeight = radius * std::tan(halfFov);
    // The box starts BEHIND the eye (negative near plane). An orthographic
    // camera has no "eye point" really - every ray is parallel - so hills
    // between the orbit eye and the target must not be clipped away. Depth is
    // linear in ortho, so the long box costs no depth precision worth noting.
    return glm::ortho(-halfHeight * aspect, halfHeight * aspect, -halfHeight, halfHeight, -FAR_PLANE, FAR_PLANE);
}

void Camera::lookFrom(const glm::vec3& eye, const glm::vec3& tgt) {
    target = tgt;
    glm::vec3 offset = eye - tgt;
    radius = glm::length(offset);
    pitch = glm::degrees(std::asin(offset.y / radius));
    yaw = glm::degrees(std::atan2(offset.z, offset.x));
}

void Camera::orbit(float dYawDeg, float dPitchDeg) {
    yaw += dYawDeg;
    pitch = std::clamp(pitch + dPitchDeg, -85.0f, 85.0f);
}

void Camera::zoom(float dRadius) {
    radius = std::clamp(radius + dRadius, 5.0f, 400.0f);
}
