// =============================================================================
//  FlowLines.h  -  a bundle of thin glowing tubes along paths, with bright
//  pulses running along them (PAT_FLOW in materials.glsl).
//
//  Used for: water streamlines through the turbine, magnetic field lines in
//  the generator, electric current in the coils, bus ducts and power lines.
// =============================================================================
#pragma once
#include <vector>
#include "../render/Renderer.h"

class FlowLines {
public:
    float longest = 0;                       // length of the longest line (m)

    void begin() { b.clear(); longest = 0; }
    void add(const std::vector<Vec3>& path, float radius);
    void finish() { mesh = b.build(); }
    // strength 0 = invisible.  speed = metres per second of the pulses,
    // spacing = metres between pulses, front = only the first 'front' metres glow.
    void draw(Renderer& r, const Mat4& model, const Vec3& color, float strength,
              float speed, float spacing, float front = 1e5f) const;

private:
    MeshBuilder b;
    Mesh mesh;
};
