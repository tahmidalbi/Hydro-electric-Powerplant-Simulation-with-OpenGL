#include "Shader.h"
#include <glad/gl.h>
#include <glm/gtc/type_ptr.hpp>

#include <fstream>
#include <sstream>
#include <iostream>
#include <stdexcept>

static std::string readFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open())
        throw std::runtime_error("Shader: could not open file " + path);
    std::stringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

// Pastes the common files right after the first line ("#version 330 core").
// The #line directives keep compiler error messages readable:
//   "1(25)" = line 25 of the first common file, "2(..)" of the second, ...
//   "0(12)" = line 12 of the stage's own file (phong.vert, ...)
static std::string injectCommon(const std::string& source, const std::vector<std::string>& commons) {
    if (commons.empty()) return source;
    size_t endOfVersionLine = source.find('\n');
    if (endOfVersionLine == std::string::npos)
        throw std::runtime_error("Shader: source has no #version line");
    std::string result = source.substr(0, endOfVersionLine + 1);
    for (size_t i = 0; i < commons.size(); ++i)
        result += "#line 1 " + std::to_string(i + 1) + "\n" + commons[i] + "\n";
    result += "#line 2 0\n" + source.substr(endOfVersionLine + 1);
    return result;
}

unsigned int Shader::compile(unsigned int type, const std::string& source, const std::string& path) {
    unsigned int shader = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    int success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char log[4096];
        glGetShaderInfoLog(shader, sizeof(log), nullptr, log);
        std::cerr << "Shader compile error (" << path << "):\n" << log << std::endl;
        throw std::runtime_error("Shader compilation failed: " + path);
    }
    return shader;
}

Shader::Shader(const std::string& vertexPath, const std::string& fragmentPath,
               const std::string& geometryPath, const std::vector<std::string>& commonPaths) {
    std::vector<std::string> commons;
    for (const std::string& path : commonPaths) commons.push_back(readFile(path));

    unsigned int vs = compile(GL_VERTEX_SHADER, injectCommon(readFile(vertexPath), commons), vertexPath);
    unsigned int fs = compile(GL_FRAGMENT_SHADER, injectCommon(readFile(fragmentPath), commons), fragmentPath);
    unsigned int gs = 0;
    if (!geometryPath.empty())
        gs = compile(GL_GEOMETRY_SHADER, injectCommon(readFile(geometryPath), commons), geometryPath);

    ID = glCreateProgram();
    glAttachShader(ID, vs);
    if (gs) glAttachShader(ID, gs);
    glAttachShader(ID, fs);
    glLinkProgram(ID);

    int success = 0;
    glGetProgramiv(ID, GL_LINK_STATUS, &success);
    if (!success) {
        char log[4096];
        glGetProgramInfoLog(ID, sizeof(log), nullptr, log);
        std::cerr << "Shader link error (" << vertexPath << " + " << fragmentPath << "):\n" << log << std::endl;
        throw std::runtime_error("Shader linking failed");
    }

    glDeleteShader(vs);
    glDeleteShader(fs);
    if (gs) glDeleteShader(gs);
}

Shader::~Shader() {
    glDeleteProgram(ID);
}

void Shader::use() const { glUseProgram(ID); }

int Shader::location(const std::string& name) const {
    auto it = locationCache.find(name);
    if (it != locationCache.end()) return it->second;
    int loc = glGetUniformLocation(ID, name.c_str());   // -1 if unused: glUniform ignores -1
    locationCache.emplace(name, loc);
    return loc;
}

void Shader::setBool(const std::string& name, bool value) const { glUniform1i(location(name), (int)value); }
void Shader::setInt(const std::string& name, int value) const { glUniform1i(location(name), value); }
void Shader::setFloat(const std::string& name, float value) const { glUniform1f(location(name), value); }
void Shader::setVec2(const std::string& name, const glm::vec2& value) const { glUniform2fv(location(name), 1, glm::value_ptr(value)); }
void Shader::setVec3(const std::string& name, const glm::vec3& value) const { glUniform3fv(location(name), 1, glm::value_ptr(value)); }
void Shader::setMat3(const std::string& name, const glm::mat3& mat) const {
    glUniformMatrix3fv(location(name), 1, GL_FALSE, glm::value_ptr(mat));
}
void Shader::setMat4(const std::string& name, const glm::mat4& mat) const {
    glUniformMatrix4fv(location(name), 1, GL_FALSE, glm::value_ptr(mat));
}
