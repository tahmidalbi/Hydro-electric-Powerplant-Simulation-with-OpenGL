#include "Terrain.h"
#include "Materials.h"

// Value noise: random heights on an integer grid, smoothly interpolated.
static float latticeValue(int x, int z) {
    unsigned int h = (unsigned int)x * 374761393u + (unsigned int)z * 668265263u;
    h = (h ^ (h >> 13)) * 1274126177u;
    h ^= h >> 16;
    return (h & 0xffffff) / 16777215.0f;
}
static float valueNoise(float x, float z) {
    int ix = (int)std::floor(x), iz = (int)std::floor(z);
    float fx = x - ix, fz = z - iz;
    float ux = fx * fx * (3 - 2 * fx), uz = fz * fz * (3 - 2 * fz);
    float a = latticeValue(ix, iz), b = latticeValue(ix + 1, iz);
    float c = latticeValue(ix, iz + 1), d = latticeValue(ix + 1, iz + 1);
    return mixf(mixf(a, b, ux), mixf(c, d, ux), uz);
}
float Terrain::fbm(float x, float z) {
    float s = 0, amp = 0.5f, norm = 0;
    for (int i = 0; i < 6; i++) {
        s += amp * valueNoise(x, z);
        norm += amp;
        x = x * 2.03f + 17.3f;
        z = z * 2.03f + 5.1f;
        amp *= 0.5f;
    }
    return s / norm;
}

float Terrain::height(float x, float z) {
    float ax = std::fabs(x);
    float n = fbm(x * 0.006f, z * 0.006f);
    // half width of the valley floor: wide reservoir upstream, narrower river below
    float W = 122.0f + (z < -30.0f ? (-z - 30.0f) * 0.45f : 0.0f) + (z > 110.0f ? (z - 110.0f) * 0.3f : 0.0f);
    if (z > -10.0f && z < 70.0f) W = 122.0f;            // keep the dam abutments straight
    else W += (n - 0.5f) * 60.0f;
    float base = z < -8.0f ? 16.0f : 0.5f;              // reservoir bed / river bed
    float h = base + (fbm(x * 0.05f, z * 0.05f) - 0.5f) * 2.0f;
    float d = ax - W;
    if (d > 0.0f) {
        float mountain = 230.0f * (1.0f - std::exp(-d * 3.4f / 230.0f));   // steep cliffs that level off
        float rough = fbm(x * 0.02f, z * 0.02f);
        h += mountain * (0.75f + 0.5f * rough) + (fbm(x * 0.08f, z * 0.08f) - 0.5f) * 14.0f * std::fmin(d / 20.0f, 1.0f);
    }
    return h;
}

void Terrain::build() {
    const int N = 260;
    const float X0 = -700, X1 = 700, Z0 = -800, Z1 = 650;
    MeshBuilder b;
    std::vector<float> H((N + 1) * (N + 1));
    for (int j = 0; j <= N; j++)
        for (int i = 0; i <= N; i++) {
            float x = mixf(X0, X1, (float)i / N), z = mixf(Z0, Z1, (float)j / N);
            H[j * (N + 1) + i] = height(x, z);
        }
    float dx = (X1 - X0) / N, dz = (Z1 - Z0) / N;
    for (int j = 0; j <= N; j++)
        for (int i = 0; i <= N; i++) {
            float x = mixf(X0, X1, (float)i / N), z = mixf(Z0, Z1, (float)j / N);
            int il = i > 0 ? i - 1 : i, ir = i < N ? i + 1 : i;
            int jd = j > 0 ? j - 1 : j, ju = j < N ? j + 1 : j;
            // normal from the slope (central differences)
            float hx = (H[j * (N + 1) + ir] - H[j * (N + 1) + il]) / ((ir - il) * dx);
            float hz = (H[ju * (N + 1) + i] - H[jd * (N + 1) + i]) / ((ju - jd) * dz);
            b.addVertex(Vec3(x, H[j * (N + 1) + i], z), normalize(Vec3(-hx, 1.0f, -hz)), Vec2(x, z));
        }
    for (int j = 0; j < N; j++)
        for (int i = 0; i < N; i++) {
            unsigned int a = j * (N + 1) + i, c = a + N + 1;
            b.tri(a, c, a + 1);
            b.tri(a + 1, c, c + 1);
        }
    mesh = b.build();
}

void Terrain::draw(Renderer& r) {
    Material m;
    m.color = Vec3(0.2f, 0.19f, 0.18f);
    m.pattern = PAT_TERRAIN;
    m.specular = 0.08f;
    m.shininess = 10.0f;
    m.gouraud = true;           // Gouraud shading for the big terrain
    r.drawMesh(mesh, Mat4(), m);
}
