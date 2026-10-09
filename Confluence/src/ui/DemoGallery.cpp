#include "DemoGallery.h"
#include "../core/Material.h"
#include "../core/MeshLibrary.h"
#include "../core/Render.h"
#include "../core/SceneState.h"
#include "../core/Shader.h"
#include "../core/Transform.h"
#include "../objects/Turbine.h"
#include <cmath>

namespace {

using glm::vec3;
using glm::mat4;
using namespace Transform;

// Where each exhibit stands. Back row z = -14 (Step 2), front row z = +8 (Step 4).
// The camera looks down on both rows so the front row never hides the back row.
constexpr float BACK_Z = -14.0f;
constexpr float FRONT_Z = 8.0f;

const vec3 TURBINE_BASE(-20.0f, 0.0f, BACK_Z);
const vec3 GATE_BASE(-9.0f, 0.0f, BACK_Z);
const vec3 ORDER_ORIGIN(-2.0f, 0.0f, BACK_Z);   // the post stands at ORDER_ORIGIN + (2, 0, 0)
const vec3 HINGE_POINT(8.0f, 3.0f, BACK_Z);
const vec3 SHEAR_BASE(14.0f, 0.0f, BACK_Z);
const vec3 L_SHAPE_BASE(18.6f, 0.0f, BACK_Z);
constexpr float MIRROR_PLANE_X = 20.5f;

const vec3 SPHERE_CENTRE(-15.0f, 3.0f, FRONT_Z);
const vec3 CYLINDER_CENTRE(-7.0f, 2.8f, FRONT_Z);
const vec3 PATCH_CENTRE(3.0f, 0.05f, FRONT_Z);
constexpr float PATCH_SIZE = 9.0f;
const vec3 ELLIPSOID_CENTRE(15.0f, 3.0f, FRONT_Z);

const vec3 CAMERA_EYE(0.0f, 26.0f, 44.0f);
const vec3 CAMERA_TARGET(0.0f, 2.0f, -4.0f);

// 1. HIERARCHY. Only the root is animated; drawTurbine() builds every child
//    as "root * local offset", so the whole turbine follows.
void drawHierarchyTurbine(const Shader& shader, const MeshLibrary& m, float t) {
    mat4 root = translate(TURBINE_BASE + vec3(2.5f * std::sin(0.4f * t), 0.0f, 0.0f))
              * rotateY(0.45f * std::sin(0.3f * t));
    drawTurbine(shader, m, root, 1.5f * t);
}

// 2. GATE IN ITS SLOT. The frame turns; the gate only ever moves along the
//    frame's LOCAL y axis, so it stays in the slot whichever way the frame faces.
//      gate = frameRoot * gateSlot * Translate(0, opening, 0)
void drawGateInSlot(const Shader& shader, const MeshLibrary& m, float t) {
    mat4 frameRoot = translate(GATE_BASE) * rotateY(0.5f + 0.35f * std::sin(0.25f * t));

    for (float side : {-2.4f, 2.4f})   // two posts
        drawMesh(shader, m.cube, frameRoot * translate({side, 3.5f, 0.0f}) * scale({0.5f, 7.0f, 0.6f}), Materials::WHITE_PAINT);
    drawMesh(shader, m.cube, frameRoot * translate({0.0f, 7.25f, 0.0f}) * scale({5.3f, 0.5f, 0.6f}), Materials::WHITE_PAINT);
    drawMesh(shader, m.cube, frameRoot * translate({0.0f, 0.25f, 0.0f}) * scale({5.3f, 0.5f, 1.0f}), Materials::WHITE_PAINT);

    float opening = 0.5f + 0.5f * std::sin(0.8f * t);                  // 0 shut .. 1 open
    mat4 gateSlot = frameRoot * translate({0.0f, 0.5f, 0.0f});         // bottom of the slot
    mat4 gate = gateSlot * translate({0.0f, 1.25f + 4.0f * opening, 0.0f});
    drawMesh(shader, m.cube, gate * scale({4.2f, 2.5f, 0.3f}), Materials::STEEL);
}

// 3. MATRIX ORDER. Same three matrices, two orders, inside the exhibit's own
//    frame (origin = ORDER_ORIGIN on the floor; the gold post stands 2 units
//    to its right).
//    Correct  T * R * S : scale the bar, spin it about its own centre, then
//                         move it onto the post. It spins in place.
//    Wrong    S * R * T : move first, so the rotation swings the bar around
//                         the exhibit ORIGIN (not the post), and the scale then
//                         stretches that circle into a wide ellipse and skews
//                         the bar. It wanders off its post.
void drawMatrixOrder(const Shader& shader, const MeshLibrary& m, float t, bool wrongOrder) {
    const mat4 exhibit = translate(ORDER_ORIGIN);
    drawMesh(shader, m.cylinder, exhibit * translate({2.0f, 1.25f, 0.0f}) * scale({0.3f, 2.5f, 0.3f}), Materials::GALLERY_GOLD);
    drawMesh(shader, m.cube, exhibit * scale({0.4f, 0.1f, 0.4f}), Materials::GALLERY_GOLD);   // marks the exhibit origin

    mat4 T = translate({2.0f, 3.0f, 0.0f});   // onto the top of the post
    mat4 R = rotateY(0.9f * t);
    mat4 S = scale({3.0f, 0.5f, 0.5f});
    mat4 local = wrongOrder ? S * R * T : T * R * S;
    drawMesh(shader, m.cube, exhibit * local, Materials::GALLERY_GREEN);
}

// 4. PIVOT. The panel turns about its hinge line, not about its own centre:
//      Translate(hinge) * RotateX(tilt) * Translate(-hinge)
void drawHingedPanel(const Shader& shader, const MeshLibrary& m, float t) {
    drawMesh(shader, m.cylinder, translate({HINGE_POINT.x, HINGE_POINT.y * 0.5f, HINGE_POINT.z}) * scale({0.3f, HINGE_POINT.y, 0.3f}), Materials::STEEL);
    drawMesh(shader, m.cylinder, translate(HINGE_POINT) * rotateZ(degreesToRadians(90.0f)) * scale({0.25f, 4.6f, 0.25f}), Materials::STEEL);

    float tilt = 0.7f * std::sin(0.6f * t) - 0.3f;
    mat4 aboutHinge = rotateAboutPivot(HINGE_POINT, rotateX(tilt));
    mat4 restPose = translate(HINGE_POINT + vec3(0.0f, 1.6f, 0.0f)) * scale({4.2f, 3.2f, 0.15f});
    drawMesh(shader, m.cube, aboutHinge * restPose, Materials::POLISHED_BLUE);
}

// 5. SHEAR. x is pushed sideways in proportion to height; the base stays put.
void drawShearedBox(const Shader& shader, const MeshLibrary& m) {
    drawMesh(shader, m.cube, translate(SHEAR_BASE) * shearXByY(0.45f) * translate({0.0f, 2.5f, 0.0f}) * scale({1.6f, 5.0f, 1.6f}),
             Materials::GALLERY_GOLD);
}

// 6. REFLECTION. The same L-shape drawn twice; the second copy goes through
//    a mirror matrix (x -> -x about the plane). An L is not symmetric, so the
//    copy is visibly a mirror image, not a rotation.
void drawLShape(const Shader& shader, const MeshLibrary& m, const mat4& placement) {
    drawMesh(shader, m.cube, placement * translate({0.0f, 2.0f, 0.0f}) * scale({0.7f, 4.0f, 0.7f}), Materials::GALLERY_GREEN);
    drawMesh(shader, m.cube, placement * translate({0.7f, 0.35f, 0.0f}) * scale({1.9f, 0.7f, 0.7f}), Materials::GALLERY_GREEN);
    drawMesh(shader, m.cube, placement * translate({0.0f, 4.3f, 0.0f}) * scale({0.9f, 0.6f, 0.9f}), Materials::GALLERY_GOLD);
}

void drawReflection(const Shader& shader, const MeshLibrary& m) {
    drawMesh(shader, m.cube, translate({MIRROR_PLANE_X, 2.25f, BACK_Z}) * scale({0.08f, 4.5f, 2.4f}), Materials::STEEL);
    mat4 original = translate(L_SHAPE_BASE);
    drawLShape(shader, m, original);
    drawLShape(shader, m, reflectAcrossPlaneX(MIRROR_PLANE_X) * original);
}

// 10. NORMAL MATRIX. A sphere stretched 3:1:1 (non-uniform scale).
//     Correct normals: (M^-1)^T.  Wrong (M key): the model matrix itself,
//     which tilts the normals towards the stretched axis -> the lighting
//     no longer matches the shape (highlight and shading slide sideways).
void drawStretchedEllipsoid(const Shader& shader, const MeshLibrary& m, bool wrongNormalMatrix) {
    mat4 model = translate(ELLIPSOID_CENTRE) * rotateZ(0.5f) * scale({6.0f, 2.0f, 2.0f});
    glm::mat3 normals = wrongNormalMatrix ? glm::mat3(model) : normalMatrix(model);
    drawMeshWithNormalMatrix(shader, m.sphere, model, normals, Materials::POLISHED_RED);
}

void drawPlinth(const Shader& shader, const MeshLibrary& m, const vec3& centre) {
    drawMesh(shader, m.cube, translate({centre.x, 0.25f, centre.z}) * scale({2.6f, 0.5f, 2.6f}), Materials::WHITE_PAINT);
}

}

DemoGallery::DetailLevel::DetailLevel(const char* levelName, int sphereStacks, int sphereSlices, int cylinderSegments, int patchCells)
    : name(levelName),
      sphere(sphereData(sphereStacks, sphereSlices)),
      cylinder(cylinderData(cylinderSegments)),
      patch(gridData(patchCells)) {}

DemoGallery::DemoGallery() : floor(gridData(40)) {
    // LOW is deliberately coarse: big triangles make the Gouraud/Phong
    // difference obvious. HIGH shows that more triangles help Gouraud, but
    // only by sampling more often -- it does not turn Gouraud into Phong.
    levels.reserve(3);
    levels.emplace_back("LOW",    6,  8,  6,  1);
    levels.emplace_back("MEDIUM", 12, 16, 12, 4);
    levels.emplace_back("HIGH",   32, 48, 48, 32);
}

void DemoGallery::draw(const Shader& shader, const MeshLibrary& m, const SceneState& state) const {
    const float t = (float)state.plant.simulationTimeSeconds;
    const ViewSettings& view = state.view;
    const DetailLevel& level = levels[(int)view.galleryDetail];

    drawMesh(shader, floor, scale({80.0f, 1.0f, 50.0f}), Materials::GALLERY_FLOOR);

    // ---- Back row: Step 2, transformations ----
    drawHierarchyTurbine(shader, m, t);
    drawGateInSlot(shader, m, t);
    drawMatrixOrder(shader, m, t, view.galleryWrongMatrixOrder);
    drawHingedPanel(shader, m, t);
    drawShearedBox(shader, m);
    drawReflection(shader, m);

    // ---- Front row: Step 4, shading comparison ----
    drawPlinth(shader, m, SPHERE_CENTRE);
    drawMesh(shader, level.sphere, translate(SPHERE_CENTRE) * scale(vec3(5.0f)), Materials::POLISHED_RED);

    drawPlinth(shader, m, CYLINDER_CENTRE);
    drawMesh(shader, level.cylinder, translate(CYLINDER_CENTRE + vec3(0.0f, 0.4f, 0.0f)) * scale({3.6f, 4.6f, 3.6f}), Materials::POLISHED_BLUE);

    // A flat patch: every vertex normal is straight up, so the ONLY thing that
    // can make a highlight appear inside a triangle is per-pixel lighting.
    drawMesh(shader, level.patch, translate(PATCH_CENTRE) * scale({PATCH_SIZE, 1.0f, PATCH_SIZE}), Materials::GALLERY_WATER);

    drawStretchedEllipsoid(shader, m, view.galleryWrongNormalMatrix);
}

std::string DemoGallery::detailSummary(GalleryDetail detail) const {
    const DetailLevel& level = levels[(int)detail];
    return std::string(level.name) + " detail: sphere " + std::to_string(level.sphere.triangleCount())
         + " tris, cylinder " + std::to_string(level.cylinder.triangleCount())
         + ", water patch " + std::to_string(level.patch.triangleCount());
}

glm::vec3 DemoGallery::cameraEye() { return CAMERA_EYE; }
glm::vec3 DemoGallery::cameraTarget() { return CAMERA_TARGET; }
glm::vec3 DemoGallery::waterPatchCentre() { return PATCH_CENTRE; }
float DemoGallery::waterPatchSize() { return PATCH_SIZE; }

glm::vec3 DemoGallery::lampStartPosition() {
    // Mirror trick: a flat surface reflects the lamp towards the camera at the
    // point where the line from the camera's MIRROR IMAGE (camera reflected
    // in the water plane) to the lamp crosses the plane. So: start at the
    // mirrored camera, aim through the patch centre, and keep going a little.
    // The highlight then sits exactly in the middle of the patch -- between
    // its corners, which is where Gouraud shading cannot see it.
    vec3 mirroredEye(CAMERA_EYE.x, 2.0f * PATCH_CENTRE.y - CAMERA_EYE.y, CAMERA_EYE.z);
    return PATCH_CENTRE + 0.25f * (PATCH_CENTRE - mirroredEye);
}
