// =============================================================================
//  Shader.h  -  loads a GLSL vertex + fragment shader pair from the shaders/
//  folder, compiles and links them, and offers simple uniform setters.
//
//  Small extra feature: a line   #include "common.glsl"   inside a shader is
//  replaced by that file's text, so lighting code is written only once.
// =============================================================================
#pragma once
#include <string>
#include "MathLib.h"

class Shader {
public:
    unsigned int id = 0;

    bool load(const std::string& vertFile, const std::string& fragFile);
    void use() const;

    void set(const char* name, int v) const;
    void set(const char* name, float v) const;
    void set(const char* name, const Vec2& v) const;
    void set(const char* name, const Vec3& v) const;
    void set(const char* name, const Vec4& v) const;
    void set(const char* name, const Mat4& m) const;
    void setArray(const char* name, const Vec3* v, int count) const;

    static std::string shaderFolder;   // "shaders/" next to the .exe
};
