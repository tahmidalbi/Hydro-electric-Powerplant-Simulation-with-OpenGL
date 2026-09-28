#include "Render.h"
#include "Shader.h"
#include "Mesh.h"
#include "Material.h"
#include "Light.h"
#include <glm/gtc/matrix_inverse.hpp>
#include <string>

void drawMesh(const Shader& shader, const Mesh& mesh, const glm::mat4& model, const Material& mat) {
    shader.setMat4("uModel", model);
    glm::mat3 normalMatrix = glm::mat3(glm::transpose(glm::inverse(model)));
    shader.setMat3("uNormalMatrix", normalMatrix);

    shader.setVec3("uKa", mat.ka);
    shader.setVec3("uKd", mat.kd);
    shader.setVec3("uKs", mat.ks);
    shader.setVec3("uEmission", mat.emission);
    shader.setFloat("uShininess", mat.shininess);

    mesh.draw();
}

void setLightUniforms(const Shader& shader, int index, const Light& light) {
    std::string p = "uLights[" + std::to_string(index) + "].";
    shader.setInt(p + "type", light.type);
    shader.setVec3(p + "direction", light.direction);
    shader.setVec3(p + "position", light.position);
    shader.setVec3(p + "diffuse", light.diffuse);
    shader.setVec3(p + "specular", light.specular);
    shader.setVec3(p + "attenuation", light.attenuation);
    shader.setFloat(p + "enabled", light.enabled);
}
