#include "Scene6_Penstock.h"
#include "../objects/Materials.h"
#include <cstdio>

static const float TOP_Y = 38.0f;   // pipe centre at the intake
static const float R = 2.4f;        // pipe radius

void Scene6_Penstock::build() {
    // intake (top left) -> steep run down the dam -> bend -> turbine casing (right)
    std::vector<Vec3> corners = {Vec3(-46, TOP_Y, 0), Vec3(-24, TOP_Y, 0), Vec3(22, 6.0f, 0), Vec3(34, 4.0f, 0), Vec3(47, 4.0f, 0)};
    pipe.build(corners, R, 9.0f, normalize(Vec3(0, 1.0f, 0.9f)), 78.0f, 7.5f);

    MeshBuilder b;
    // sloped downstream face of the dam behind the pipe
    {
        std::vector<Vec2> outline = {Vec2(-60, 0), Vec2(-60, 52), Vec2(-34, 52), Vec2(30, 0)};
        // re-use the extrude helper: outline is (z, y) -> here we build it along x by rotating afterwards
        b.setTransform(translate(Vec3(0, 0, -9.0f)) * rotateY(PI * 0.5f));
        b.extrudeOutlineX(outline, -4.0f, 30.0f, -1.0f);
        b.resetTransform();
    }
    damFace = b.build();

    b.clear();     // buttress fins and walkways on the dam face (diagonal structures in the video)
    for (float x = -30; x <= 20; x += 12.5f) {
        float yTop = 52.0f - (x + 34.0f) * 52.0f / 64.0f;
        b.beam(Vec3(x, std::fmax(yTop, 0.5f) - 1.0f, -4.3f), Vec3(x + 9.0f, 0.5f, -4.3f), 2.2f, 1.6f);
    }
    buttresses = b.build();
    b.clear();
    for (int k = 0; k < 3; k++) {
        float y = 14.0f + k * 12.0f;
        float x0 = (52.0f - y) * 64.0f / 52.0f - 34.0f;   // where the slope is at height y
        b.box(Vec3(x0 - 18, y, -5.0f), Vec3(x0 - 2, y + 0.4f, -2.6f));
    }
    walkways = b.build();
    b.clear();
    for (int k = 0; k < 3; k++) {
        float y = 14.0f + k * 12.0f, x0 = (52.0f - y) * 64.0f / 52.0f - 34.0f;
        for (float x = x0 - 18; x <= x0 - 2; x += 1.5f) b.box(Vec3(x, y + 0.4f, -2.7f), Vec3(x + 0.06f, y + 1.5f, -2.64f));
        b.box(Vec3(x0 - 18, y + 1.45f, -2.72f), Vec3(x0 - 2, y + 1.55f, -2.62f));
    }
    rails = b.build();

    b.clear();
    b.box(Vec3(-120, -2, -40), Vec3(120, 0, 40));
    ground = b.build();

    // intake tower at the top left (pipe starts in it)
    b.clear();
    b.box(Vec3(-60, 30, -12), Vec3(-43, 52, -4));
    b.box(Vec3(-60, 30, 4), Vec3(-43, 52, 9));
    b.box(Vec3(-60, 41, -4), Vec3(-43, 52, 4));
    b.box(Vec3(-60, 0, -12), Vec3(-43, 34.5f, 9));
    b.box(Vec3(-58, 52, -10), Vec3(-46, 57, 6));                    // gate house on top
    b.box(Vec3(-43, 30, -12), Vec3(-30, 31, 9));                     // ledge
    intakeBlock = b.build();
    b.clear();
    b.setTransform(translate(Vec3(-110, 49, 0)));
    b.grid(100, 120, 4, 4);
    reservoir = b.build();

    // concrete saddles under the pipe every 7.5 m
    b.clear();
    for (float s = 6.0f; s < pipe.length - 8.0f; s += 7.5f) {
        Vec3 p = pipe.pointAt(s);
        if (p.x < -42) continue;
        float bottom = p.y - R - 0.2f;
        b.box(Vec3(p.x - 1.3f, 0, -2.2f), Vec3(p.x + 1.3f, bottom, 2.2f));
    }
    saddles = b.build();

    // spiral casing / turbine housing at the end: big drum with radial ribs
    b.clear();
    Vec3 c(47, 4.0f, 0);
    b.setTransform(translate(c) * rotateZ(-PI * 0.5f));
    b.cylinder(6.5f, 6.5f, 4.5f, 64, false, false);
    b.annulus(R, 6.5f, 64);
    b.setTransform(translate(c + Vec3(4.5f, 0, 0)) * rotateZ(-PI * 0.5f));
    b.disk(6.5f, 64);
    b.setTransform(translate(c - Vec3(0.3f, 0, 0)) * rotateZ(-PI * 0.5f));
    b.cylinder(7.0f, 7.0f, 0.6f, 64, true, true);
    b.setTransform(translate(c + Vec3(4.2f, 0, 0)) * rotateZ(-PI * 0.5f));
    b.cylinder(7.0f, 7.0f, 0.6f, 64, true, true);
    b.setTransform(translate(c - Vec3(1.4f, 0, 0)) * rotateZ(-PI * 0.5f));
    b.cylinder(R * 1.25f, R * 1.25f, 1.4f, 48, false, false);
    b.resetTransform();
    b.box(Vec3(44, -0.5f, -5), Vec3(53, c.y - 4.5f, 5));             // foundation
    casing = b.build();
    b.clear();
    for (int k = 0; k < 16; k++) {
        float a = k * 2 * PI / 16;
        b.setTransform(translate(c - Vec3(0.35f, 0, 0)) * rotateX(a));
        b.box(Vec3(-0.25f, R * 1.25f, -0.18f), Vec3(0.25f, 6.6f, 0.18f));
    }
    b.resetTransform();
    casingRibs = b.build();

    // small machine on a platform (top right, like the video)
    b.clear();
    b.box(Vec3(26, 18, -16), Vec3(40, 18.6f, -6));
    for (int i = 0; i < 4; i++) b.box(Vec3(26.2f + (i % 2) * 13.2f, 0, -15.8f + (i / 2) * 9.4f), Vec3(26.6f + (i % 2) * 13.2f, 18, -15.4f + (i / 2) * 9.4f));
    b.setTransform(translate(Vec3(30, 20.2f, -11)) * rotateZ(-PI * 0.5f));
    b.cylinder(1.6f, 1.6f, 5, 24, true, true);
    b.resetTransform();
    machine = b.build();
}

void Scene6_Penstock::enter() {
    t = 0;
    front = 0;
    xray = 0;
    xrayManual = false;
    xrayOn = false;
    arrivedTime = 0;
    next = false;
}

float Scene6_Penstock::speedAt(float s) const {
    float y = pipe.pointAt(std::fmax(s, 0.0f)).y;
    return std::sqrt(144.0f + 2.0f * 9.81f * std::fmax(TOP_Y - y, 0.0f));   // enters at 12 m/s
}

void Scene6_Penstock::update(float dt) {
    t += dt;
    if (t > 0.6f) front = std::fmin(front + speedAt(front) * dt * SLOW_MOTION, pipe.length + 4.0f);
    // the hologram x-ray look switches on when the water is half way down
    if (!xrayManual) xrayOn = front > pipe.length * 0.55f;
    xray += clampf((xrayOn ? 1.0f : 0.0f) - xray, -dt * 1.2f, dt * 1.2f);
    if (front >= pipe.length) arrivedTime += dt;
}

// The camera chases the water front, then pulls back to show the bend + casing.
void Scene6_Penstock::camera(Camera& cam) {
    float k = clampf(front / pipe.length, 0.0f, 1.0f);
    Vec3 f = pipe.pointAt(std::fmin(front, pipe.length - 2.0f));
    Vec3 chaseEye = f + Vec3(7.0f, 6.0f, 15.0f);
    Vec3 wideEye(18, 16, 44), wideTarget(26, 6, 0);
    float w = smoothstepf(0.62f, 1.0f, k);
    if (t < 0.6f) { chaseEye = pipe.pointAt(0) + Vec3(7.0f, 6.0f, 15.0f); f = pipe.pointAt(2); }
    cam.eye = mix(chaseEye, wideEye, w);
    cam.target = mix(f + Vec3(4, -1.5f, 0), wideTarget, w);
    cam.fovDeg = 52.0f;
}

void Scene6_Penstock::lighting(Lighting& L) {
    L.sunDir = normalize(Vec3(-0.5f, 0.85f, 0.45f));
    L.sunColor = Vec3(2.3f, 2.25f, 2.15f);
    L.skyTop = Vec3(0.45f, 0.55f, 0.66f);
    L.skyHorizon = Vec3(0.7f, 0.73f, 0.76f);
    L.ground = Vec3(0.3f, 0.3f, 0.3f);
    L.ambient = 0.6f;
    L.fogColor = Vec3(0.66f, 0.69f, 0.72f);
    L.fogDensity = 0.004f;
    L.skyMode = 1;
    L.exposure = 0.4f;
}

void Scene6_Penstock::drawOpaque(Renderer& r) {
    Mat4 I;
    Material conc = Mat::concrete(1.0f, Vec3(0.55f, 0.55f, 0.54f));
    r.drawMesh(damFace, I, conc);
    r.drawMesh(buttresses, I, conc);
    r.drawMesh(walkways, I, conc);
    r.drawMesh(rails, I, Mat::steel(Vec3(0.7f, 0.7f, 0.7f)));
    r.drawMesh(ground, I, Mat::concrete(1.0f, Vec3(0.5f, 0.5f, 0.49f)));
    r.drawMesh(intakeBlock, I, conc);
    r.drawMesh(saddles, I, Mat::concrete(1.5f, Vec3(0.58f, 0.58f, 0.57f)));
    Material steel = Mat::steel(Vec3(0.36f, 0.38f, 0.41f));
    pipe.drawShell(r, steel);
    r.drawMesh(casing, I, Mat::steel(Vec3(0.4f, 0.42f, 0.45f)));
    r.drawMesh(casingRibs, I, Mat::steel(Vec3(0.35f, 0.37f, 0.4f)));
    r.drawMesh(machine, I, Mat::paint(Vec3(0.5f, 0.52f, 0.55f)));
}

void Scene6_Penstock::drawTransparent(Renderer& r) {
    WaterStyle res;
    res.mode = 0;
    res.alpha = 0.95f;
    res.ripple = 0.4f;
    r.drawWater(reservoir, Mat4(), res);

    WaterStyle w;
    w.deep = Vec3(0.02f, 0.13f, 0.24f);
    w.shallow = Vec3(0.1f, 0.42f, 0.62f);
    w.alpha = 0.85f;
    w.foam = 0.55f;
    w.flowSpeed = 6.0f + speedAt(front) * 0.6f;
    w.ripple = 0.8f;
    w.front = front;
    w.noseLen = 3.0f;
    w.glow = xray * 1.4f;
    pipe.drawWater(r, w);
    pipe.drawHolo(r, xray);
}

void Scene6_Penstock::hud(Ui& ui, const Mat4& vp) {
    // a tag that rides along with the water front
    float x, y;
    Vec3 p = pipe.pointAt(std::fmin(front, pipe.length)) - Vec3(0, R + 0.5f, 0);
    if (front > 1.0f && front < pipe.length + 1.0f && projectToScreen(vp, p, ui.W, ui.H, x, y)) {
        char buf[64];
        std::snprintf(buf, sizeof(buf), "V = %.1F M/S", speedAt(front));
        const Vec4 cyan(0.45f, 0.88f, 1.0f, 0.95f);
        ui.line(x, y, x + 40, y + 50, 1.5f, cyan);
        ui.rect(x + 40, y + 50, ui.textWidth(buf, 2) + 16, 30, Vec4(0.02f, 0.06f, 0.1f, 0.75f));
        ui.text(x + 48, y + 56, buf, 2, Vec4(1, 1, 1, 1));
    }
}

void Scene6_Penstock::telemetry(std::vector<Telemetry>& out) {
    char buf[64];
    float y = pipe.pointAt(std::fmin(front, pipe.length)).y;
    std::snprintf(buf, sizeof(buf), "%.1f M", TOP_Y - y);
    out.push_back({"DROP SO FAR (H)", buf});
    std::snprintf(buf, sizeof(buf), "%.1f M/S", speedAt(front));
    out.push_back({"FLOW SPEED", buf});
    std::snprintf(buf, sizeof(buf), "%.1f BAR", 1.0f + (TOP_Y - y + 14.0f) / 10.2f);
    out.push_back({"PIPE PRESSURE", buf});
    std::snprintf(buf, sizeof(buf), "%d %%", (int)(clampf(front / pipe.length, 0, 1) * 100));
    out.push_back({"PENSTOCK FILLED", buf});
    out.push_back({"PLAYBACK", "SLOW MOTION 0.6X"});
}

void Scene6_Penstock::buttons(std::vector<Button>& out) {
    out.push_back({xrayOn ? "X-RAY OFF" : "X-RAY VIEW", 1, "X", false});
    out.push_back({"REPLAY FLOW", 2, "SPACE", false});
    out.push_back({"HIGH-SPEED VIEW >", 3, "ENTER", true});
}

void Scene6_Penstock::onButton(int id) {
    if (id == 1) { xrayManual = true; xrayOn = !xrayOn; }
    if (id == 2) enter();
    if (id == 3) next = true;
}
