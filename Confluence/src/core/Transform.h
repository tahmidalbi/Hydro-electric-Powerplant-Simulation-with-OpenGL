#pragma once
#include <glm/glm.hpp>

// STEP 2 - Transformations, written out by hand.
//
// Every function here builds a 4x4 matrix entry by entry, so you can read the
// maths instead of trusting a library call. glm is only used as the matrix
// TYPE. The self-test (tests/SelfTest.cpp) checks each result against glm's
// own functions.
//
// Conventions (same as OpenGL and GLM):
//   * Points are column vectors: worldPoint = Model * localPoint.
//   * The RIGHTMOST matrix acts first:  T * R * S  means scale, then rotate,
//     then translate.
//   * Angles are always in RADIANS. Use degreesToRadians() at the call site.
//   * glm stores matrices column-major: m[column][row].
//
// Matrix order in one line:
//   clipPoint = Projection * View * Model * localPoint
//   Model     = Translate * Rotate * Scale
namespace Transform {

    constexpr float PI = 3.14159265358979323846f;
    constexpr float degreesToRadians(float degrees) { return degrees * PI / 180.0f; }

    glm::mat4 identity();

    // Move by `offset`.                         | 1 0 0 x |
    //                                           | 0 1 0 y |
    //                                           | 0 0 1 z |
    //                                           | 0 0 0 1 |
    glm::mat4 translate(const glm::vec3& offset);

    // Stretch along each axis.                  | sx 0  0  0 |
    // Never pass 0: a zero scale has no         | 0  sy 0  0 |
    // inverse, so normals break.                | 0  0  sz 0 |
    glm::mat4 scale(const glm::vec3& factors);

    // Turn about one axis through the origin (right-handed: positive angle is
    // counter-clockwise when the axis points at you).
    glm::mat4 rotateX(float angleRad);
    glm::mat4 rotateY(float angleRad);
    glm::mat4 rotateZ(float angleRad);

    // Turn about a chosen point instead of the origin:
    //   Translate(pivot) * rotation * Translate(-pivot)
    // read right to left: move the pivot to the origin, turn, move it back.
    // The pivot itself does not move. Used for the hub and the panel hinge.
    glm::mat4 rotateAboutPivot(const glm::vec3& pivot, const glm::mat4& rotation);

    // Shear: x is pushed sideways in proportion to height y (x' = x + k*y).
    // A box becomes a leaning parallelogram; its volume is unchanged.
    glm::mat4 shearXByY(float k);

    // Mirror across the plane x = planeX. A reflection flips the triangle
    // winding (clockwise <-> counter-clockwise), which matters if back-face
    // culling is on. This project keeps culling off.
    glm::mat4 reflectAcrossPlaneX(float planeX);

    // The matrix that transforms NORMALS correctly:  (M^-1)^T  (upper 3x3).
    // Under non-uniform scale the plain model matrix tilts normals the wrong
    // way; the inverse-transpose keeps them perpendicular to the surface.
    glm::mat3 normalMatrix(const glm::mat4& model);
}
