// =============================================================================
//  Renderer.h  -  everything needed to draw one frame.
//
//   * Lighting   : sun (directional light), sky/ground ambient, fog, up to 4
//                  point lights, underwater caustics, exposure.
//   * Material   : colour + shininess + a procedural "pattern" (concrete,
//                  steel, bark, tiles ...) computed in the fragment shader.
//   * WaterStyle : parameters of the animated water shader.
//   * Shadow map : the sun renders the scene into a depth texture first, then
//                  every surface checks it -> real cast shadows.
//   * Phong vs Gouraud : lit.frag lights every PIXEL (Phong shading);
//                  gouraud.vert lights every VERTEX and interpolates (Gouraud).
// =============================================================================
#pragma once
#include "../core/Shader.h"
#include "../core/Mesh.h"
#include "../core/Camera.h"

enum Pattern {
    PAT_NONE = 0, PAT_CONCRETE = 1, PAT_STEEL = 2, PAT_PAINT = 3, PAT_BARK = 4,
    PAT_TERRAIN = 5, PAT_SOIL = 6, PAT_TILES = 7, PAT_GRATING = 8, PAT_COPPER = 9,
    PAT_HOLO = 10, PAT_LEAF = 11, PAT_BEAM = 12, PAT_ROCK = 13, PAT_PLASTIC = 14,
    PAT_FLOW = 15     // glowing flow line: bright pulses travel along uv.y
};

struct Material {
    Vec3  color{0.6f, 0.6f, 0.6f};
    float specular = 0.15f;     // strength of the highlight
    float shininess = 24.0f;    // size of the highlight (bigger = sharper)
    float reflect = 0.0f;       // mirror-like reflection of the sky (metals)
    int   pattern = PAT_NONE;
    float patternScale = 1.0f;
    Vec3  emissive{0, 0, 0};    // glow (lamps, LEDs, light shafts)
    float alpha = 1.0f;
    bool  gouraud = false;      // light this object per-vertex (Gouraud) in "mixed" mode
    bool  twoSided = false;     // thin sheets (leaves, plastic, plates)
    bool  additive = false;     // light shafts / glows
    bool  castShadow = true;
    float flowSpeed = 0.0f;     // PAT_FLOW: how fast the pulses travel (m/s)
    float flowFront = 1e5f;     // PAT_FLOW: only uv.y below this glows (a front that grows)
};

struct WaterStyle {
    int   mode = 0;                         // 0 lake/river, 1 flowing sheet, 2 water tube in a pipe, 3 cut face of a water volume
    Vec3  deep{0.01f, 0.06f, 0.09f};
    Vec3  shallow{0.04f, 0.25f, 0.32f};
    float alpha = 0.9f;
    float foam = 0.0f;                      // amount of white streaks
    float flowSpeed = 0.0f;                 // metres per second along uv.y
    float ripple = 0.5f;                    // ripple frequency
    float rippleStrength = 1.0f;
    float waveAmp = 0.0f, waveFreq = 0.4f;  // vertex waves for vertices at height surfaceY
    float surfaceY = -1e9f;
    float front = 1e5f, tail = -1e3f;       // only uv.y in [tail, front] holds water
    float noseLen = 0.0f, tubeRadius = 0.0f;// rounded front of water inside a pipe
    float sheetWidth = 0.0f;                // foam along both banks of a chute
    float glow = 0.0f;                      // sci-fi glowing streaks
    Vec4  foamSpot{0, 0, 0, 0};             // x, z, radius, strength (foam where a jet lands)
    Vec4  foamSpot2{0, 0, 0, 0};            // a second foam patch
};

struct Lighting {
    Vec3  sunDir = normalize(Vec3(-0.4f, 0.75f, 0.45f));   // direction TOWARDS the sun
    Vec3  sunColor{3.0f, 2.85f, 2.6f};
    Vec3  skyTop{0.22f, 0.4f, 0.7f};
    Vec3  skyHorizon{0.65f, 0.72f, 0.8f};
    Vec3  ground{0.22f, 0.21f, 0.2f};
    float ambient = 0.55f;
    Vec3  fogColor{0.62f, 0.7f, 0.78f};
    float fogDensity = 0.0015f;
    int   skyMode = 0;           // 0 outdoor sky, 1 studio backdrop, 2 underwater
    bool  underwater = false;
    float waterLevel = -1e9f;    // caustics appear below this height
    float caustics = 0.0f;
    Vec4  causticRect{-1e9f, -1e9f, 1e9f, 1e9f};   // xmin, zmin, xmax, zmax
    float exposure = 1.0f;
    bool  shadows = true;
    int   numPoint = 0;
    Vec3  pointPos[4];
    Vec3  pointColor[4];
};

enum ShadingMode { SHADE_MIXED = 0, SHADE_ALL_PHONG = 1, SHADE_ALL_GOURAUD = 2 };

class Renderer {
public:
    Shader lit, gouraud, water, depth, sky, particle;
    int width = 1280, height = 720;
    ShadingMode shadingMode = SHADE_MIXED;
    Lighting L;
    Camera cam;
    Mat4 viewM, projM;
    float time = 0;

    bool init();
    void beginFrame(const Camera& c, const Lighting& lighting, float t, int w, int h);

    // ---- shadow pass (call scene drawing between these) ----
    void beginShadowPass(const Vec3& centre, float radius);
    void endShadowPass();
    void prepareMainPass();       // always call before drawing the visible frame
    void applyCommon(const Shader& s);
    bool inShadowPass() const { return shadowPass; }

    void drawSky();
    void drawMesh(const Mesh& m, const Mat4& model, const Material& mat);
    void drawWater(const Mesh& m, const Mat4& model, const WaterStyle& w);
    void drawFullscreen();        // one big triangle (used by sky and the ray tracer)

    Vec3 camRight, camUp, camForward;

private:
    unsigned int shadowFbo = 0, shadowTex = 0, emptyVao = 0;
    int shadowSize = 2048;
    bool shadowPass = false;
    Mat4 lightSpace;
    float shadowBias = 0.001f, shadowNormalOffset = 0.1f;
};
