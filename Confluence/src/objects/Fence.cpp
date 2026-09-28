#include "Fence.h"
#include "Terrain.h"
#include "../core/Material.h"
#include "../core/Render.h"
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <cmath>
#include <vector>

namespace {

using glm::vec3;
using glm::mat4;

const vec3 WOOD(0.46f, 0.34f, 0.21f);
const glm::vec2 PATH[] = {{-6.5f, -26.0f}, {-4.8f, -28.0f}, {-3.2f, -30.0f}, {-2.4f, -32.0f}};
constexpr float POST_SPACING = 1.8f;

MeshData build() {
    MeshData d;
    MeshData cube = cubeData();

    std::vector<vec3> posts;
    for (size_t s = 0; s + 1 < sizeof(PATH) / sizeof(PATH[0]); ++s) {
        glm::vec2 a = PATH[s], b = PATH[s + 1];
        int n = std::max(1, int(glm::length(b - a) / POST_SPACING));
        for (int i = 0; i < n; ++i) {
            glm::vec2 p = glm::mix(a, b, float(i) / n);
            posts.push_back({p.x, terrainHeight(p.x, p.y), p.y});
        }
    }
    glm::vec2 last = PATH[sizeof(PATH) / sizeof(PATH[0]) - 1];
    posts.push_back({last.x, terrainHeight(last.x, last.y), last.y});

    for (const vec3& p : posts)
        d.append(cube, glm::translate(mat4(1.0f), p + vec3(0.0f, 0.4f, 0.0f)) * glm::scale(mat4(1.0f), {0.14f, 1.4f, 0.14f}), WOOD);

    // Two rails per span, each a box aligned with the (sloping) span.
    for (size_t i = 0; i + 1 < posts.size(); ++i) {
        for (float h : {0.55f, 1.0f}) {
            vec3 a = posts[i] + vec3(0.0f, h, 0.0f), b = posts[i + 1] + vec3(0.0f, h, 0.0f);
            vec3 dir = b - a;
            float len = glm::length(dir);
            float yaw = std::atan2(dir.x, dir.z), pitch = std::asin(dir.y / len);
            mat4 m = glm::translate(mat4(1.0f), (a + b) * 0.5f)
                   * glm::rotate(mat4(1.0f), yaw, vec3(0, 1, 0))
                   * glm::rotate(mat4(1.0f), -pitch, vec3(1, 0, 0))
                   * glm::scale(mat4(1.0f), {0.07f, 0.1f, len});
            d.append(cube, m, WOOD);
        }
    }
    return d;
}

}

Fence::Fence() : mesh(build()) {}

void Fence::draw(const Shader& shader) const {
    drawMesh(shader, mesh, mat4(1.0f), Materials::MATTE);
}
