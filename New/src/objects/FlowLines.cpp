#include "FlowLines.h"

void FlowLines::add(const std::vector<Vec3>& path, float radius) {
    if (path.size() < 2) return;
    b.sweep(path, radius, 4, 0.0f, 2.0f * PI, false);      // thin 4-sided tube, uv.y = distance along
    float len = 0;
    for (size_t i = 1; i < path.size(); i++) len += length(path[i] - path[i - 1]);
    if (len > longest) longest = len;
}

void FlowLines::draw(Renderer& r, const Mat4& model, const Vec3& color, float strength,
                     float speed, float spacing, float front) const {
    if (strength <= 0.01f || r.inShadowPass()) return;
    Material m;
    m.color = Vec3(0, 0, 0);
    m.pattern = PAT_FLOW;
    m.patternScale = 1.0f / spacing;
    m.emissive = color * strength;
    m.alpha = std::fmin(strength, 1.0f);
    m.additive = true;
    m.castShadow = false;
    m.twoSided = true;
    m.flowSpeed = speed;
    m.flowFront = front;
    r.drawMesh(mesh, model, m);
}
