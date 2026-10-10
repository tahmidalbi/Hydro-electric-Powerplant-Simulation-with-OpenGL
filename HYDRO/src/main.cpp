// =============================================================================
//  HYDRO // Dam to Turbine  -  OpenGL computer graphics project
//
//  Recreates the whole "Dam Hydroelectric Power Plant" 3D animation (3:00)
//  as one continuous journey of the water and the energy:
//  preview of the running plant -> reservoir -> trash rack -> intake gate
//  -> penstock -> ray-traced pipe -> main inlet valve -> Francis turbine
//  -> shaft -> generator -> transformer -> switchyard and power line.
//
//  Normal start:        hydro.exe
//  Window size:         hydro.exe --size 1920 1080
//  Save pictures:       hydro.exe --shot <scene 1-11> <seconds> <file.bmp> [--press <sec> <buttonId>] ...
//  Measure speed:       hydro.exe --bench
//  Start options:       --nopost (no bloom), --noshadows, --shading mixed|phong|gouraud
// =============================================================================
#include <iostream>
#include <string>
#include <cstdlib>
#include "App.h"
#include "core/Shader.h"

int main(int argc, char** argv) {
    int w = 1600, h = 900;
    bool hud = true, bench = false, noPost = false, noShadows = false;
    int shading = 0;
    float autoTest = 0;
    std::string autoFile;
    std::vector<ShotRequest> shots;
    for (int i = 1; i < argc; i++) {
        std::string a = argv[i];
        if (a == "--size" && i + 2 < argc) { w = std::atoi(argv[i + 1]); h = std::atoi(argv[i + 2]); i += 2; }
        else if (a == "--nohud") hud = false;
        else if (a == "--bench") bench = true;
        else if (a == "--nopost") noPost = true;
        else if (a == "--noshadows") noShadows = true;
        else if (a == "--shading" && i + 1 < argc) {
            std::string m = argv[++i];
            shading = m == "phong" ? 1 : (m == "gouraud" ? 2 : 0);
        }
        else if (a == "--autotest" && i + 2 < argc) { autoTest = (float)std::atof(argv[i + 1]); autoFile = argv[i + 2]; i += 2; }
        else if (a == "--shot" && i + 3 < argc) {
            ShotRequest s;
            s.scene = std::atoi(argv[i + 1]) - 1;
            s.time = (float)std::atof(argv[i + 2]);
            s.file = argv[i + 3];
            shots.push_back(s);
            i += 3;
        } else if (a == "--press" && i + 2 < argc && !shots.empty()) {
            shots.back().presses.push_back({(float)std::atof(argv[i + 1]), std::atoi(argv[i + 2])});
            i += 2;
        }
    }

    // shaders are looked up next to the executable
    std::string exe = argv[0];
    size_t slash = exe.find_last_of("/\\");
    Shader::shaderFolder = (slash == std::string::npos ? std::string("") : exe.substr(0, slash + 1)) + "shaders/";

    App app;
    if (!app.init(w, h, true)) {
        std::cerr << "Could not create the OpenGL 3.3 window.\n";
        return 1;
    }
    if (noPost) app.setPostFX(false);
    if (noShadows) app.setShadows(false);
    app.setShading(shading);
    if (bench) app.runBenchmark();
    else if (autoTest > 0) app.runAutoTest(autoTest, autoFile);
    else if (!shots.empty()) app.runShots(shots, hud);
    else app.run();
    app.shutdown();
    return 0;
}
