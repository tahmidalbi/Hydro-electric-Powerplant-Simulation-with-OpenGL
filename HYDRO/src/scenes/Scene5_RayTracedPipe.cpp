#include "Scene5_RayTracedPipe.h"
#include "../objects/PlantData.h"
#include <cstdio>

static const float PIPE_R = 2.0f;

void Scene5_RayTracedPipe::build() {
    rt.load("fullscreen.vert", "raytrace.frag");
    pipeDir = normalize(Vec3(1.0f, -0.1f, 0.0f));     // gently downhill, flow towards +x
}

void Scene5_RayTracedPipe::enter(bool) {
    t = 0;
    restart = false;
    flowVisual = 0;
    flowOn = 0;
    travelled = -34.0f;
}

void Scene5_RayTracedPipe::update(float dt) {
    t += dt;
    flowOn = std::fmin(flowOn + dt * 0.8f, 1.0f);
    // the streaks are shown at 1/3 of the real speed so the eye can follow them
    float target = speed() / 3.0f;
    flowVisual += (target - flowVisual) * std::fmin(dt * 1.5f, 1.0f);
    travelled += dt * 3.2f;                          // the camera glides along the pipe
}

Vec3 alongPipe(const Vec3& a, const Vec3& d, float s) { return a + d * s; }

void Scene5_RayTracedPipe::camera(Camera& cam) {
    // side-high tracking shot, slowly turning, like the video
    float s = travelled;
    Vec3 p = alongPipe(pipeA, pipeDir, s);
    float swing = std::sin(t * 0.25f) * 1.5f;
    cam.eye = p + Vec3(-3.0f + swing, 5.2f, 8.5f);
    cam.target = alongPipe(pipeA, pipeDir, s + 9.0f) + Vec3(0, -0.6f, 0);
    cam.fovDeg = 55.0f;
}

void Scene5_RayTracedPipe::lighting(Lighting& L, const Vec3&) {
    L.exposure = 0.62f;
    L.shadows = false;          // the ray tracer does its own shadow rays
}

void Scene5_RayTracedPipe::drawOpaque(Renderer& r) {
    if (r.inShadowPass()) return;
    r.applyCommon(rt);          // camera, time, exposure ...
    rt.use();
    rt.set("uPipeA", pipeA);
    rt.set("uPipeDir", pipeDir);
    rt.set("uPipeR", PIPE_R);
    rt.set("uFlowSpeed", flowVisual);
    rt.set("uFlowOn", flowOn);
    rt.set("uScan", travelled - 10.0f + std::fmod(t * 22.0f, 60.0f));
    r.drawFullscreen();
}

void Scene5_RayTracedPipe::hud(Ui& ui, const Mat4& vp) {
    const Vec4 cyan(0.45f, 0.88f, 1.0f, 0.9f), white(1, 1, 1, 0.95f), faint(0.45f, 0.88f, 1.0f, 0.18f);
    // hologram grid overlay
    for (int i = 1; i < 12; i++) ui.rect(ui.W * i / 12.0f, 30, 1, ui.H - 60.0f, faint);
    for (int i = 1; i < 7; i++) ui.rect(0, 30 + (ui.H - 60.0f) * i / 7.0f, (float)ui.W, 1, faint);

    // callouts attached to the pipe
    auto tag = [&](float s, float dx, float dy, const std::string& txt) {
        float x, y;
        Vec3 p = alongPipe(pipeA, pipeDir, s) - Vec3(0, PIPE_R * 0.6f, -PIPE_R * 0.8f);
        if (!projectToScreen(vp, p, ui.W, ui.H, x, y)) return;
        float w0 = ui.textWidth(txt, 1.5f) + 16;
        if (x < 40 || x + dx + w0 > ui.W - 20 || y + dy < 220 || y + dy > ui.H - 150) return;
        ui.rect(x - 3, y - 3, 6, 6, cyan);
        ui.line(x, y, x + dx, y + dy, 1.5f, cyan);
        float w = ui.textWidth(txt, 1.5f) + 16;
        ui.rect(x + dx, y + dy - 11, w, 22, Vec4(0.02f, 0.06f, 0.1f, 0.75f));
        ui.outline(x + dx, y + dy - 11, w, 22, 1, cyan);
        ui.text(x + dx + 8, y + dy - 5, txt, 1.5f, white);
    };
    char buf[64];
    std::snprintf(buf, sizeof(buf), "FLOW %.1f M/S", speed());
    tag(travelled + 5.0f, 30, 80, buf);
    std::snprintf(buf, sizeof(buf), "PRESSURE %.1f BAR", 1.0f + head / 10.2f);
    tag(travelled + 13.0f, 40, 110, buf);
    tag(travelled + 21.0f, 30, 60, "GLASS SECTION - RAY TRACED");
}

void Scene5_RayTracedPipe::telemetry(std::vector<Telemetry>& out) {
    char buf[64];
    std::snprintf(buf, sizeof(buf), "%.0f M", head);
    out.push_back({"HEAD  H", buf});
    std::snprintf(buf, sizeof(buf), "%.1f M/S", speed());
    out.push_back({"SPEED  SQRT(2GH)", buf});
    std::snprintf(buf, sizeof(buf), "%.0f KM/H", speed() * 3.6f);
    out.push_back({"", buf});
    // same data sheet as the rest of the plant: Q grows with sqrt(H)
    float q = Plant::Q_UNIT * std::sqrt(head / Plant::HEAD);
    std::snprintf(buf, sizeof(buf), "%.0f M3/S", q);
    out.push_back({"DISCHARGE", buf});
    std::snprintf(buf, sizeof(buf), "%.0f MW", Plant::powerMW(q, head));
    out.push_back({"POWER  P = RHO G Q H ETA", buf});
}

void Scene5_RayTracedPipe::buttons(std::vector<Button>& out) {
    out.push_back({"RAISE HEAD +10 M", 1, "UP", false});
    out.push_back({"LOWER HEAD -10 M", 2, "DOWN", false});
    out.push_back({"FOLLOW TO MAIN INLET VALVE >", 3, "ENTER", true});
}

void Scene5_RayTracedPipe::onButton(int id) {
    if (id == 1) head = std::fmin(head + 10.0f, 150.0f);
    if (id == 2) head = std::fmax(head - 10.0f, 10.0f);
    if (id == 3) restart = true;
}
