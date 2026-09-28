#include "WaterWheel.h"
#include "../core/Material.h"
#include "../core/MeshLibrary.h"
#include "../core/Render.h"
#include <glm/gtc/matrix_transform.hpp>

namespace {

using glm::vec3;
using glm::mat4;

constexpr float PI = 3.14159265358979323846f;
constexpr float RADIUS = 3.8f;
constexpr float WIDTH = 1.6f;

const vec3 BLUE(0.12f, 0.38f, 0.80f);
const vec3 BLUE_DARK(0.09f, 0.28f, 0.62f);
const vec3 BUCKET(0.20f, 0.24f, 0.30f);
const vec3 HUB(0.35f, 0.37f, 0.40f);

mat4 T(vec3 t) { return glm::translate(mat4(1.0f), t); }
mat4 S(vec3 s) { return glm::scale(mat4(1.0f), s); }
mat4 RZ(float rad) { return glm::rotate(mat4(1.0f), rad, vec3(0, 0, 1)); }

MeshData buildWheel() {
    MeshData d;
    MeshData cube = cubeData(), cyl = cylinderData(20);

    auto ring = [&](float r, float radial, float axial, float z, int segments) {
        float len = 2.0f * PI * r / segments * 1.08f;
        for (int i = 0; i < segments; ++i)
            d.append(cube, RZ(2.0f * PI * i / segments) * T({0.0f, r, z}) * S({len, radial, axial}), BLUE);
    };

    for (float side : {-1.0f, 1.0f}) {
        float z = side * WIDTH * 0.5f;
        ring(RADIUS - 0.18f, 0.36f, 0.24f, z, 40);   // outer rim
        ring(RADIUS * 0.66f, 0.24f, 0.20f, z, 32);   // inner rim
        const float spokeLen = RADIUS - 0.9f;
        for (int i = 0; i < 10; ++i)                 // spokes
            d.append(cube, RZ(2.0f * PI * i / 10) * T({0.0f, 0.6f + spokeLen * 0.5f, z}) * S({0.2f, spokeLen, 0.18f}), BLUE_DARK);
    }
    for (int i = 0; i < 28; ++i)                     // buckets spanning the two rims
        d.append(cube, RZ(2.0f * PI * i / 28) * T({0.0f, RADIUS - 0.65f, 0.0f}) * RZ(0.35f) * S({0.12f, 1.1f, WIDTH}), BUCKET);
    d.append(cyl, glm::rotate(mat4(1.0f), PI * 0.5f, vec3(1, 0, 0)) * S({1.2f, WIDTH + 0.8f, 1.2f}), HUB);
    return d;
}

}

WaterWheel::WaterWheel() : wheel(buildWheel()) {}

void WaterWheel::draw(const Shader& shader, const MeshLibrary& m, const glm::mat4& root, float spinAngle) const {
    // Axle runs back into its bearing pier against the dam.
    drawMesh(shader, m.cylinder, root * T({0.0f, 0.0f, -2.0f}) * glm::rotate(mat4(1.0f), PI * 0.5f, vec3(1, 0, 0)) * S({0.5f, 5.0f, 0.5f}), Materials::STEEL);
    drawMesh(shader, wheel, root * RZ(spinAngle), Materials::GLOSSY);
}
