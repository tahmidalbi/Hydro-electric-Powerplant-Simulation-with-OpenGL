#pragma once
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include "../core/Mesh.h"

class Shader;
struct MeshLibrary;
struct SceneState;
enum class GalleryDetail;

// F6 - the graphics comparison gallery.
//
// A small, quiet stage (no fog, no landscape) where each graphics idea gets
// one object you can point at during the demonstration.
//
// BACK ROW  - STEP 2, transformations (they move with simulation time, so
//             Space pauses them):
//   1. turbine whose ROOT slides and turns  -> hierarchy: children follow
//   2. gate sliding in a turning frame      -> gate follows its local slot
//   3. bar on a gold post                   -> matrix ORDER (O key: wrong order)
//   4. panel on a hinge                     -> rotation about a PIVOT
//   5. leaning box                          -> SHEAR
//   6. green L + mirror + mirrored L        -> REFLECTION
// FRONT ROW - STEP 4, shading (static, so comparisons are repeatable):
//   7. coarse shiny sphere   8. coarse cylinder   9. coarse flat water patch
//      (+ / - switch the detail level; triangle counts are in the title)
//  10. stretched ellipsoid  -> normal matrix (M key: use the wrong one)
class DemoGallery {
public:
    DemoGallery();

    void draw(const Shader& shader, const MeshLibrary& meshes, const SceneState& state) const;

    // e.g. "LOW detail: sphere 96 tris, cylinder 24, water patch 2"
    std::string detailSummary(GalleryDetail detail) const;

    static glm::vec3 cameraEye();
    static glm::vec3 cameraTarget();

    // The flat water patch (exhibit 9). The self-test uses these to prove,
    // with numbers, that Gouraud misses the lamp's highlight there.
    static glm::vec3 waterPatchCentre();
    static float waterPatchSize();

    // Where the inspection lamp starts in the gallery: placed so its mirror
    // highlight lands in the middle of the water patch (Enter resets it here).
    static glm::vec3 lampStartPosition();

private:
    // The same three shapes built at three tessellation levels.
    struct DetailLevel {
        const char* name;
        Mesh sphere, cylinder, patch;
        DetailLevel(const char* levelName, int sphereStacks, int sphereSlices, int cylinderSegments, int patchCells);
    };
    std::vector<DetailLevel> levels;
    Mesh floor;
};
