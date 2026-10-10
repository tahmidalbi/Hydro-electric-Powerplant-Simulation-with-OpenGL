#include "Shader.h"
#include <glad/gl.h>
#include <fstream>
#include <sstream>
#include <iostream>

std::string Shader::shaderFolder = "shaders/";

// Read a text file and expand  #include "file"  lines (one level is enough).
static std::string readSource(const std::string& file, int depth = 0) {
    std::ifstream in(Shader::shaderFolder + file);
    if (!in) {
        std::cerr << "[Shader] cannot open " << Shader::shaderFolder + file << "\n";
        return "";
    }
    std::stringstream out;
    std::string line;
    while (std::getline(in, line)) {
        if (line.rfind("#include", 0) == 0 && depth < 3) {
            size_t a = line.find('"'), b = line.rfind('"');
            out << readSource(line.substr(a + 1, b - a - 1), depth + 1) << "\n";
        } else {
            out << line << "\n";
        }
    }
    return out.str();
}

static unsigned int compile(GLenum type, const std::string& src, const std::string& name) {
    unsigned int s = glCreateShader(type);
    const char* c = src.c_str();
    glShaderSource(s, 1, &c, nullptr);
    glCompileShader(s);
    int ok = 0;
    glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[4096];
        glGetShaderInfoLog(s, sizeof(log), nullptr, log);
        std::cerr << "[Shader] compile error in " << name << ":\n" << log << "\n";
    }
    return s;
}

bool Shader::load(const std::string& vertFile, const std::string& fragFile) {
    unsigned int vs = compile(GL_VERTEX_SHADER, readSource(vertFile), vertFile);
    unsigned int fs = compile(GL_FRAGMENT_SHADER, readSource(fragFile), fragFile);
    id = glCreateProgram();
    glAttachShader(id, vs);
    glAttachShader(id, fs);
    glLinkProgram(id);
    glDeleteShader(vs);
    glDeleteShader(fs);
    int ok = 0;
    glGetProgramiv(id, GL_LINK_STATUS, &ok);
    if (!ok) {
        char log[4096];
        glGetProgramInfoLog(id, sizeof(log), nullptr, log);
        std::cerr << "[Shader] link error (" << vertFile << " + " << fragFile << "):\n" << log << "\n";
        return false;
    }
    return true;
}

void Shader::use() const { glUseProgram(id); }

void Shader::set(const char* n, int v) const          { glUniform1i(glGetUniformLocation(id, n), v); }
void Shader::set(const char* n, float v) const        { glUniform1f(glGetUniformLocation(id, n), v); }
void Shader::set(const char* n, const Vec2& v) const  { glUniform2f(glGetUniformLocation(id, n), v.x, v.y); }
void Shader::set(const char* n, const Vec3& v) const  { glUniform3f(glGetUniformLocation(id, n), v.x, v.y, v.z); }
void Shader::set(const char* n, const Vec4& v) const  { glUniform4f(glGetUniformLocation(id, n), v.x, v.y, v.z, v.w); }
void Shader::set(const char* n, const Mat4& m) const  { glUniformMatrix4fv(glGetUniformLocation(id, n), 1, GL_FALSE, m.m); }
void Shader::setArray(const char* n, const Vec3* v, int count) const {
    glUniform3fv(glGetUniformLocation(id, n), count, &v[0].x);
}
