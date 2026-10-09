// =============================================================================
//  CutawayPipe.h  -  a steel penstock pipe that follows a path, with one side
//  cut open so we can watch the water inside (like the video).
//
//  Parts:
//    * shell  : outer + inner wall of the pipe, only the kept angle range
//    * rims   : the thin cut edges
//    * rings  : stiffener / flange rings around the full pipe, with bolts
//    * holo   : the removed part, drawn as a glowing "x-ray" grid when wanted
//    * water  : a tube of water inside the pipe, with a moving rounded front
// =============================================================================
#pragma once
#include <vector>
#include "../render/Renderer.h"

class CutawayPipe {
public:
    float radius = 2.0f;
    float wall = 0.2f;
    float length = 0.0f;
    std::vector<PathFrame> frames;

    // corners  : the pipe centre line (corners get rounded with bendRadius)
    // cutDir   : world direction of the side that is cut away (towards the camera)
    // cutHalfDeg : half of the removed angle (90 = half pipe)
    void build(const std::vector<Vec3>& corners, float radius, float bendRadius,
               const Vec3& cutDir, float cutHalfDeg, float ringSpacing);

    void drawShell(Renderer& r, const Material& steel, float holoAmount = 0.0f);
    void drawHolo(Renderer& r, float amount);
    void drawWater(Renderer& r, WaterStyle w);

    Vec3 pointAt(float s) const;
    Vec3 tangentAt(float s) const;

private:
    Mesh shell, rims, rings, bolts, holo, water;
};
