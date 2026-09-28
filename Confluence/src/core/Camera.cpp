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
