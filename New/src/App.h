// =============================================================================
//  App.h  -  owns the window, the renderer, the world, all scenes, and
//  handles input.
//
//  Frame loop:
//    1. read input (keyboard, mouse buttons, mouse drag, wheel)
//    2. update the current scene (animation / physics), scaled by the
//       global SPEED control (0.25x .. 2x)
//    3. shadow pass (sun's view) -> main pass (camera view) -> HUD
//    4. scene changes: following the story the camera simply flies on
//       (continuous); a cross-dissolve is used where the picture style
//       changes; jumping with number keys fades through black.
// =============================================================================
#pragma once
#include <memory>
#include <vector>
#include <string>
#include "render/Renderer.h"
#include "render/FrameCapture.h"
#include "render/PostFX.h"
#include "ui/Ui.h"
#include "scenes/Scene.h"
#include "objects/World.h"

struct GLFWwindow;

// --shot <scene> <seconds> <file.bmp> : start at that scene and let the story run
struct ShotRequest {
    int scene = 0;
    float time = 0;
    std::string file;
    std::vector<std::pair<float, int>> presses;   // (time, button id) pressed during the run
};

class App {
public:
    bool init(int width, int height, bool visible);
    void run();
    void runShots(const std::vector<ShotRequest>& shots, bool hud);
    void runBenchmark();          // prints the real frame time of every scene
    void runAutoTest(float seconds, const std::string& file);   // real frame loop in AUTO mode, then a picture
    void setPostFX(bool on) { postfx.enabled = on; }
    void shutdown();

    // input callbacks (called from GLFW)
    void onKey(int key, int action);
    void onMouseButton(int button, int action);
    void onCursor(double x, double y);
    void onScroll(double dy);
    void onResize(int w, int h);

private:
    GLFWwindow* window = nullptr;
    int W = 1600, H = 900;
    Renderer renderer;
    FrameCapture capture;
    PostFX postfx;
    Ui ui;
    Camera cam;
    World world;
    std::vector<std::unique_ptr<Scene>> scenes;
    int current = 0;
    int pending = -1;             // scene we are fading to (number-key jumps)
    float fade = 1.0f;            // 1 = black, 0 = fully visible
    bool fadingOut = false;
    float dissolve = 0.0f;        // 1 = old picture fully on top, fades to 0
    bool autoPlay = false;
    bool showHud = true;
    bool freeCamera = false;      // the user has taken over the camera
    bool shadowsOn = true;        // L key
    bool shotMode = false;
    int speedIndex = 2;           // index into SPEEDS (1x)
    float fps = 60, globalTime = 0;

    // mouse state
    double mouseX = 0, mouseY = 0;
    bool leftDown = false, rightDown = false;

    struct ButtonRect { float x, y, w, h; int id; bool global; bool enabled; };
    std::vector<ButtonRect> buttonRects;
    int hoverIndex = -1;

    float timeScale() const;
    void jumpToScene(int index);          // fade through black
    void continueTo(int index);           // the story moves on: camera flies on
    void update(float realDt);
    void render();
    void drawHud();
    void pressButton(int id, bool global);
    void pressSceneButton(int id);
    std::string keyName(int key) const;
};
