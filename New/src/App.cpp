#include "App.h"
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <cstdio>
#include <iostream>
#include "core/Screenshot.h"
#include "scenes/Scene1_Spillway.h"
#include "scenes/Scene2_DamSurvey.h"
#include "scenes/Scene3_Underwater.h"
#include "scenes/Scene4_TrashRack.h"
#include "scenes/Scene5_IntakeGate.h"
#include "scenes/Scene6_Penstock.h"
#include "scenes/Scene7_RayTracedFlow.h"

// global navigation buttons (bottom right)
enum { GBTN_PREV = 1001, GBTN_NEXT = 1002, GBTN_AUTO = 1003, GBTN_SHADE = 1004 };

static App* gApp = nullptr;
static void keyCb(GLFWwindow*, int key, int, int action, int) { gApp->onKey(key, action); }
static void mouseCb(GLFWwindow*, int b, int action, int) { gApp->onMouseButton(b, action); }
static void cursorCb(GLFWwindow*, double x, double y) { gApp->onCursor(x, y); }
static void scrollCb(GLFWwindow*, double, double dy) { gApp->onScroll(dy); }
static void sizeCb(GLFWwindow*, int w, int h) { gApp->onResize(w, h); }

bool App::init(int width, int height, bool visible) {
    gApp = this;
    W = width;
    H = height;
    if (!glfwInit()) return false;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);                      // anti-aliasing
    glfwWindowHint(GLFW_VISIBLE, visible ? GLFW_TRUE : GLFW_FALSE);
    glfwWindowHint(GLFW_RESIZABLE, visible ? GLFW_TRUE : GLFW_FALSE);
    window = glfwCreateWindow(W, H, "HYDRO // Dam to Turbine - OpenGL", nullptr, nullptr);
    if (!window) return false;
    glfwMakeContextCurrent(window);
    if (!gladLoadGL((GLADloadfunc)glfwGetProcAddress)) return false;
    glfwSwapInterval(1);
    glfwSetKeyCallback(window, keyCb);
    glfwSetMouseButtonCallback(window, mouseCb);
    glfwSetCursorPosCallback(window, cursorCb);
    glfwSetScrollCallback(window, scrollCb);
    glfwSetFramebufferSizeCallback(window, sizeCb);
    glfwGetFramebufferSize(window, &W, &H);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
    glDisable(GL_CULL_FACE);       // thin parts (gates, leaves) are seen from both sides

    if (!renderer.init()) std::cerr << "shader problem - see messages above\n";
    if (!ui.init()) std::cerr << "ui shader problem\n";

    std::cout << "Building the world...\n";
    site.build();
    scenes.emplace_back(new Scene1_Spillway(site));
    scenes.emplace_back(new Scene2_DamSurvey(site));
    scenes.emplace_back(new Scene3_Underwater());
    scenes.emplace_back(new Scene4_TrashRack());
    scenes.emplace_back(new Scene5_IntakeGate());
    scenes.emplace_back(new Scene6_Penstock());
    scenes.emplace_back(new Scene7_RayTracedFlow());
    for (auto& s : scenes) s->build();
    scenes[0]->enter();
    fade = 1.0f;
    return true;
}

void App::shutdown() {
    glfwDestroyWindow(window);
    glfwTerminate();
}

// ---------------------------------------------------------------------------
void App::goToScene(int index) {
    if (index < 0 || index >= (int)scenes.size()) return;
    pending = index;
    fadingOut = true;
}

void App::update(float dt) {
    globalTime += dt;
    Scene& s = *scenes[current];
    if (autoPlay) s.autoPlay();
    s.update(dt);

    // fade out -> switch scene -> fade in
    if (fadingOut) {
        fade += dt / 0.45f;
        if (fade >= 1.0f) {
            fade = 1.0f;
            fadingOut = false;
            current = pending;
            pending = -1;
            scenes[current]->enter();
            cam.resetUser();
            freeCamera = false;
        }
    } else if (fade > 0.0f) {
        fade = std::fmax(0.0f, fade - dt / 0.6f);
    }
    // the story moves on by itself when a scene says so
    if (!fadingOut && s.finished()) goToScene(current + 1 < (int)scenes.size() ? current + 1 : 0);

    // free camera panning with W A S D Q E
    if (freeCamera) {
        Vec3 f = normalize(Vec3(renderer.camForward.x, 0, renderer.camForward.z));
        Vec3 rgt = normalize(cross(f, Vec3(0, 1, 0)));
        float sp = 12.0f * dt * std::fmax(cam.zoom, 0.3f);
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) cam.pan += f * sp;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) cam.pan -= f * sp;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) cam.pan += rgt * sp;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) cam.pan -= rgt * sp;
        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) cam.pan.y += sp;
        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) cam.pan.y -= sp;
    }
}

void App::render() {
    Scene& s = *scenes[current];
    // the director's camera for this moment; the user's orbit is added on top
    cam.nearZ = 0.2f;
    cam.farZ = 3000.0f;
    cam.fovDeg = 50.0f;
    s.camera(cam);
    Lighting L;
    s.lighting(L);
    if (!shadowsOn) L.shadows = false;
    renderer.beginFrame(cam, L, globalTime, W, H);

    glViewport(0, 0, W, H);
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (s.rayTraced()) {
        renderer.prepareMainPass();
        s.drawOpaque(renderer);                  // the ray tracer draws everything itself
    } else {
        if (L.shadows) {
            renderer.beginShadowPass(s.shadowCentre(), s.shadowRadius());
            s.drawOpaque(renderer);
            renderer.endShadowPass();
        }
        renderer.prepareMainPass();
        renderer.drawSky();
        s.drawOpaque(renderer);
        s.drawTransparent(renderer);
    }
    drawHud();
}

// ---------------------------------------------------------------------------
//  HUD: title, caption, telemetry, buttons, help text, fade
// ---------------------------------------------------------------------------
void App::drawHud() {
    ui.begin(W, H);
    Scene& s = *scenes[current];
    const Vec4 cyan(0.45f, 0.88f, 1.0f, 1.0f), white(0.95f, 0.97f, 1.0f, 1.0f), panel(0.02f, 0.05f, 0.08f, 0.55f);
    const Vec4 dimCyan(0.45f, 0.88f, 1.0f, 0.35f);

    // cinematic letterbox bars + vignette edges
    ui.rect(0, 0, (float)W, 30, Vec4(0, 0, 0, 0.85f));
    ui.rect(0, H - 30.0f, (float)W, 30, Vec4(0, 0, 0, 0.85f));

    if (showHud) {
        Mat4 vp = renderer.projM * renderer.viewM;
        s.hud(ui, vp);

        // title block (top left)
        char idx[32];
        std::snprintf(idx, sizeof(idx), "SCENE %02d / %02d", current + 1, (int)scenes.size());
        std::string cap = s.caption();
        float panelW = std::fmax(560.0f, ui.textWidth(cap, 1.5f) + 34.0f);
        ui.rect(24, 44, panelW, 86, panel);
        ui.rect(24, 44, 4, 86, cyan);
        ui.text(40, 52, idx, 2, cyan);
        ui.text(40, 72, s.title(), 4, white);
        ui.text(40, 110, cap, 1.5f, Vec4(0.8f, 0.9f, 1.0f, 0.9f));

        // telemetry (top right)
        std::vector<Telemetry> tel;
        s.telemetry(tel);
        if (!tel.empty()) {
            float pw = 330, px = W - pw - 24, py = 44;
            float ph = 34 + tel.size() * 24.0f;
            ui.rect(px, py, pw, ph, panel);
            ui.corners(px, py, pw, ph, 14, 2, cyan);
            ui.text(px + 14, py + 10, "TELEMETRY", 1.5f, cyan);
            for (size_t i = 0; i < tel.size(); i++) {
                float y = py + 32 + i * 24.0f;
                ui.text(px + 14, y, tel[i].label, 1.5f, Vec4(0.7f, 0.82f, 0.9f, 1));
                ui.text(px + pw - 14 - ui.textWidth(tel[i].value, 2), y - 2, tel[i].value, 2, white);
            }
        }

        // help (bottom left)
        const char* help[] = {"MOUSE DRAG: ORBIT   RIGHT DRAG / WASDQE: MOVE   WHEEL: ZOOM",
                              "C: DIRECTOR CAMERA   G: PHONG/GOURAUD   L: SHADOWS   H: HUD   1-7: SCENE   N/B: NEXT/BACK   P: AUTO"};
        ui.text(24, H - 72.0f, help[0], 1.5f, Vec4(0.75f, 0.85f, 0.95f, 0.75f));
        ui.text(24, H - 54.0f, help[1], 1.5f, Vec4(0.75f, 0.85f, 0.95f, 0.75f));

        // scene buttons (bottom centre) + global buttons (bottom right)
        buttonRects.clear();
        std::vector<Button> btns;
        s.buttons(btns);
        float total = 0;
        for (const Button& b : btns) total += ui.textWidth(b.label, 2) + 60 + 16;
        float x = (W - total) * 0.5f + 8, y = H - 128.0f;
        int k = 0;
        for (const Button& b : btns) {
            float w = ui.textWidth(b.label, 2) + 60, h = 44;
            bool hover = hoverButton == k;
            Vec4 fill = b.primary ? Vec4(0.05f, 0.35f, 0.5f, hover ? 0.95f : 0.8f) : Vec4(0.03f, 0.08f, 0.12f, hover ? 0.95f : 0.75f);
            ui.rect(x, y, w, h, fill);
            ui.outline(x, y, w, h, 1, b.primary ? cyan : dimCyan);
            ui.corners(x - 3, y - 3, w + 6, h + 6, 8, 2, cyan);
            ui.text(x + 30, y + 8, b.label, 2, white);
            std::string key = "[" + b.key + "]";
            ui.text(x + (w - ui.textWidth(key, 1)) * 0.5f, y + 31, key, 1, Vec4(0.6f, 0.85f, 1.0f, 0.9f));
            buttonRects.push_back({x, y, w, h, b.id, false});
            x += w + 16;
            k++;
        }
        const char* glabels[] = {"<", ">", autoPlay ? "AUTO: ON" : "AUTO: OFF",
                                 renderer.shadingMode == SHADE_MIXED ? "SHADING: MIXED" :
                                 renderer.shadingMode == SHADE_ALL_PHONG ? "SHADING: PHONG" : "SHADING: GOURAUD"};
        int gids[] = {GBTN_PREV, GBTN_NEXT, GBTN_AUTO, GBTN_SHADE};
        float gx = W - 24.0f;
        for (int i = 3; i >= 0; i--) {
            float w = ui.textWidth(glabels[i], 1.5f) + 24, h = 30;
            gx -= w;
            bool hover = hoverButton == 100 + i;
            ui.rect(gx, H - 72.0f, w, h, Vec4(0.03f, 0.08f, 0.12f, hover ? 0.95f : 0.7f));
            ui.outline(gx, H - 72.0f, w, h, 1, dimCyan);
            ui.text(gx + 12, H - 63.0f, glabels[i], 1.5f, white);
            buttonRects.push_back({gx, H - 72.0f, w, h, gids[i], true});
            gx -= 8;
        }

        // frame rate + mode (top bar)
        char top[96];
        std::snprintf(top, sizeof(top), "HYDRO//SIM   %3.0f FPS   %s", fps, freeCamera ? "FREE CAMERA" : "DIRECTOR CAMERA");
        ui.text(24, 9, top, 1.5f, Vec4(0.6f, 0.8f, 0.9f, 0.8f));
        const char* rt = s.rayTraced() ? "RENDER: GPU RAY TRACING" : "RENDER: RASTER + SHADOW MAP";
        ui.text(W - 24 - ui.textWidth(rt, 1.5f), 9, rt, 1.5f, Vec4(0.6f, 0.8f, 0.9f, 0.8f));
    }
    if (fade > 0.001f) ui.rect(0, 0, (float)W, (float)H, Vec4(0, 0, 0, fade));
    ui.end();
}

// ---------------------------------------------------------------------------
void App::pressButton(int id, bool global) {
    if (!global) { scenes[current]->onButton(id); return; }
    if (id == GBTN_PREV) goToScene(current - 1);
    if (id == GBTN_NEXT) goToScene(current + 1);
    if (id == GBTN_AUTO) autoPlay = !autoPlay;
    if (id == GBTN_SHADE) renderer.shadingMode = (ShadingMode)((renderer.shadingMode + 1) % 3);
}

std::string App::keyName(int key) const {
    if (key == GLFW_KEY_SPACE) return "SPACE";
    if (key == GLFW_KEY_ENTER || key == GLFW_KEY_KP_ENTER) return "ENTER";
    if (key == GLFW_KEY_UP) return "UP";
    if (key == GLFW_KEY_DOWN) return "DOWN";
    if (key >= GLFW_KEY_A && key <= GLFW_KEY_Z) return std::string(1, (char)('A' + key - GLFW_KEY_A));
    return "";
}

void App::onKey(int key, int action) {
    if (action != GLFW_PRESS) return;
    // 1) does a scene button use this key?
    std::vector<Button> btns;
    scenes[current]->buttons(btns);
    std::string name = keyName(key);
    for (const Button& b : btns)
        if (!name.empty() && b.key == name) { scenes[current]->onButton(b.id); return; }
    // 2) global keys
    switch (key) {
        case GLFW_KEY_ESCAPE: glfwSetWindowShouldClose(window, 1); break;
        case GLFW_KEY_RIGHT: case GLFW_KEY_N: goToScene(current + 1); break;
        case GLFW_KEY_LEFT: case GLFW_KEY_B: goToScene(current - 1); break;
        case GLFW_KEY_P: autoPlay = !autoPlay; break;
        case GLFW_KEY_G: renderer.shadingMode = (ShadingMode)((renderer.shadingMode + 1) % 3); break;
        case GLFW_KEY_H: showHud = !showHud; break;
        case GLFW_KEY_L: shadowsOn = !shadowsOn; break;
        case GLFW_KEY_C: cam.resetUser(); freeCamera = false; break;
        case GLFW_KEY_R: goToScene(current); break;
        case GLFW_KEY_F12: {
            static int n = 0;
            char f[64];
            std::snprintf(f, sizeof(f), "captures/screenshot_%02d.bmp", n++);
            render();
            saveScreenshotBMP(f, W, H);
            std::cout << "saved " << f << "\n";
            break;
        }
        default:
            if (key >= GLFW_KEY_1 && key <= GLFW_KEY_7) goToScene(key - GLFW_KEY_1);
    }
}

void App::onMouseButton(int button, int action) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            for (const ButtonRect& r : buttonRects)
                if (mouseX >= r.x && mouseX <= r.x + r.w && mouseY >= r.y && mouseY <= r.y + r.h) {
                    pressButton(r.id, r.global);
                    return;
                }
            leftDown = true;
            pressX = mouseX;
            pressY = mouseY;
        } else {
            leftDown = false;
        }
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT) rightDown = action == GLFW_PRESS;
}

void App::onCursor(double x, double y) {
    double dx = x - mouseX, dy = y - mouseY;
    mouseX = x;
    mouseY = y;
    if (leftDown) {                         // orbit
        cam.yaw -= (float)dx * 0.005f;
        cam.pitch += (float)dy * 0.004f;
        freeCamera = true;
    }
    if (rightDown) {                        // pan
        float k = 0.02f * std::fmax(cam.zoom, 0.2f) * length(cam.eye - cam.target) * 0.05f;
        cam.pan -= renderer.camRight * (float)dx * k;
        cam.pan += renderer.camUp * (float)dy * k;
        freeCamera = true;
    }
    hoverButton = -1;
    int k = 0, g = 0;
    for (const ButtonRect& r : buttonRects) {
        bool in = x >= r.x && x <= r.x + r.w && y >= r.y && y <= r.y + r.h;
        if (in) hoverButton = r.global ? 100 + (3 - g) : k;
        if (r.global) g++; else k++;
    }
}

void App::onScroll(double dy) {
    cam.zoom = clampf(cam.zoom * (dy > 0 ? 0.9f : 1.1f), 0.15f, 4.0f);
    freeCamera = true;
}

void App::onResize(int w, int h) {
    if (w > 0 && h > 0) { W = w; H = h; }
}

// ---------------------------------------------------------------------------
void App::run() {
    double last = glfwGetTime();
    while (!glfwWindowShouldClose(window)) {
        double now = glfwGetTime();
        float dt = (float)std::fmin(now - last, 0.05);
        last = now;
        fps = fps * 0.95f + (dt > 0 ? 1.0f / dt : 60.0f) * 0.05f;
        glfwPollEvents();
        update(dt);
        render();
        glfwSwapBuffers(window);
    }
}

// Measures how long one frame of each scene really takes on this GPU.
void App::runBenchmark() {
    glfwSwapInterval(0);
    for (int i = 0; i < (int)scenes.size(); i++) {
        current = i;
        scenes[i]->enter();
        fade = 0;
        for (int f = 0; f < 30; f++) { update(1.0f / 60.0f); render(); glfwSwapBuffers(window); }
        glFinish();
        double t0 = glfwGetTime();
        const int N = 120;
        for (int f = 0; f < N; f++) { update(1.0f / 60.0f); render(); glfwSwapBuffers(window); glfwPollEvents(); }
        glFinish();
        double ms = (glfwGetTime() - t0) * 1000.0 / N;
        std::printf("scene %d  %-24s %6.2f ms/frame  (%5.1f fps)\n", i + 1, scenes[i]->title().c_str(), ms, 1000.0 / ms);
    }
}

// Renders exact moments of scenes to .bmp files (used to check the result).
void App::runShots(const std::vector<ShotRequest>& shots, bool hud) {
    showHud = hud;
    for (const ShotRequest& sh : shots) {
        current = sh.scene;
        Scene& s = *scenes[current];
        s.enter();
        cam.resetUser();
        autoPlay = sh.presses.empty();
        fade = 0;
        const float dt = 1.0f / 60.0f;
        size_t nextPress = 0;
        while (s.t < sh.time) {
            if (nextPress < sh.presses.size() && s.t >= sh.presses[nextPress].first) {
                s.onButton(sh.presses[nextPress].second);
                nextPress++;
            }
            if (autoPlay) s.autoPlay();
            s.update(dt);
            globalTime += dt;
        }
        if (sh.free) { cam.yaw = sh.yaw; cam.pitch = sh.pitch; cam.zoom = sh.zoom; freeCamera = true; }
        fps = 60;
        render();
        glFinish();
        saveScreenshotBMP(sh.file, W, H);
        glfwSwapBuffers(window);
        std::cout << "shot: scene " << sh.scene + 1 << " t=" << sh.time << " -> " << sh.file << "\n";
    }
}
