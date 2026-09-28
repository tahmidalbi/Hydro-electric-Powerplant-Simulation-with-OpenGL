#include "Cloud.h"
#include "../core/Material.h"
#include "../core/Noise.h"
#include "../core/Render.h"
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <cmath>

namespace {

using glm::vec3;
using glm::mat4;

MeshData build(unsigned int seed, vec3 extent) {
    MeshData d;
    MeshData puff = sphereData(10, 16);
    Noise::Rng rng(seed);

    // Large rounded masses first, then many small puffs over their tops for the
    // cauliflower texture of a cumulus.
    for (int i = 0; i < 22; ++i) {
        float x = rng.range(-0.5f, 0.5f), z = rng.range(-0.5f, 0.5f);
        float core = 1.0f - std::min(1.0f, std::sqrt(x * x + z * z) * 2.0f);   // 1 in the middle, 0 at the rim
        float r = extent.x * rng.range(0.09f, 0.17f) * (0.6f + 0.6f * core);
        float y = extent.y * rng.range(0.05f, 0.40f) * core;
        d.append(puff, glm::translate(mat4(1.0f), {x * extent.x * 0.8f, y, z * extent.z * 0.8f}) * glm::scale(mat4(1.0f), vec3(r * 2.0f)));
    }
    for (int i = 0; i < 50; ++i) {
        float x = rng.range(-0.45f, 0.45f), z = rng.range(-0.4f, 0.4f);
        float core = 1.0f - std::min(1.0f, std::sqrt(x * x + z * z) * 2.2f);
        float r = extent.x * rng.range(0.035f, 0.07f);
        float y = extent.y * (0.25f + 0.55f * core) * rng.range(0.7f, 1.0f);
        d.append(puff, glm::translate(mat4(1.0f), {x * extent.x * 0.8f, y, z * extent.z * 0.8f}) * glm::scale(mat4(1.0f), vec3(r * 2.0f)));
    }

    // Flatten the underside and grade the colour from grey-blue base to white top.
    for (Vertex& v : d.vertices) {
        if (v.position.y < 0.0f) {
            v.position.y *= 0.2f;
            v.normal = glm::normalize(glm::mix(v.normal, vec3(0.0f, -1.0f, 0.0f), 0.6f));
        }
        float t = std::clamp(v.position.y / (extent.y * 0.6f), 0.0f, 1.0f);
        v.color = glm::mix(vec3(0.80f, 0.84f, 0.90f), vec3(1.0f), t);
    }
    return d;
}

}

Cloud::Cloud(unsigned int seed, glm::vec3 extent) : mesh(build(seed, extent)) {}

void Cloud::draw(const Shader& shader, const glm::mat4& root) const {
    drawMesh(shader, mesh, root, Materials::CLOUD);
}
