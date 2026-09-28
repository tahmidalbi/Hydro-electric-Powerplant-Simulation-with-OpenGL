#pragma once
#include <glm/glm.hpp>

class Shader;
class Mesh;
struct Material;
struct Light;

// Sets uModel/uNormalMatrix/material uniforms for one draw and issues it.
// Every object file funnels its geometry through this single call site.
void drawMesh(const Shader& shader, const Mesh& mesh, const glm::mat4& model, const Material& mat);

// Uploads one entry of the shader's `uLights[2]` array.
void setLightUniforms(const Shader& shader, int index, const Light& light);
