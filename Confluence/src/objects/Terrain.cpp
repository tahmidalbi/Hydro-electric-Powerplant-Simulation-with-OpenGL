#include "Terrain.h"
#include "Layout.h"
#include "../core/Material.h"
#include "../core/Noise.h"
#include "../core/Render.h"
#include <algorithm>
#include <cmath>
#include <vector>

namespace {

float smooth01(float t) { t = std::clamp(t, 0.0f, 1.0f); return t * t * (3.0f - 2.0f * t); }
float sq(float v) { return v * v; }
float gauss(float x, float z, float cx, float cz, float rx, float rz) {
    return std::exp(-sq((x - cx) / rx) - sq((z - cz) / rz));
}

// Water bed: the river valley below the dam, the lake basin above it (the
// step between them is hidden inside the dam wall).
float bedHeight(float x, float z) {
    float upstream = smooth01((-Layout::damSide(x, z) - 1.0f) / 6.0f);
    return -1.0f + upstream * (Layout::LAKE_LEVEL - 5.0f);
}

float hillsHeight(float x, float z) {
    // Left massif: the slope the viewer stands on, climbing back to the turbine ridge.
    float dL = Layout::leftBankX(z) - x;
    float ridge = 16.0f + 8.5f * smooth01((-z - 30.0f) / 40.0f);
    float rise = 6.0f + 3.0f * smooth01((-z - 35.0f) / 35.0f);   // steep meadow bank near the viewer
    float left = ridge * (1.0f - std::exp(-std::max(dL, 0.0f) / rise)) + std::min(dL, 0.0f) * 1.5f;

    // Right cliff the dam's right end is keyed into; it gives way to the lake upstream.
    float dR = x - Layout::rightBankX(z);
    float cliff = 15.5f * (1.0f - std::exp(-std::max(dR, 0.0f) / 3.0f)) + std::min(dR, 0.0f) * 1.5f;
    float keep = smooth01((Layout::damSide(x, z) + 14.0f) / 6.0f);
    float right = cliff * keep - 8.0f * (1.0f - keep);

    // Forested hills around the lake and the distant range closing the view.
    // Forested hills rising from the lake's far right shore.
    float farRight = 55.0f * gauss(x, z, 140.0f, -240.0f, 70.0f, 70.0f)
                   + 40.0f * gauss(x, z, 210.0f, -130.0f, 60.0f, 70.0f);
    float farLeft = 40.0f * gauss(x, z, -120.0f, -210.0f, 70.0f, 90.0f)
                  + 34.0f * gauss(x, z, -60.0f, -250.0f, 45.0f, 45.0f);
    float range = (34.0f + 10.0f * std::sin(x * 0.02f + 1.3f) + 7.0f * std::sin(x * 0.05f + 0.5f) + 4.0f * std::sin(x * 0.11f))
                * smooth01((-z - 350.0f) / 60.0f);

    return std::max({left, right, farRight, farLeft, range});
}

glm::vec3 terrainColor(float x, float z, float h, float ny) {
    float n1 = Noise::fbm(x * 0.06f, z * 0.06f, 3, 41);
    float n2 = Noise::value(x * 0.7f, z * 0.7f, 43);
    float n3 = Noise::fbm(x * 0.25f, z * 0.25f, 2, 47);

    // Meadow: mottled between deep green, fresh green and sun-dried yellow-green.
    glm::vec3 grass = glm::mix(glm::vec3(0.21f, 0.37f, 0.11f), glm::vec3(0.33f, 0.50f, 0.15f), smooth01(n1 * 2.2f));
    grass = glm::mix(grass, glm::vec3(0.50f, 0.56f, 0.23f), smooth01((n1 - 0.55f) * 5.0f));
    grass *= 0.78f + 0.18f * n2 + 0.28f * n3;
    glm::vec3 rock = glm::vec3(0.53f, 0.50f, 0.45f) * (0.8f + 0.35f * n2);
    glm::vec3 forest = glm::vec3(0.09f, 0.22f, 0.09f) * (0.8f + 0.4f * n1);

    // Far slopes read as unbroken forest; near slopes as meadow.
    float dist = std::sqrt(x * x + z * z);
    glm::vec3 c = glm::mix(grass, forest, smooth01((dist - 110.0f) / 90.0f));

    // Steep faces are bare rock, and grey outcrops break through the hillside.
    float outcrop = smooth01((Noise::fbm(x * 0.09f, z * 0.09f, 3, 53) - 0.56f) * 7.0f) * smooth01((h - 4.0f) / 4.0f);
    // Near the viewer even steep ground stays grassed (a meadow running down to
    // the river); further off, steep flanks turn to rock.
    // Distant slopes stay uniformly forested (rock/forest striping would alias
    // into streaks on the coarse far grid).
    float rockSlope = glm::mix(0.25f, 0.72f, smooth01((dist - 45.0f) / 40.0f));
    float rockW = std::max(smooth01((rockSlope - ny) / 0.15f),
                           outcrop * smooth01((dist - 30.0f) / 20.0f) * (1.0f - smooth01((dist - 120.0f) / 40.0f)));
    c = glm::mix(c, rock, rockW * (1.0f - smooth01((dist - 170.0f) / 60.0f)));

    // The right bank below the dam is a bare rock wall up to its wooded top.
    if (Layout::damSide(x, z) > -4.0f) {
        float wall = smooth01((x - Layout::rightBankX(z) + 1.0f) / 2.0f) * (1.0f - smooth01((h - 12.5f) / 2.0f));
        c = glm::mix(c, rock * 0.95f, wall);
    }

    // Rocky, wet band just above the water line (lake shore and river banks).
    float waterLine = Layout::damSide(x, z) < 0.0f ? Layout::LAKE_LEVEL : Layout::RIVER_LEVEL;
    float shore = 1.0f - smooth01((h - waterLine - 0.3f) / 1.8f);
    return glm::mix(c, glm::mix(rock, glm::vec3(0.50f, 0.46f, 0.36f), 0.4f), shore);
}

// Grid coordinates: fine spacing where the camera looks closely, growing
// geometrically towards the horizon.
std::vector<float> axisSamples(float lo, float hi, float denseLo, float denseHi, float fine, float coarse) {
    std::vector<float> s;
    for (float v = denseLo; v < denseHi; v += fine) s.push_back(v);
    s.push_back(denseHi);
    float step = fine;
    for (float v = denseLo; v > lo;) { step = std::min(step * 1.07f, coarse); v = std::max(v - step, lo); s.push_back(v); }
    step = fine;
    for (float v = denseHi; v < hi;) { step = std::min(step * 1.07f, coarse); v = std::min(v + step, hi); s.push_back(v); }
    std::sort(s.begin(), s.end());
    return s;
}

MeshData buildTerrain() {
    std::vector<float> xs = axisSamples(-320.0f, 340.0f, -75.0f, 80.0f, 0.8f, 9.0f);
    std::vector<float> zs = axisSamples(-560.0f, 60.0f, -160.0f, 30.0f, 0.8f, 9.0f);

    MeshData d;
    d.vertices.reserve(xs.size() * zs.size());
    for (float z : zs) {
        for (float x : xs) {
            float h = terrainHeight(x, z);
            glm::vec3 n = terrainNormal(x, z);
            d.vertices.push_back({{x, h, z}, n, terrainColor(x, z, h, n.y)});
        }
    }
    const unsigned int cols = (unsigned int)xs.size();
    for (unsigned int j = 0; j + 1 < zs.size(); ++j) {
        for (unsigned int i = 0; i + 1 < cols; ++i) {
            unsigned int a = j * cols + i, c = a + cols;
            d.indices.insert(d.indices.end(), {a, c, a + 1, a + 1, c, c + 1});
        }
    }
    return d;
}

}

float terrainHeight(float x, float z) {
    float bed = bedHeight(x, z);
    float hills = hillsHeight(x, z);
    float land = smooth01((hills - bed) / 4.0f);
    // Small-scale bumps only near the viewer, where the grid is fine enough
    // to carry them (far away they alias into streaks on the coarse grid).
    float near = 1.0f - smooth01((std::sqrt(x * x + z * z) - 140.0f) / 80.0f);
    float detail = ((Noise::fbm(x * 0.035f, z * 0.035f, 4, 3) - 0.5f) * 6.0f
                  + (Noise::fbm(x * 0.22f, z * 0.22f, 3, 9) - 0.5f) * 1.2f) * near
                 + (Noise::fbm(x * 0.008f, z * 0.008f, 3, 21) - 0.5f) * 16.0f * smooth01((-z - 250.0f) / 100.0f);
    return std::max(bed, hills) + land * detail;
}

glm::vec3 terrainNormal(float x, float z) {
    const float e = 0.35f;
    float hx = terrainHeight(x + e, z) - terrainHeight(x - e, z);
    float hz = terrainHeight(x, z + e) - terrainHeight(x, z - e);
    return glm::normalize(glm::vec3(-hx, 2.0f * e, -hz));
}

Terrain::Terrain() : mesh(buildTerrain()) {}

void Terrain::draw(const Shader& shader) const {
    drawMesh(shader, mesh, glm::mat4(1.0f), Materials::TERRAIN);
}
