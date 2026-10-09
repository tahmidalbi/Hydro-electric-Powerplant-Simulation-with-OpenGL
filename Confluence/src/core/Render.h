#pragma once
#include <glm/glm.hpp>

class Shader;
class Mesh;
struct Material;
struct Light;

// Sets uModel/uNormalMatrix/material uniforms for one draw and issues it.
// Every object file funnels its geometry through this single call site.
// The normal matrix is the correct one: (M^-1)^T  (see Transform::normalMatrix).
void drawMesh(const Shader& shader, const Mesh& mesh, const glm::mat4& model, const Material& mat);

// Same, but the caller chooses the normal matrix. Only the gallery's
// "wrong normal matrix" demonstration (M key) uses this.
void drawMeshWithNormalMatrix(const Shader& shader, const Mesh& mesh, const glm::mat4& model,
                              const glm::mat3& normalMatrix, const Material& mat);

// Step 6: switches the moving-water streaks on for the next draws.
//   flowMode 0 = off, 1 = falling sheet, 2 = river surface. Always reset to 0 afterwards.
void setFlowUniforms(const Shader& shader, int flowMode, float speed);

// Uploads one entry of the shader's `uLights[MAX_LIGHTS]` array.
void setLightUniforms(const Shader& shader, int index, const Light& light);
