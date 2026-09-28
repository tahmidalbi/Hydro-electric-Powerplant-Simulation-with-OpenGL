#pragma once
#include <string>
#include <glm/glm.hpp>

// Loads, compiles and links a vertex+fragment GLSL program, and exposes
// small typed setters so call sites never touch raw glUniform* calls.
class Shader {
public:
    Shader(const std::string& vertexPath, const std::string& fragmentPath);
    ~Shader();

    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;

    void use() const;

    void setBool(const std::string& name, bool value) const;
    void setInt(const std::string& name, int value) const;
    void setFloat(const std::string& name, float value) const;
    void setVec3(const std::string& name, const glm::vec3& value) const;
    void setMat3(const std::string& name, const glm::mat3& mat) const;
    void setMat4(const std::string& name, const glm::mat4& mat) const;

    unsigned int id() const { return ID; }

private:
    unsigned int ID;
    static unsigned int compile(unsigned int type, const std::string& source, const std::string& path);
};
