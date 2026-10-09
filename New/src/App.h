// =============================================================================
//  App.h  -  owns the window, the renderer, all scenes, and handles input.
//
//  Frame loop:
//    1. read input (keyboard, mouse buttons, mouse drag, wheel)
//    2. update the current scene (animation / physics)
//    3. shadow pass (sun's view) -> main pass (camera view) -> HUD
//    4. scene transitions with a fade to black
// =============================================================================
#pragma once
#include <memory>
#include <vector>
#include <string>
#include "render/Renderer.h"
#include "ui/Ui.h"
#include "scenes/Scene.h"
#include "objects/DamSite.h"

struct GLFWwindow;

struct ShotRequest {                      // --shot <scene> <seconds> <file.bmp>
    int scene = 0;
    float time = 0;
    std::string file;
    std::vector<std::pair<float, int>> presses;   // (time, button id) pressed during the run
    bool free = false;
    float yaw = 0, pitch = 0, zoom = 1;
};

class App {
public:
    bool init(int width, int height, bool visible);
    void run();
    void runShots(const std::vector<ShotRequest>& shots, bool hud);
    void runBenchmark();          // prints the real frame time of every scene
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
    Ui ui;
    Camera cam;
    DamSite site;
    std::vector<std::unique_ptr<Scene>> scenes;
    int current = 0;
    int pending = -1;             // scene we are fading to
    float fade = 1.0f;            // 1 = black, 0 = fully visible
    bool fadingOut = false;
    bool autoPlay = false;
    bool showHud = true;
    bool freeCamera = false;      // the user has taken over the camera
    bool shadowsOn = true;        // L key
    float fps = 60, globalTime = 0;
    bool hudForShots = true;

    // mouse state
    double mouseX = 0, mouseY = 0;
    bool leftDown = false, rightDown = false, dragged = false;
    double pressX = 0, pressY = 0;

    struct ButtonRect { float x, y, w, h; int id; bool global; };
    std::vector<ButtonRect> buttonRects;
    int hoverButton = -1;

    void goToScene(int index);
    void update(float dt);
    void render();
    void drawHud();
    void pressButton(int id, bool global);
    std::string keyName(int key) const;
};
