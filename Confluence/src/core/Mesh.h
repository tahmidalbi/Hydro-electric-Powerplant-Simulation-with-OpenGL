#pragma once
#include <vector>
#include <glm/glm.hpp>

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec3 color{1.0f, 1.0f, 1.0f};   // albedo tint; white for single-material primitives
};

// CPU-side geometry. Primitives are authored centred at the origin, spanning
// [-0.5, 0.5] on each local axis; the cylinder/cone axis is local Y.
struct MeshData {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    // Appends `other` transformed by `model` and tinted by `color`. Static
    // scenery (trees, railings, rocks, the dam) is baked this way so each
    // object costs one draw call.
    void append(const MeshData& other, const glm::mat4& model, const glm::vec3& color = glm::vec3(1.0f));
};

// One VAO/VBO/EBO on the GPU.
class Mesh {
public:
    explicit Mesh(const MeshData& data);
    ~Mesh();

    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;
    Mesh(Mesh&& other) noexcept;
    Mesh& operator=(Mesh&& other) noexcept;

    void draw() const;

private:
    unsigned int VAO = 0, VBO = 0, EBO = 0;
    int indexCount = 0;
    void release();
};

MeshData cubeData();
MeshData cylinderData(int segments = 24, float topScale = 1.0f);  // topScale < 1: tapered frustum
MeshData sphereData(int stacks = 16, int slices = 24);
MeshData coneData(int segments = 24);
MeshData quadData();                     // XY plane, normal +Z
MeshData gridData(int subdivisions);     // XZ plane, normal +Y
MeshData wedgeData(float topFraction);   // vertical back face at z=-0.5, sloped front face

// N_v = normalize(sum of adjacent face normals), area-weighted.
void computeSmoothNormals(std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices);
