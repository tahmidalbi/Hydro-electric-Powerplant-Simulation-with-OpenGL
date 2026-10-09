#pragma once
#include <string>
#include "../systems/CameraDirector.h"

struct App;

// Command-line options. They let you reproduce any view exactly -- useful for
// report screenshots and for checking the program without touching the keyboard.
//
//   confluence.exe                                   normal interactive run
//   confluence.exe --selftest                        run the maths checks, no window
//   confluence.exe --screenshot out.bmp [options]    render one frame to a BMP and exit
//
// View options (also work in a normal run, they set the starting state):
//   --gallery                       start in the F6 gallery
//   --shading flat|gouraud|phong    --terms all|ambient|diffuse|specular
//   --lights all|sun|fill|lamp      --detail low|medium|high
//   --blinn  --wireframe  --normals  --wrong-order  --wrong-normals
//   --view overview|turbine|dam|solar|boat          --ortho
//   --select turbine1|turbine2|gate|wheel|solar|boat
//   --no-hud  --help-overlay  --paused
// Plant options (applied after the start pose):
//   --wind M/S   --gate PERCENT   --gate-target PERCENT   --auto-gate
//   --tilt DEGREES   --manual-boat THROTTLE(-1..1)
//   --advance SECONDS               run the whole simulation forward before drawing
struct CommandLineOptions {
    bool valid = true;          // false: unknown option, usage was printed
    bool selfTest = false;
    std::string screenshotPath; // empty: interactive
    float advanceSeconds = 0.0f;

    CameraView view = CameraView::Overview;
    bool paused = false;
    float wind = -1.0f;         // < 0: keep the default
    float gate = -1.0f;
    float gateTarget = -1.0f;
    bool autoGate = false;
    float tilt = -1.0f;
    bool manualBoat = false;
    float boatThrottle = 0.0f;
};

namespace CommandLine {
    // Reads argv. View options are written straight into `app.state.view`.
    CommandLineOptions parse(int argc, char** argv, App& app);
    // Plant options, applied after Animation::reset.
    void applyPlantOptions(const CommandLineOptions& options, App& app);
    void printUsage();
}
