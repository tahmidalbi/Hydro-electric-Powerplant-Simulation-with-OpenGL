// =============================================================================
//  HYDRO // Dam to Turbine  -  OpenGL computer graphics project
//
//  Recreates the first 70 seconds of the "Dam Hydroelectric Power Plant"
//  3D animation:  spillway -> dam survey -> under water to the trash rack ->
//  trash filtering -> intake gate -> penstock -> ray-traced high-speed flow.
//
//  Normal start:        hydro.exe
//  Window size:         hydro.exe --size 1920 1080
//  Save pictures:       hydro.exe --shot <scene 1-7> <seconds> <file.bmp> [--press <sec> <buttonId>] ...
// =============================================================================
#include <iostream>
#include <string>
#include <cstdlib>
#include "App.h"
#include "core/Shader.h"

int main(int argc, char** argv) {
    int w = 1600, h = 900;
    bool hud = true, bench = false;
    std::vector<ShotRequest> shots;
    for (int i = 1; i < argc; i++) {
        std::string a = argv[i];
        if (a == "--size" && i + 2 < argc) { w = std::atoi(argv[i + 1]); h = std::atoi(argv[i + 2]); i += 2; }
        else if (a == "--nohud") hud = false;
        else if (a == "--bench") bench = true;
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
        } else if (a == "--orbit" && i + 3 < argc && !shots.empty()) {
            shots.back().free = true;
            shots.back().yaw = (float)std::atof(argv[i + 1]);
            shots.back().pitch = (float)std::atof(argv[i + 2]);
            shots.back().zoom = (float)std::atof(argv[i + 3]);
            i += 3;
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
    if (bench) app.runBenchmark();
    else if (!shots.empty()) app.runShots(shots, hud);
    else app.run();
    app.shutdown();
    return 0;
}
