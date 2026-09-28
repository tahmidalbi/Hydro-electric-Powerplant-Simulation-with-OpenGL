#include "Rocks.h"
#include "Terrain.h"
#include "../core/Material.h"
#include "../core/Noise.h"
#include "../core/Render.h"
#include <glm/gtc/matrix_transform.hpp>

namespace {

using glm::vec3;
using glm::mat4;

struct Placement { float x, z, size; };

const Placement ROCKS[] = {
    // Foreground, bottom-left and bottom-centre of the view.
    {-7.0f, -12.0f, 1.0f}, {-5.2f, -13.5f, 0.6f}, {-9.0f, -11.0f, 0.8f},
    {-11.0f, -13.0f, 1.2f}, {-4.0f, -11.5f, 0.7f}, {-12.5f, -16.0f, 0.9f}, {-6.0f, -15.5f, 0.5f},
    {-2.0f, -18.0f, 0.9f}, {0.3f, -17.5f, 0.6f}, {-0.8f, -18.8f, 0.8f}, {-3.5f, -19.5f, 0.6f},
    // River banks and the foot of the right cliff.
    {5.0f, -30.0f, 1.3f}, {1.0f, -45.0f, 1.6f}, {-3.0f, -55.0f, 1.8f}, {-8.0f, -70.0f, 2.0f},
    {24.0f, -50.0f, 2.0f}, {27.0f, -44.0f, 1.6f}, {22.0f, -40.0f, 1.2f}, {-12.0f, -90.0f, 2.4f},
};

MeshData build() {
    MeshData d;
    Noise::Rng rng(31);
    for (const Placement& p : ROCKS) {
        MeshData rock = sphereData(10, 14);
        float seed = rng.range(0.0f, 100.0f);
        for (Vertex& v : rock.vertices) {
            vec3 q = v.position * 2.0f;   // unit sphere
            float n = Noise::fbm(q.x * 1.3f + seed, q.z * 1.3f + q.y * 0.9f, 3, 5);
            v.position = q * (0.75f + 0.5f * n);
            v.position.y *= 0.62f;
        }
        computeSmoothNormals(rock.vertices, rock.indices);

        float h = terrainHeight(p.x, p.z);
        mat4 m = glm::translate(mat4(1.0f), {p.x, h + 0.15f * p.size, p.z})
               * glm::rotate(mat4(1.0f), rng.range(0.0f, 6.28f), vec3(0, 1, 0))
               * glm::scale(mat4(1.0f), vec3(p.size));
        d.append(rock, m, vec3(0.56f, 0.54f, 0.50f) * rng.range(0.85f, 1.1f));
    }
    return d;
}

}

Rocks::Rocks() : mesh(build()) {}

void Rocks::draw(const Shader& shader) const {
    drawMesh(shader, mesh, mat4(1.0f), Materials::MATTE);
}
