#include "Dam.h"
#include "Layout.h"
#include "../core/Material.h"
#include "../core/MeshLibrary.h"
#include "../core/Noise.h"
#include "../core/Render.h"
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <cmath>
#include <utility>

namespace {

using glm::vec3;
using glm::mat4;

const vec3 CONCRETE(0.66f, 0.65f, 0.61f);
const vec3 CONCRETE_DARK(0.55f, 0.55f, 0.52f);
const vec3 ASPHALT(0.36f, 0.37f, 0.39f);
const vec3 RAIL_YELLOW(0.96f, 0.74f, 0.10f);
const vec3 ROOF(0.30f, 0.32f, 0.35f);
const vec3 WINDOW(0.14f, 0.19f, 0.25f);
const vec3 LAMP(0.25f, 0.27f, 0.30f);

constexpr float CREST = Layout::DAM_CREST;
constexpr float SPILL_CREST = 11.5f;   // ogee crest inside the gated bays
constexpr float TOE_Z = 10.0f;         // downstream toe (dam-local z)
constexpr float CREST_Z = 2.5f;        // crest thickness
constexpr float PIER_X[3] = {31.6f, 40.0f, 48.4f};
constexpr float PIER_W = 2.4f;
// Powerhouse right of the spillway; the wheel turns in a channel at its right side.
constexpr float PH_X0 = 53.5f, PH_X1 = 63.5f;
const vec3 WHEEL_MOUNT(67.6f, 4.0f, 11.0f);
constexpr float BAY_W = PIER_X[1] - PIER_X[0] - PIER_W;
constexpr float SPILL_X0 = PIER_X[0] - PIER_W * 0.5f;
constexpr float SPILL_X1 = PIER_X[2] + PIER_W * 0.5f;

mat4 T(vec3 t) { return glm::translate(mat4(1.0f), t); }
mat4 S(vec3 s) { return glm::scale(mat4(1.0f), s); }
mat4 RX(float rad) { return glm::rotate(mat4(1.0f), rad, vec3(1, 0, 0)); }

// Downstream face line of the full-height section.
float faceZ(float y) { return TOE_Z - (TOE_Z - CREST_Z) * y / CREST; }

struct Builder {
    MeshData d;
    MeshData cube = cubeData();

    void box(vec3 lo, vec3 hi, vec3 col) { d.append(cube, T((lo + hi) * 0.5f) * S(hi - lo), col); }

    // Gravity section: vertical upstream face at zBack, crest zBack..zTop, toe at zToe.
    void wedge(float x0, float x1, float height, float zBack, float zTop, float zToe, vec3 col) {
        float depth = zToe - zBack;
        d.append(wedgeData((zTop - zBack) / depth),
                 T({(x0 + x1) * 0.5f, height * 0.5f, zBack + depth * 0.5f}) * S({x1 - x0, height, depth}), col);
    }

    void railing(vec3 a, vec3 b) {   // yellow handrail between two floor points (along x or z)
        vec3 dir = b - a;
        int posts = std::max(1, int(glm::length(dir) / 2.0f));
        for (int i = 0; i <= posts; ++i) {
            vec3 p = a + dir * (float(i) / posts);
            box(p + vec3(-0.06f, 0.0f, -0.06f), p + vec3(0.06f, 1.05f, 0.06f), RAIL_YELLOW);
        }
        vec3 thick = std::abs(dir.x) > std::abs(dir.z) ? vec3(0.0f, 0.0f, 0.06f) : vec3(0.06f, 0.0f, 0.0f);
        box(glm::min(a, b) - thick + vec3(0, 0.97f, 0), glm::max(a, b) + thick + vec3(0, 1.07f, 0), RAIL_YELLOW);
        box(glm::min(a, b) - thick * 0.7f + vec3(0, 0.50f, 0), glm::max(a, b) + thick * 0.7f + vec3(0, 0.57f, 0), RAIL_YELLOW);
    }
};

MeshData buildStructure() {
    Builder b;
    Noise::Rng rng(5);

    // Body, poured in ~5 m blocks; the slight tone steps read as construction joints.
    auto section = [&](float x0, float x1, float height) {
        int n = std::max(1, int((x1 - x0) / 5.0f + 0.5f));
        for (int i = 0; i < n; ++i) {
            float a = x0 + (x1 - x0) * i / n, c = x0 + (x1 - x0) * (i + 1) / n;
            b.wedge(a, c, height, 0.0f, faceZ(height), TOE_Z, CONCRETE * rng.range(0.93f, 1.03f));
        }
    };
    section(0.0f, SPILL_X0, CREST);
    section(SPILL_X0, SPILL_X1, SPILL_CREST);
    section(SPILL_X1, Layout::DAM_LENGTH, CREST);

    // Crest road, upstream parapet, downstream handrail and lamp posts.
    for (auto [x0, x1] : {std::pair{0.0f, SPILL_X0}, std::pair{SPILL_X1, Layout::DAM_LENGTH}}) {
        b.box({x0, CREST, 0.35f}, {x1, CREST + 0.06f, CREST_Z - 0.2f}, ASPHALT);
        b.box({x0, CREST, 0.0f}, {x1, CREST + 0.8f, 0.35f}, CONCRETE);
        b.railing({x0, CREST, CREST_Z - 0.1f}, {x1, CREST, CREST_Z - 0.1f});
        for (float x = x0 + 4.0f; x < x1 - 1.0f; x += 9.0f) {
            b.box({x - 0.08f, CREST + 0.8f, 0.1f}, {x + 0.08f, CREST + 4.3f, 0.26f}, LAMP);
            b.box({x - 0.3f, CREST + 4.2f, 0.05f}, {x + 0.3f, CREST + 4.4f, 0.8f}, LAMP);
        }
    }

    // Spillway piers, each carrying a gate house; a service bridge spans them.
    for (float px : PIER_X) {
        float x0 = px - PIER_W * 0.5f, x1 = px + PIER_W * 0.5f;
        b.wedge(x0, x1, 19.0f, -1.2f, 3.2f, 12.0f, CONCRETE * 0.98f);
        b.box({px - 1.5f, 19.0f, -1.0f}, {px + 1.5f, 22.0f, 3.0f}, CONCRETE_DARK * 1.1f);
        b.box({px - 1.75f, 22.0f, -1.25f}, {px + 1.75f, 22.35f, 3.25f}, ROOF);
        for (float wx : {-0.7f, 0.7f})
            b.box({px + wx - 0.35f, 20.0f, 3.0f}, {px + wx + 0.35f, 21.2f, 3.06f}, WINDOW);
        b.box({x0 + 0.4f, 0.0f, 11.5f}, {x1 - 0.4f, 1.6f, 17.0f}, CONCRETE_DARK);   // training wall
    }
    b.box({SPILL_X0, 18.3f, -0.8f}, {SPILL_X1, 19.0f, 2.6f}, CONCRETE);
    b.railing({SPILL_X0, 19.0f, 2.5f}, {SPILL_X1, 19.0f, 2.5f});
    b.box({SPILL_X0, -0.5f, 10.0f}, {SPILL_X1, 0.3f, 19.0f}, CONCRETE_DARK);        // stilling basin

    // Powerhouse against the dam face, right of the spillway.
    const float PH_H = 7.5f, PH_FRONT = 13.0f;
    b.box({PH_X0, 0.0f, 5.0f}, {PH_X1, PH_H, PH_FRONT}, CONCRETE * 0.99f);
    b.box({PH_X0 - 0.2f, PH_H, 4.8f}, {PH_X1 + 0.2f, PH_H + 0.3f, PH_FRONT + 0.2f}, ROOF);
    b.box({PH_X0 + 1.5f, PH_H + 0.3f, 5.5f}, {PH_X1 - 1.5f, PH_H + 2.8f, 10.5f}, CONCRETE_DARK * 1.1f);
    b.box({PH_X0 + 1.3f, PH_H + 2.8f, 5.3f}, {PH_X1 - 1.3f, PH_H + 3.1f, 10.7f}, ROOF);
    for (int i = 0; i < 4; ++i) {
        float x = PH_X0 + 1.8f + i * 1.8f;
        b.box({x, PH_H + 0.9f, 10.5f}, {x + 1.0f, PH_H + 2.1f, 10.56f}, WINDOW);
        b.box({x, 4.0f, PH_FRONT}, {x + 1.0f, 5.6f, PH_FRONT + 0.06f}, WINDOW);
    }
    b.railing({PH_X0, PH_H + 0.3f, PH_FRONT}, {PH_X1, PH_H + 0.3f, PH_FRONT});
    b.railing({PH_X0, PH_H + 0.3f, 5.0f}, {PH_X0, PH_H + 0.3f, PH_FRONT});
    b.railing({PH_X1, PH_H + 0.3f, 5.0f}, {PH_X1, PH_H + 0.3f, PH_FRONT});

    // Water-wheel channel: tailrace sill, bearing pier and outer wall.
    b.box({PH_X1, -0.5f, 6.0f}, {72.0f, 0.5f, 16.0f}, CONCRETE_DARK);
    b.box({WHEEL_MOUNT.x - 0.9f, 0.0f, 7.0f}, {WHEEL_MOUNT.x + 0.9f, WHEEL_MOUNT.y, 9.2f}, CONCRETE_DARK);
    b.box({71.5f, 0.0f, 6.0f}, {72.7f, 3.5f, 16.0f}, CONCRETE);
    return b.d;
}

// Water pouring under a gate, over the ogee crest and down the spillway face,
// built in bay-local space (x across the bay, y/z as dam-local).
MeshData buildCascade() {
    MeshData d;
    MeshData cube = cubeData(), puff = sphereData(6, 10);
    Noise::Rng rng(21);
    const glm::vec2 path[] = {{0.6f, 12.3f}, {3.9f, 12.1f}, {5.1f, 10.2f}, {6.7f, 7.2f},
                              {8.3f, 4.2f}, {9.7f, 1.7f}, {11.6f, 0.9f}, {14.0f, 0.75f}};   // (z, y)
    const int n = sizeof(path) / sizeof(path[0]);

    for (int i = 0; i + 1 < n; ++i) {
        glm::vec2 a = path[i], c = path[i + 1];
        glm::vec2 dir = c - a;
        float t = float(i) / (n - 2);
        float angle = std::atan2(-dir.y, dir.x);   // turns local +z onto the flow direction
        float width = BAY_W + 0.8f * t, thick = 0.45f + 0.9f * t;
        vec3 col = glm::mix(vec3(0.78f, 0.89f, 0.96f), vec3(1.0f), t);
        mat4 seg = T({0.0f, (a.y + c.y) * 0.5f, (a.x + c.x) * 0.5f}) * RX(angle);
        d.append(cube, seg * S({width, thick, glm::length(dir) * 1.12f}), col);

        // Froth lumps riding on the sheet.
        glm::vec2 normal = glm::normalize(glm::vec2(-dir.y, dir.x));   // (z, y), away from the concrete
        for (int k = 0; k < 10; ++k) {
            float s = rng.next();
            glm::vec2 p = a + dir * s + normal * thick * 0.45f;
            float r = rng.range(0.35f, 0.8f) * (0.7f + t);
            float w = rng.range(0.93f, 1.0f);
            d.append(puff, T({rng.range(-0.45f, 0.45f) * width, p.y, p.x}) * RX(angle) * S({r * 2.0f, r * 0.8f, r * 2.6f}), vec3(w, w, 1.0f));
        }
    }
    // Spray where the chute hits the basin.
    for (int k = 0; k < 18; ++k) {
        float r = rng.range(0.8f, 1.6f);
        d.append(puff, T({rng.range(-0.5f, 0.5f) * (BAY_W + 1.0f), rng.range(0.8f, 2.0f), rng.range(10.0f, 14.5f)}) * S(vec3(r * 2.0f, r * 1.4f, r * 2.0f)), vec3(1.0f));
    }
    return d;
}

}

Dam::Dam() : structure(buildStructure()), cascade(buildCascade()) {}

glm::vec3 Dam::wheelMount() { return WHEEL_MOUNT; }

void Dam::draw(const Shader& shader, const MeshLibrary& m, float gateOpening) const {
    const mat4 root = Layout::damRoot();
    drawMesh(shader, structure, root, Materials::MATTE);

    for (int bay = 0; bay < 2; ++bay) {
        float cx = (PIER_X[bay] + PIER_X[bay + 1]) * 0.5f;
        glm::mat4 gateSlot = root * T({cx, SPILL_CREST, 0.3f});
        glm::mat4 gate = gateSlot * T({0.0f, 2.9f + 3.2f * gateOpening, 0.0f});
        drawMesh(shader, m.cube, gate * S({BAY_W, 5.8f, 0.5f}), Materials::STEEL);

        if (gateOpening > 0.02f)
            drawMesh(shader, cascade, root * T({cx, 0.0f, 0.0f}), Materials::FOAM);
    }
}
