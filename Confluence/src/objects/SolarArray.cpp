#include "SolarArray.h"
#include "../core/Material.h"
#include "../core/Noise.h"
#include "../core/Render.h"
#include <glm/gtc/matrix_transform.hpp>

namespace {

using glm::vec3;
using glm::mat4;

constexpr int ROWS = 2, COLS = 7;               // panels
constexpr int CELL_ROWS = 8, CELL_COLS = 6;      // cells per panel
constexpr float PW = 1.9f, PH = 2.1f, GAP = 0.06f;
constexpr float TILT_DEG = 30.0f;
constexpr float LOWER_EDGE_Y = 0.6f;

const vec3 CELL(0.08f, 0.22f, 0.62f);
const vec3 BACKSHEET(0.86f, 0.88f, 0.92f);
const vec3 FRAME(0.78f, 0.80f, 0.83f);
const vec3 RACK(0.60f, 0.62f, 0.65f);

mat4 T(vec3 t) { return glm::translate(mat4(1.0f), t); }
mat4 S(vec3 s) { return glm::scale(mat4(1.0f), s); }

MeshData build() {
    MeshData d;
    MeshData quad = quadData(), cube = cubeData();
    Noise::Rng rng(11);

    // The tilted plane: +x along the array, +y up the slope, +z out of the glass.
    const mat4 plane = T({0.0f, LOWER_EDGE_Y, 0.0f}) * glm::rotate(mat4(1.0f), glm::radians(TILT_DEG - 90.0f), vec3(1, 0, 0));
    const float width = COLS * (PW + GAP), height = ROWS * (PH + GAP);

    for (int r = 0; r < ROWS; ++r) {
        for (int c = 0; c < COLS; ++c) {
            mat4 p = plane * T({(c - (COLS - 1) * 0.5f) * (PW + GAP), PH * 0.5f + r * (PH + GAP), 0.0f});
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

    // Rack: two rails under the glass and legs running down into the ground
    // (they are long enough to reach it on a slope).
    for (float v : {0.25f, 0.78f}) {
        d.append(cube, plane * T({0.0f, v * height, -0.08f}) * S({width, 0.08f, 0.1f}), RACK);
        vec3 onPlane = vec3(plane * glm::vec4(0.0f, v * height, -0.1f, 1.0f));
        const float bottom = -2.5f;
        for (int k = 0; k <= COLS / 2; ++k) {
            float x = -width * 0.5f + 0.3f + k * (width - 0.6f) / (COLS / 2);
            d.append(cube, T({x, (onPlane.y + bottom) * 0.5f, onPlane.z}) * S({0.09f, onPlane.y - bottom, 0.09f}), RACK);
        }
    }
    return d;
}

}

SolarArray::SolarArray() : mesh(build()) {}

void SolarArray::draw(const Shader& shader, const glm::mat4& root) const {
    drawMesh(shader, mesh, root, Materials::GLOSSY);
}
