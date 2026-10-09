#include "Transform.h"
#include <cmath>

// Reminder: glm is column-major, so  m[c][r]  is column c, row r.
// The "picture" comments show the matrix the way it is written on paper.

glm::mat4 Transform::identity() {
    glm::mat4 m(0.0f);
    m[0][0] = 1.0f;
    m[1][1] = 1.0f;
    m[2][2] = 1.0f;
    m[3][3] = 1.0f;
    return m;
}

glm::mat4 Transform::translate(const glm::vec3& offset) {
    glm::mat4 m = identity();
    // The offset lives in the 4th column, so it is added to every point
    // (points have w = 1) but ignored by directions (w = 0).
    m[3][0] = offset.x;
    m[3][1] = offset.y;
    m[3][2] = offset.z;
    return m;
}

glm::mat4 Transform::scale(const glm::vec3& factors) {
    glm::mat4 m = identity();
    m[0][0] = factors.x;
    m[1][1] = factors.y;
    m[2][2] = factors.z;
    return m;
}

glm::mat4 Transform::rotateX(float a) {
    // | 1  0   0  |      y' = c*y - s*z
    // | 0  c  -s  |      z' = s*y + c*z
    // | 0  s   c  |
    const float c = std::cos(a), s = std::sin(a);
    glm::mat4 m = identity();
    m[1][1] = c;  m[2][1] = -s;
    m[1][2] = s;  m[2][2] = c;
    return m;
}

glm::mat4 Transform::rotateY(float a) {
    // |  c  0  s |       x' =  c*x + s*z
    // |  0  1  0 |       z' = -s*x + c*z
    // | -s  0  c |
    const float c = std::cos(a), s = std::sin(a);
    glm::mat4 m = identity();
    m[0][0] = c;   m[2][0] = s;
    m[0][2] = -s;  m[2][2] = c;
    return m;
}

glm::mat4 Transform::rotateZ(float a) {
    // | c -s  0 |        x' = c*x - s*y
    // | s  c  0 |        y' = s*x + c*y
    // | 0  0  1 |
    const float c = std::cos(a), s = std::sin(a);
    glm::mat4 m = identity();
    m[0][0] = c;  m[1][0] = -s;
    m[0][1] = s;  m[1][1] = c;
    return m;
}

glm::mat4 Transform::rotateAboutPivot(const glm::vec3& pivot, const glm::mat4& rotation) {
    return translate(pivot) * rotation * translate(-pivot);
}

glm::mat4 Transform::shearXByY(float k) {
    // | 1 k 0 |          x' = x + k*y
    // | 0 1 0 |
    // | 0 0 1 |
    glm::mat4 m = identity();
    m[1][0] = k;
    return m;
}

glm::mat4 Transform::reflectAcrossPlaneX(float planeX) {
    // Move the mirror plane to x = 0, flip x, move back.
    return translate({planeX, 0.0f, 0.0f}) * scale({-1.0f, 1.0f, 1.0f}) * translate({-planeX, 0.0f, 0.0f});
}

glm::mat3 Transform::normalMatrix(const glm::mat4& model) {
    // Only the upper 3x3 matters for directions (no translation).
    // glm::inverse is used here because a hand-written 3x3 inverse adds
    // nothing to the lesson; the rule is still "inverse, then transpose".
    return glm::transpose(glm::inverse(glm::mat3(model)));
}
