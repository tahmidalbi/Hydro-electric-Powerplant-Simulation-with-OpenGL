#include "SolarArray.h"
#include "Placement.h"
#include "Terrain.h"
#include "../core/Material.h"
#include "../core/Noise.h"
#include "../core/Render.h"
#include "../core/Transform.h"
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <cmath>

namespace {

using glm::vec3;
using glm::mat4;

constexpr int ROWS = 2, COLS = 7;               // panels
constexpr int CELL_ROWS = 8, CELL_COLS = 6;      // cells per panel
constexpr float PW = 1.9f, PH = 2.1f, GAP = 0.06f;
constexpr float WIDTH = COLS * (PW + GAP);
constexpr float HEIGHT = ROWS * (PH + GAP);      // up the slope
constexpr float TUBE_Z = -0.1f;                  // torque tube, just behind the glass

constexpr float HINGE_Z = -1.92f;                // tube position front/back (as the old fixed array)
constexpr float POST_HEIGHT = 2.0f;              // tube height above the ground at each end

// The torque tube FOLLOWS THE HILLSIDE, like real single-axis trackers built
// on slopes: the ground under the array falls about 4.5 m from its left end
// to its right end, so a level tube would bury one end. Measured once from
// the terrain:
//   centre = midpoint of the tube, POST_HEIGHT above the ground at both ends
//   roll   = angle of the tube along the array (rotation about local z)
struct HingeLine { vec3 centre; float roll; };

const HingeLine& hingeLine() {
    static const HingeLine line = [] {
        const mat4 root = Placement::solarRoot();
        auto groundAt = [&](float x) {
            glm::vec3 p = glm::vec3(root * glm::vec4(x, 0.0f, HINGE_Z, 1.0f));
            return terrainHeight(p.x, p.z) - root[3][1];   // relative to the root's height
        };
        float left = groundAt(-WIDTH * 0.5f), right = groundAt(WIDTH * 0.5f);
        HingeLine h;
        h.centre = vec3(0.0f, 0.5f * (left + right) + POST_HEIGHT, HINGE_Z);
        h.roll = std::atan2(right - left, WIDTH);   // rotateZ(roll) turns +x along the slope
        return h;
    }();
    return line;
}

// Height of the tube above the root at position x along the array.
float tubeHeight(float x) { return hingeLine().centre.y + x * std::tan(hingeLine().roll); }

const vec3 CELL(0.08f, 0.22f, 0.62f);
const vec3 BACKSHEET(0.86f, 0.88f, 0.92f);
const vec3 FRAME(0.78f, 0.80f, 0.83f);
const vec3 RACK(0.60f, 0.62f, 0.65f);

mat4 T(vec3 t) { return glm::translate(mat4(1.0f), t); }
mat4 S(vec3 s) { return glm::scale(mat4(1.0f), s); }

// MOVING part, built in the "panel plane" frame: +x along the array, +y up
// the slope (0 at the lower edge), +z out of the glass. The frame's origin is
// moved onto the hinge at draw time.
MeshData buildPanels() {
    MeshData d;
    MeshData quad = quadData(), cube = cubeData();
    Noise::Rng rng(11);

    for (int r = 0; r < ROWS; ++r) {
        for (int c = 0; c < COLS; ++c) {
            mat4 p = T({(c - (COLS - 1) * 0.5f) * (PW + GAP), PH * 0.5f + r * (PH + GAP), 0.0f});
            d.append(quad, p * S({PW, PH, 1.0f}), BACKSHEET);   // shows through as the cell grid lines

            float cw = (PW - 0.08f) / CELL_COLS, ch = (PH - 0.08f) / CELL_ROWS;
            for (int i = 0; i < CELL_ROWS; ++i)
                for (int j = 0; j < CELL_COLS; ++j)
                    d.append(quad, p * T({-PW * 0.5f + 0.04f + cw * (j + 0.5f), -PH * 0.5f + 0.04f + ch * (i + 0.5f), 0.006f})
                                     * S({cw * 0.9f, ch * 0.88f, 1.0f}), CELL * rng.range(0.9f, 1.12f));

            d.append(cube, p * T({0.0f,  PH * 0.5f, 0.012f}) * S({PW + 0.05f, 0.05f, 0.04f}), FRAME);
            d.append(cube, p * T({0.0f, -PH * 0.5f, 0.012f}) * S({PW + 0.05f, 0.05f, 0.04f}), FRAME);
            d.append(cube, p * T({ PW * 0.5f, 0.0f, 0.012f}) * S({0.05f, PH, 0.04f}), FRAME);
            d.append(cube, p * T({-PW * 0.5f, 0.0f, 0.012f}) * S({0.05f, PH, 0.04f}), FRAME);
        }
    }
    // Two rails under the glass; they turn with the panels.
    for (float v : {0.25f, 0.78f})
        d.append(cube, T({0.0f, v * HEIGHT, -0.08f}) * S({WIDTH, 0.08f, 0.1f}), RACK);
    return d;
}

// FIXED part, built in the array root's frame: the sloping torque tube and
// vertical posts, each post reaching from the tube down into the real ground.
MeshData buildRack() {
    using namespace Transform;
    MeshData d;
    MeshData cube = cubeData(), cyl = cylinderData(12);
    const HingeLine& h = hingeLine();
    // The tube: a cylinder (axis = local y) laid along x, then rolled with the slope.
    d.append(cyl, T(h.centre) * rotateZ(h.roll) * rotateZ(degreesToRadians(90.0f)) * S({0.16f, WIDTH + 0.4f, 0.16f}), RACK);

    const mat4 root = Placement::solarRoot();
    for (int k = 0; k <= COLS / 2; ++k) {
        float x = -WIDTH * 0.5f + 0.3f + k * (WIDTH - 0.6f) / (COLS / 2);
        glm::vec3 w = glm::vec3(root * glm::vec4(x, 0.0f, HINGE_Z, 1.0f));
        float bottom = terrainHeight(w.x, w.z) - root[3][1] - 0.3f;   // sunk 30 cm into the ground
        float top = tubeHeight(x) - 0.05f;
        d.append(cube, T({x, 0.5f * (top + bottom), HINGE_Z}) * S({0.11f, top - bottom, 0.11f}), RACK);
    }
    return d;
}

}

SolarArray::SolarArray() : panels(buildPanels()), rack(buildRack()) {}

mat4 SolarArray::panelMatrix(const mat4& root, float tiltDeg) {
    using namespace Transform;
    const HingeLine& h = hingeLine();
    // Pivot rotation about the (sloping) tube:
    //   root * Translate(tube centre) * RotateZ(slope) * RotateX(tilt - 90 deg) * Translate(panel centre -> origin)
    // Read right to left: put the middle of the panel plane on the origin, tilt
    // it about the x axis (= the tube), roll it to follow the slope, move it
    // onto the tube. The panels turn about the tube, not about a corner.
    return root * translate(h.centre) * rotateZ(h.roll) * rotateX(degreesToRadians(tiltDeg - 90.0f))
         * translate({0.0f, -HEIGHT * 0.5f, -TUBE_Z});
}

// Lowest gap between the panel plane and the ground below it, sampled on a
// 21 x 9 grid over the WHOLE plane (a bump in the hill can rise above the
// middle of a panel even when both edges are clear).
float SolarArray::groundClearance(float tiltDeg) {
    const mat4 m = panelMatrix(Placement::solarRoot(), tiltDeg);
    float lowest = 1e9f;
    for (int i = 0; i <= 20; ++i) {
        float x = -WIDTH * 0.5f + WIDTH * i / 20.0f;
        for (int j = 0; j <= 8; ++j) {
            float v = HEIGHT * j / 8.0f;
            glm::vec3 p = glm::vec3(m * glm::vec4(x, v, 0.0f, 1.0f));
            lowest = std::min(lowest, p.y - terrainHeight(p.x, p.z));
        }
    }
    return lowest;
}

void SolarArray::tiltLimits(float& minDeg, float& maxDeg) {
    // Worked out once from the actual terrain: the steepest tilt (up to 50
    // degrees) whose panel edges still clear the slope by MIN_CLEARANCE.
    static float lo = 0.0f, hi = 0.0f;
    if (hi == 0.0f) {
        const float MIN_CLEARANCE = 0.35f;
        lo = 10.0f;
        hi = lo;
        for (float t = lo; t <= 50.0f; t += 1.0f) {
            if (groundClearance(t) < MIN_CLEARANCE) break;
            hi = t;
        }
    }
    minDeg = lo;
    maxDeg = hi;
}

void SolarArray::draw(const Shader& shader, const mat4& root, float tiltDeg) const {
    drawMesh(shader, rack, root, Materials::STEEL);
    drawMesh(shader, panels, panelMatrix(root, tiltDeg), Materials::PANEL_GLASS);
}
