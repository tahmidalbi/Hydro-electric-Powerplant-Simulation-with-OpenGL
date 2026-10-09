#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <glm/glm.hpp>

// Loads, compiles and links a GLSL program, and exposes small typed setters
// so call sites never touch raw glUniform* calls.
//
// Optional extras:
//   geometryPath - a geometry shader between the vertex and fragment stages
//                  (the strict Flat shading program uses one).
//   commonPaths  - files of shared GLSL code (lighting_common.glsl,
//                  surface_common.glsl). GLSL 3.30 has no #include, so their
//                  text is pasted into every stage right after the "#version"
//                  line, in the order given. One formula, used by every stage.
//
// Uniform locations are looked up once by name and remembered (a cache),
// because the scene sets thousands of uniforms per frame.
class Shader {
public:
    Shader(const std::string& vertexPath, const std::string& fragmentPath,
           const std::string& geometryPath = "", const std::vector<std::string>& commonPaths = {});
    ~Shader();

    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;

    void use() const;

    void setBool(const std::string& name, bool value) const;
    void setInt(const std::string& name, int value) const;
    void setFloat(const std::string& name, float value) const;
    void setVec2(const std::string& name, const glm::vec2& value) const;
    void setVec3(const std::string& name, const glm::vec3& value) const;
    void setMat3(const std::string& name, const glm::mat3& mat) const;
    void setMat4(const std::string& name, const glm::mat4& mat) const;

    unsigned int id() const { return ID; }

private:
    unsigned int ID;
    mutable std::unordered_map<std::string, int> locationCache;
    int location(const std::string& name) const;
    static unsigned int compile(unsigned int type, const std::string& source, const std::string& path);
};
