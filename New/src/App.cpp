#include "App.h"
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <cstdio>
#include <iostream>
#include "core/Screenshot.h"
#include "scenes/Scene1_Preview.h"
#include "scenes/Scene2_ReservoirToRack.h"
#include "scenes/Scene3_IntakeGate.h"
#include "scenes/Scene4_Penstock.h"
#include "scenes/Scene5_RayTracedPipe.h"
#include "scenes/Scene6_InletValve.h"
#include "scenes/Scene7_Turbine.h"
#include "scenes/Scene8_Shaft.h"
#include "scenes/Scene9_Generator.h"
#include "scenes/Scene10_Transformer.h"
#include "scenes/Scene11_Grid.h"

// global buttons
enum { GBTN_PREV = 1001, GBTN_NEXT = 1002, GBTN_AUTO = 1003, GBTN_SHADE = 1004,
       GBTN_SLOWER = 1005, GBTN_FASTER = 1006 };

static const float SPEEDS[] = {0.25f, 0.5f, 1.0f, 2.0f};
static const char* SPEED_NAMES[] = {"0.25X", "0.5X", "1X", "2X"};
static const int NUM_SPEEDS = 4;

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
    if (!capture.init(W, H)) std::cerr << "blit shader problem\n";
    if (!postfx.init(W, H)) std::cerr << "post-processing shader problem\n";

    std::cout << "Building the world...\n";
    world.build();
    scenes.emplace_back(new Scene1_Preview(world));
    scenes.emplace_back(new Scene2_ReservoirToRack(world));
    scenes.emplace_back(new Scene3_IntakeGate(world));
    scenes.emplace_back(new Scene4_Penstock(world));
    scenes.emplace_back(new Scene5_RayTracedPipe());
    scenes.emplace_back(new Scene6_InletValve(world));
    scenes.emplace_back(new Scene7_Turbine(world));
    scenes.emplace_back(new Scene8_Shaft(world));
    scenes.emplace_back(new Scene9_Generator(world));
    scenes.emplace_back(new Scene10_Transformer(world));
    scenes.emplace_back(new Scene11_Grid(world));
    for (auto& s : scenes) s->build();
    scenes[0]->enter(false);
    fade = 1.0f;
    return true;
}

void App::shutdown() {
    glfwDestroyWindow(window);
    glfwTerminate();
}

float App::timeScale() const { return SPEEDS[speedIndex]; }

// ---------------------------------------------------------------------------
//  Scene changes
// ---------------------------------------------------------------------------
void App::jumpToScene(int index) {
    if (index < 0 || index >= (int)scenes.size()) return;
    pending = index;
    fadingOut = true;
}

void App::continueTo(int index) {
    Scene& next = *scenes[index];
    // the next scene's camera starts exactly where we are now -> one continuous shot
    next.fromEye = cam.finalEye();
    next.fromTarget = cam.target + cam.pan;
    cam.resetUser();
    freeCamera = false;
    current = index;
    next.enter(true);
    if (next.dissolveOnEnter() && !shotMode) {
        capture.freeze();
        dissolve = 1.0f;
    }
}

void App::update(float realDt) {
    float dt = realDt * timeScale();
    globalTime += dt;
    Scene& s = *scenes[current];
    if (autoPlay) s.autoPlay();
    s.update(dt);

    // director's camera for this moment (the user's orbit is added on top)
    cam.nearZ = 0.2f;
    cam.farZ = 3000.0f;
    cam.fovDeg = 50.0f;
    s.camera(cam);
    Lighting probe;
    s.lighting(probe, cam.finalEye());    // lets the world know if the camera is under water

    if (s.dissolveRequest) {
        s.dissolveRequest = false;
        if (!shotMode) { capture.freeze(); dissolve = 1.0f; }
    }
    if (dissolve > 0.0f) dissolve = std::fmax(0.0f, dissolve - realDt / 1.1f);

    // number-key jumps: fade out -> switch scene -> fade in
    if (fadingOut) {
        fade += realDt / 0.45f;
        if (fade >= 1.0f) {
            fade = 1.0f;
            fadingOut = false;
            current = pending;
            pending = -1;
            cam.resetUser();
            freeCamera = false;
            scenes[current]->enter(false);
        }
    } else if (fade > 0.0f) {
        fade = std::fmax(0.0f, fade - realDt / 0.6f);
    }
    // the story moves on when a scene says so
    if (!fadingOut && scenes[current]->finished()) {
        if (current + 1 < (int)scenes.size()) continueTo(current + 1);
        else jumpToScene(0);              // replay the journey from the preview
    }

    // free camera panning with W A S D Q E
    if (freeCamera && window) {
        Vec3 f = normalize(Vec3(renderer.camForward.x, 0, renderer.camForward.z));
        Vec3 rgt = normalize(cross(f, Vec3(0, 1, 0)));
        float sp = 12.0f * realDt * std::fmax(cam.zoom, 0.3f);
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
    Lighting L;
    s.lighting(L, cam.finalEye());
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
    if (dissolve > 0.0f) capture.drawFrozen(dissolve * dissolve * (3.0f - 2.0f * dissolve));
    capture.captureLive();                        // copy of the picture: dissolves + post-processing
    if (postfx.enabled) postfx.apply(capture.liveTexture(), globalTime);   // bloom, vignette, grain
    drawHud();
}

// ---------------------------------------------------------------------------
//  HUD: title, caption, telemetry, speed, buttons, help text, fade
// ---------------------------------------------------------------------------
void App::drawHud() {
    ui.begin(W, H);
    Scene& s = *scenes[current];
    const Vec4 cyan(0.45f, 0.88f, 1.0f, 1.0f), white(0.95f, 0.97f, 1.0f, 1.0f), panel(0.02f, 0.05f, 0.08f, 0.55f);
    const Vec4 dimCyan(0.45f, 0.88f, 1.0f, 0.35f), grey(0.6f, 0.62f, 0.65f, 0.7f);

    // cinematic letterbox bars
    ui.rect(0, 0, (float)W, 30, Vec4(0, 0, 0, 0.85f));
    ui.rect(0, H - 30.0f, (float)W, 30, Vec4(0, 0, 0, 0.85f));
    buttonRects.clear();

    // one chip-style button (used for the small global buttons)
    auto chip = [&](float x, float y, const char* label, int id) {
        float w = ui.textWidth(label, 1.5f) + 24, h = 30;
        bool hover = hoverIndex == (int)buttonRects.size();
        ui.rect(x, y, w, h, Vec4(0.03f, 0.08f, 0.12f, hover ? 0.95f : 0.7f));
        ui.outline(x, y, w, h, 1, dimCyan);
        ui.text(x + 12, y + 9, label, 1.5f, white);
        buttonRects.push_back({x, y, w, h, id, true, true});
        return w;
    };

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
        float pw = 330, px = W - pw - 24, py = 44;
        float ph = tel.empty() ? 0.0f : 34 + tel.size() * 24.0f;
        if (!tel.empty()) {
            ui.rect(px, py, pw, ph, panel);
            ui.corners(px, py, pw, ph, 14, 2, cyan);
            ui.text(px + 14, py + 10, "TELEMETRY", 1.5f, cyan);
            for (size_t i = 0; i < tel.size(); i++) {
                float y = py + 32 + i * 24.0f;
                ui.text(px + 14, y, tel[i].label, 1.5f, Vec4(0.7f, 0.82f, 0.9f, 1));
                ui.text(px + pw - 14 - ui.textWidth(tel[i].value, 2), y - 2, tel[i].value, 2, white);
            }
        }

        // global SPEED control (under the telemetry panel)
        {
            float sy = py + ph + 10;
            char sp[32];
            std::snprintf(sp, sizeof(sp), "SPEED %s", SPEED_NAMES[speedIndex]);
            float wFaster = ui.textWidth("FASTER [ ]", 1.5f) + 24;
            float wSpeed = ui.textWidth(sp, 1.5f) + 24;
            float wSlower = ui.textWidth("[ [ ] SLOWER", 1.5f) + 24;
            float x = W - 24 - wFaster - 6 - wSpeed - 6 - wSlower;
            x += chip(x, sy, "[ [ ] SLOWER", GBTN_SLOWER) + 6;
            ui.rect(x, sy, wSpeed, 30, Vec4(0.05f, 0.3f, 0.45f, 0.8f));
            ui.outline(x, sy, wSpeed, 30, 1, cyan);
            ui.text(x + 12, sy + 9, sp, 1.5f, white);
            x += wSpeed + 6;
            chip(x, sy, "FASTER [ ]", GBTN_FASTER);
        }

        // help (bottom left)
        // (kept short so it never runs under the navigation chips on the right)
        const char* help[] = {"DRAG: ORBIT  RIGHT DRAG/WASDQE: MOVE  WHEEL: ZOOM  [ ]: SPEED  C: DIRECTOR CAMERA",
                              "G: PHONG/GOURAUD  L: SHADOWS  V: BLOOM  H: HUD  1-9,0: SCENE  N/B: NEXT/BACK  P: AUTO"};
        ui.text(24, H - 72.0f, help[0], 1.5f, Vec4(0.75f, 0.85f, 0.95f, 0.75f));
        ui.text(24, H - 54.0f, help[1], 1.5f, Vec4(0.75f, 0.85f, 0.95f, 0.75f));

        // scene buttons (bottom centre)
        std::vector<Button> btns;
        s.buttons(btns);
        float total = 0;
        for (const Button& b : btns) total += ui.textWidth(b.label, 2) + 60 + 16;
        float x = (W - total) * 0.5f + 8, y = H - 128.0f;
        for (const Button& b : btns) {
            float w = ui.textWidth(b.label, 2) + 60, h = 44;
            bool hover = b.enabled && hoverIndex == (int)buttonRects.size();
            Vec4 fill = !b.enabled ? Vec4(0.08f, 0.09f, 0.1f, 0.6f)
                      : b.primary ? Vec4(0.05f, 0.35f, 0.5f, hover ? 0.95f : 0.8f)
                                  : Vec4(0.03f, 0.08f, 0.12f, hover ? 0.95f : 0.75f);
            ui.rect(x, y, w, h, fill);
            ui.outline(x, y, w, h, 1, !b.enabled ? grey : (b.primary ? cyan : dimCyan));
            if (b.enabled) ui.corners(x - 3, y - 3, w + 6, h + 6, 8, 2, cyan);
            ui.text(x + 30, y + 8, b.label, 2, b.enabled ? white : grey);
            std::string key = b.enabled ? "[" + b.key + "]" : (b.hint.empty() ? "(NOT READY YET)" : b.hint);
            ui.text(x + (w - ui.textWidth(key, 1)) * 0.5f, y + 31, key, 1, b.enabled ? Vec4(0.6f, 0.85f, 1.0f, 0.9f) : grey);
            buttonRects.push_back({x, y, w, h, b.id, false, b.enabled});
            x += w + 16;
        }

        // navigation chips (bottom right)
        const char* glabels[] = {"<", ">", autoPlay ? "AUTO: ON" : "AUTO: OFF",
                                 renderer.shadingMode == SHADE_MIXED ? "SHADING: MIXED" :
                                 renderer.shadingMode == SHADE_ALL_PHONG ? "SHADING: PHONG" : "SHADING: GOURAUD"};
        int gids[] = {GBTN_PREV, GBTN_NEXT, GBTN_AUTO, GBTN_SHADE};
        float total2 = 0;
        for (int i = 0; i < 4; i++) total2 += ui.textWidth(glabels[i], 1.5f) + 24 + 8;
        float gx = W - 24.0f - total2 + 8;
        for (int i = 0; i < 4; i++) gx += chip(gx, H - 72.0f, glabels[i], gids[i]) + 8;

        // frame rate + mode (top bar)
        char top[128];
        std::snprintf(top, sizeof(top), "HYDRO//SIM   %3.0f FPS   %s   SPEED %s", fps,
                      freeCamera ? "FREE CAMERA" : "DIRECTOR CAMERA", SPEED_NAMES[speedIndex]);
        ui.text(24, 9, top, 1.5f, Vec4(0.6f, 0.8f, 0.9f, 0.8f));
        const char* rt = s.rayTraced() ? "RENDER: GPU RAY TRACING" : "RENDER: RASTER + SHADOW MAP";
        ui.text(W - 24 - ui.textWidth(rt, 1.5f), 9, rt, 1.5f, Vec4(0.6f, 0.8f, 0.9f, 0.8f));
    }
    if (fade > 0.001f) ui.rect(0, 0, (float)W, (float)H, Vec4(0, 0, 0, fade));
    ui.end();
}

// ---------------------------------------------------------------------------
//  Input
// ---------------------------------------------------------------------------
void App::pressSceneButton(int id) {
    std::vector<Button> btns;
    scenes[current]->buttons(btns);
    for (const Button& b : btns)
        if (b.id == id && b.enabled) { scenes[current]->onButton(id); return; }
}

void App::pressButton(int id, bool global) {
    if (!global) { pressSceneButton(id); return; }
    if (id == GBTN_PREV) jumpToScene(current - 1);
    if (id == GBTN_NEXT) jumpToScene(current + 1);
    if (id == GBTN_AUTO) autoPlay = !autoPlay;
    if (id == GBTN_SHADE) renderer.shadingMode = (ShadingMode)((renderer.shadingMode + 1) % 3);
    if (id == GBTN_SLOWER && speedIndex > 0) speedIndex--;
    if (id == GBTN_FASTER && speedIndex < NUM_SPEEDS - 1) speedIndex++;
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
        if (!name.empty() && b.key == name) { pressSceneButton(b.id); return; }
    // 2) global keys
    switch (key) {
        case GLFW_KEY_ESCAPE: glfwSetWindowShouldClose(window, 1); break;
        case GLFW_KEY_RIGHT: case GLFW_KEY_N: jumpToScene(current + 1); break;
        case GLFW_KEY_LEFT: case GLFW_KEY_B: jumpToScene(current - 1); break;
        case GLFW_KEY_P: autoPlay = !autoPlay; break;
        case GLFW_KEY_G: renderer.shadingMode = (ShadingMode)((renderer.shadingMode + 1) % 3); break;
        case GLFW_KEY_H: showHud = !showHud; break;
        case GLFW_KEY_L: shadowsOn = !shadowsOn; break;
        case GLFW_KEY_V: postfx.enabled = !postfx.enabled; break;
        case GLFW_KEY_0: jumpToScene(9); break;
        case GLFW_KEY_C: cam.resetUser(); freeCamera = false; break;
        case GLFW_KEY_R: jumpToScene(current); break;
        case GLFW_KEY_LEFT_BRACKET: case GLFW_KEY_MINUS: case GLFW_KEY_KP_SUBTRACT: pressButton(GBTN_SLOWER, true); break;
        case GLFW_KEY_RIGHT_BRACKET: case GLFW_KEY_EQUAL: case GLFW_KEY_KP_ADD: pressButton(GBTN_FASTER, true); break;
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
            if (key >= GLFW_KEY_1 && key <= GLFW_KEY_9) jumpToScene(key - GLFW_KEY_1);
    }
}

void App::onMouseButton(int button, int action) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            for (const ButtonRect& r : buttonRects)
                if (mouseX >= r.x && mouseX <= r.x + r.w && mouseY >= r.y && mouseY <= r.y + r.h) {
                    if (r.enabled) pressButton(r.id, r.global);
                    return;
                }
            leftDown = true;
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
    hoverIndex = -1;
    for (size_t i = 0; i < buttonRects.size(); i++) {
        const ButtonRect& r = buttonRects[i];
        if (x >= r.x && x <= r.x + r.w && y >= r.y && y <= r.y + r.h) hoverIndex = (int)i;
    }
}

void App::onScroll(double dy) {
    cam.zoom = clampf(cam.zoom * (dy > 0 ? 0.9f : 1.1f), 0.15f, 4.0f);
    freeCamera = true;
}

void App::onResize(int w, int h) {
    if (w > 0 && h > 0) { W = w; H = h; capture.resize(w, h); postfx.resize(w, h); }
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

// Runs the real frame loop (every frame rendered, dissolves included) in AUTO
// mode with a fixed time step, saves a picture after 'seconds' and stops.
void App::runAutoTest(float seconds, const std::string& file) {
    autoPlay = true;
    fade = 0;
    for (float T = 0; T < seconds; T += 1.0f / 60.0f) {
        glfwPollEvents();
        update(1.0f / 60.0f);
        render();
        if (T + 1.0f / 60.0f >= seconds) { glFinish(); saveScreenshotBMP(file, W, H); }
        glfwSwapBuffers(window);
    }
    std::cout << "autotest " << seconds << "s (scene " << current + 1 << ") -> " << file << "\n";
}

// Measures how long one frame of each scene really takes on this GPU.
void App::runBenchmark() {
    glfwSwapInterval(0);
    for (int i = 0; i < (int)scenes.size(); i++) {
        current = i;
        scenes[i]->enter(false);
        fade = 0;
        for (int f = 0; f < 240; f++) update(1.0f / 60.0f);      // let the scene get going
        for (int f = 0; f < 30; f++) { update(1.0f / 60.0f); render(); glfwSwapBuffers(window); }
        current = i;
        glFinish();
        double t0 = glfwGetTime();
        const int N = 120;
        for (int f = 0; f < N; f++) { update(1.0f / 60.0f); current = i; render(); glfwSwapBuffers(window); glfwPollEvents(); }
        glFinish();
        double ms = (glfwGetTime() - t0) * 1000.0 / N;
        std::printf("scene %d  %-30s %6.2f ms/frame  (%5.1f fps)\n", i + 1, scenes[i]->title().c_str(), ms, 1000.0 / ms);
    }
}

// Starts at a scene and lets the story run (buttons pressed automatically, or
// at the given times), then saves the picture.  Scene changes happen exactly
// like in the real program, so this checks the continuity too.
void App::runShots(const std::vector<ShotRequest>& shots, bool hud) {
    showHud = hud;
    shotMode = true;
    for (const ShotRequest& sh : shots) {
        current = sh.scene;
        cam.resetUser();
        freeCamera = false;
        fade = 0;
        fadingOut = false;
        dissolve = 0;
        scenes[current]->enter(false);
        autoPlay = sh.presses.empty();
        const float dt = 1.0f / 60.0f;
        size_t nextPress = 0;
        for (float T = 0; T < sh.time; T += dt) {
            if (nextPress < sh.presses.size() && T >= sh.presses[nextPress].first) {
                pressSceneButton(sh.presses[nextPress].second);
                nextPress++;
            }
            update(dt);
        }
        fps = 60;
        render();
        glFinish();
        saveScreenshotBMP(sh.file, W, H);
        glfwSwapBuffers(window);
        std::cout << "shot: start scene " << sh.scene + 1 << " +" << sh.time << "s (now scene " << current + 1 << ") -> " << sh.file << "\n";
    }
}
