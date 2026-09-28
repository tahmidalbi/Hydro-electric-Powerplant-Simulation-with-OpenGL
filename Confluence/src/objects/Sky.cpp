#include "Sky.h"
#include "Layout.h"
#include <glad/gl.h>

Sky::Sky() : shader("shaders/sky.vert", "shaders/sky.frag") {
    glGenVertexArrays(1, &vao);   // core profile needs a bound VAO even without attributes
}

Sky::~Sky() { glDeleteVertexArrays(1, &vao); }

void Sky::draw(const glm::mat4& view, const glm::mat4& projection, const glm::vec3& cameraPos) const {
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);

    shader.use();
    shader.setMat4("uInvViewProj", glm::inverse(projection * view));
    shader.setVec3("uCamPos", cameraPos);
    shader.setVec3("uHorizon", Layout::SKY_HORIZON);
    shader.setVec3("uZenith", Layout::SKY_ZENITH);
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);

    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);
}
