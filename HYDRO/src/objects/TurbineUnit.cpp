#include "TurbineUnit.h"
#include "MainInletValve.h"
#include "Materials.h"
#include <algorithm>

using namespace TurbineDims;

static const float THETA_MAX = radians(340.0f);
static const int   NSPIRAL = 90;

// The casing centre line: straight inlet from the valve, then the spiral.
struct CasingPath {
    std::vector<Vec3> pts;
    std::vector<float> radius;     // tube radius at each point
    std::vector<float> theta;      // spiral angle (-1 for the straight inlet)
};

static Vec3 spiralPoint(float th, float R, float y) {
    return Vec3(CENTRE.x - R * std::cos(th), y, CENTRE.z + R * std::sin(th));
}
static float spiralR(float th) { return 4.5f - 1.55f * (th / THETA_MAX); }
static float spiralA(float th) { return 2.0f - 1.5f * std::pow(th / THETA_MAX, 0.85f); }

static CasingPath makeCasingPath() {
    CasingPath c;
    for (float z = ValveDims::Z1; z < CENTRE.z - 0.01f; z += 0.5f) {
        c.pts.push_back(Vec3(ValveDims::X, CENTRE.y, z));
        c.radius.push_back(ValveDims::R);
        c.theta.push_back(-1.0f);
    }
    for (int i = 0; i <= NSPIRAL; i++) {
        float th = THETA_MAX * i / NSPIRAL;
        c.pts.push_back(spiralPoint(th, spiralR(th), CENTRE.y));
        c.radius.push_back(spiralA(th));
        c.theta.push_back(th);
    }
    return c;
}

// Tube of varying radius along frames[i0..i1], angle range a0..a1 measured from 'up'.
static void sweepRange(MeshBuilder& b, const std::vector<PathFrame>& f, const std::vector<float>& rad,
                       int i0, int i1, float scale, float a0, float a1, bool inward, int seg = 36) {
    unsigned int start = (unsigned int)b.verts.size();
    for (int i = i0; i <= i1; i++)
        for (int j = 0; j <= seg; j++) {
            float a = a0 + (a1 - a0) * j / seg;
            Vec3 radial = f[i].up * std::cos(a) + f[i].side * std::sin(a);
            b.addVertex(f[i].pos + radial * rad[i] * scale, inward ? -radial : radial, Vec2(a * rad[i], f[i].s));
        }
    for (int i = 0; i < i1 - i0; i++)
        for (int j = 0; j < seg; j++) {
            unsigned int a = start + i * (seg + 1) + j, c = a + seg + 1;
            b.tri(a, c, a + 1);
            b.tri(a + 1, c, c + 1);
        }
}
// thin strip along the cut edge at angle a (joins outer and inner wall)
static void rimRange(MeshBuilder& b, const std::vector<PathFrame>& f, const std::vector<float>& rad, int i0, int i1, float a, float wall) {
    unsigned int start = (unsigned int)b.verts.size();
    for (int i = i0; i <= i1; i++) {
        Vec3 radial = f[i].up * std::cos(a) + f[i].side * std::sin(a);
        Vec3 n = f[i].up * -std::sin(a) + f[i].side * std::cos(a);
        b.addVertex(f[i].pos + radial * rad[i], n, Vec2(0, f[i].s));
        b.addVertex(f[i].pos + radial * (rad[i] - wall), n, Vec2(wall, f[i].s));
    }
    for (int i = 0; i < i1 - i0; i++) {
        unsigned int k = start + i * 2;
        b.tri(k, k + 1, k + 3);
        b.tri(k, k + 3, k + 2);
    }
}

void TurbineUnit::build() {
    CasingPath cp = makeCasingPath();
    std::vector<PathFrame> f = buildPathFrames(cp.pts);
    casingLength = f.back().s;
    int n = (int)cp.pts.size();
    int firstSpiral = 0;
    while (cp.theta[firstSpiral] < 0) firstSpiral++;

    // which index ranges lie on the near side (x < centre) - hidden in the section view
    int i90 = firstSpiral, i270 = firstSpiral;
    for (int i = firstSpiral; i < n; i++) {
        if (cp.theta[i] <= radians(90.0f)) i90 = i;
        if (cp.theta[i] <= radians(270.0f)) i270 = i;
    }

    // angle (from 'up') that points to the turbine centre: the casing is open there
    Vec3 toCentre = normalize(Vec3(CENTRE.x, CENTRE.y, CENTRE.z) - f[firstSpiral + 5].pos);
    float aIn = std::atan2(dot(toCentre, f[firstSpiral + 5].side), dot(toCentre, f[firstSpiral + 5].up));
    const float cut = radians(60.0f);         // top opening (cut-away)
    float s0, s1;                            // kept wall of the spiral part
    if (aIn > 0) { s0 = aIn + radians(55.0f); s1 = 2 * PI - cut; }
    else         { s0 = cut; s1 = 2 * PI + aIn - radians(55.0f); }
    const float wall = 0.12f;

    MeshBuilder bn, bf, rn, rf;
    // straight inlet (near side): top cut only
    sweepRange(bn, f, cp.radius, 0, firstSpiral, 1.0f, cut, 2 * PI - cut, false);
    sweepRange(bn, f, cp.radius, 0, firstSpiral, 1.0f - wall / 2.0f, cut, 2 * PI - cut, true);
    rimRange(rn, f, cp.radius, 0, firstSpiral, cut, wall);
    rimRange(rn, f, cp.radius, 0, firstSpiral, 2 * PI - cut, wall);
    // spiral: three ranges near / far / near
    struct Range { int a, b; bool near; };
    Range ranges[3] = {{firstSpiral, i90, true}, {i90, i270, false}, {i270, n - 1, true}};
    for (const Range& rg : ranges) {
        MeshBuilder& sb = rg.near ? bn : bf;
        MeshBuilder& rb = rg.near ? rn : rf;
        sweepRange(sb, f, cp.radius, rg.a, rg.b, 1.0f, s0, s1, false);
        sweepRange(sb, f, cp.radius, rg.a, rg.b, 0.94f, s0, s1, true);
        rimRange(rb, f, cp.radius, rg.a, rg.b, s0, wall * 1.5f);
        rimRange(rb, f, cp.radius, rg.a, rg.b, s1, wall * 1.5f);
    }
    shellNear = bn.build();
    shellFar = bf.build();
    rimsNear = rn.build();
    rimsFar = rf.build();

    // ribs around the casing (like the plated casing in the video)
    MeshBuilder b;
    for (int i = firstSpiral + 6; i < n - 4; i += 8) {
        Vec3 radialOut = normalize(Vec3(cp.pts[i].x - CENTRE.x, 0, cp.pts[i].z - CENTRE.z));
        float a = cp.radius[i];
        b.beam(cp.pts[i] + radialOut * (a - 0.05f) - Vec3(0, a * 0.9f, 0), cp.pts[i] + radialOut * (a - 0.05f) + Vec3(0, a * 0.5f, 0), 0.22f, 0.16f);
    }
    flangeRings = b.build();

    // water inside the casing (near + far halves)
    MeshBuilder wn, wf;
    sweepRange(wn, f, cp.radius, 0, i90, 0.84f, 0, 2 * PI, false, 28);
    sweepRange(wn, f, cp.radius, i270, n - 1, 0.84f, 0, 2 * PI, false, 28);
    sweepRange(wf, f, cp.radius, i90, i270, 0.84f, 0, 2 * PI, false, 28);
    waterNear = wn.build();
    waterFar = wf.build();

    // ---- stay ring + 12 fixed stay vanes ------------------------------------
    b.clear();
    for (int s = 0; s < 2; s++) {
        float y = s == 0 ? CENTRE.y - 0.78f : CENTRE.y + 0.72f;
        b.setTransform(translate(Vec3(CENTRE.x, y, CENTRE.z)));
        b.cylinder(2.95f, 2.95f, 0.06f, 64, false, false);
        b.cylinder(2.15f, 2.15f, 0.06f, 64, false, false);
        b.annulus(2.15f, 2.95f, 64);
        b.setTransform(translate(Vec3(CENTRE.x, y + 0.06f, CENTRE.z)));
        b.annulus(2.15f, 2.95f, 64);
    }
    b.resetTransform();
    stayRing = b.build();
    b.clear();
    for (int k = 0; k < 12; k++) {
        float th = k * 2 * PI / 12;
        Vec3 p(CENTRE.x + STAY_R * std::cos(th), CENTRE.y, CENTRE.z + STAY_R * std::sin(th));
        b.setTransform(translate(p) * rotateY(-(th + PI * 0.5f + 0.55f)));
        b.box(Vec3(-0.3f, -0.72f, -0.06f), Vec3(0.3f, 0.72f, 0.06f));
    }
    b.resetTransform();
    stayVanes = b.build();

    // one guide vane (drawn 24 times, each turned about its own pivot)
    b.clear();
    b.box(Vec3(-0.36f, -0.7f, -0.045f), Vec3(0.36f, 0.7f, 0.045f));
    b.box(Vec3(-0.08f, -0.7f, -0.075f), Vec3(0.08f, 0.7f, 0.075f));     // thick nose around the pivot
    b.setTransform(translate(Vec3(0, 0.7f, 0)));
    b.cylinder(0.05f, 0.05f, 1.0f, 8, false, true);                     // pivot stem up to the operating ring
    b.resetTransform();
    guideVane = b.build();
    b.clear();
    b.torus(2.05f, 0.09f, 64, 8);
    for (int k = 0; k < 24; k++) {                                       // levers
        float th = k * 2 * PI / 24;
        b.beam(Vec3(std::cos(th) * 1.98f, 0, std::sin(th) * 1.98f), Vec3(std::cos(th + 0.12f) * 2.05f, 0, std::sin(th + 0.12f) * 2.05f), 0.06f, 0.05f);
    }
    operatingRing = b.build();

    // ---- runner: crown + band + hub + 15 curved blades (local, spins about y) ----
    b.clear();
    b.setTransform(translate(Vec3(0, 0.42f, 0)));
    b.cylinder(1.64f, 0.55f, 0.55f, 64, false, false);                  // crown (cone)
    b.setTransform(translate(Vec3(0, 0.97f, 0)));
    b.disk(0.55f, 32);
    b.setTransform(translate(Vec3(0, -1.2f, 0)));
    b.cylinder(1.3f, 1.42f, 0.7f, 64, false, false);                    // band
    b.cylinder(1.36f, 1.48f, 0.7f, 64, false, false);
    b.setTransform(translate(Vec3(0, -1.4f, 0)));
    b.cylinder(0.5f, 0.5f, 2.4f, 32, false, true);                      // hub
    b.setTransform(translate(Vec3(0, -2.1f, 0)));
    b.cylinder(0.08f, 0.5f, 0.7f, 32, false, false);                    // hub nose
    for (int k = 0; k < 6; k++) {                                        // bolts on the crown
        float a = k * PI / 3;
        b.setTransform(translate(Vec3(std::cos(a) * 0.38f, 0.97f, std::sin(a) * 0.38f)));
        b.cylinder(0.05f, 0.05f, 0.08f, 6, false, true);
    }
    b.resetTransform();
    runner = b.build();

    b.clear();
    const int NU = 10, NV = 6, BLADES = 15;
    for (int k = 0; k < BLADES; k++) {
        float phi = k * 2 * PI / BLADES;
        unsigned int start = (unsigned int)b.verts.size();
        std::vector<Vec3> P((NU + 1) * (NV + 1));
        for (int v = 0; v <= NV; v++)
            for (int u = 0; u <= NU; u++) {
                float uu = (float)u / NU, vv = (float)v / NV;
                float rin = 1.6f, yin = mixf(0.42f, -0.5f, vv);
                float rout = mixf(0.55f, 1.3f, vv), yout = mixf(-0.1f, -1.15f, vv);
                float r = mixf(rin, rout, std::pow(uu, 1.15f));
                float y = mixf(yin, yout, uu * uu * (3 - 2 * uu));
                float a = phi - 1.15f * std::pow(uu, 1.4f);
                P[v * (NU + 1) + u] = Vec3(r * std::cos(a), y, r * std::sin(a));
            }
        for (int v = 0; v <= NV; v++)
            for (int u = 0; u <= NU; u++) {
                Vec3 du = P[v * (NU + 1) + std::min(u + 1, NU)] - P[v * (NU + 1) + std::max(u - 1, 0)];
                Vec3 dv = P[std::min(v + 1, NV) * (NU + 1) + u] - P[std::max(v - 1, 0) * (NU + 1) + u];
                b.addVertex(P[v * (NU + 1) + u], normalize(cross(du, dv)), Vec2((float)u / NU, (float)v / NV));
            }
        for (int v = 0; v < NV; v++)
            for (int u = 0; u < NU; u++) {
                unsigned int a = start + v * (NU + 1) + u, c = a + NU + 1;
                b.tri(a, c, a + 1);
                b.tri(a + 1, c, c + 1);
            }
    }
    runnerBlades = b.build();

    // draft-tube cone under the runner (far half, so we can see in)
    b.clear();
    b.setTransform(translate(Vec3(CENTRE.x, 3.5f, CENTRE.z)));
    b.cylinder(1.75f, 1.42f, CENTRE.y - 1.2f - 3.5f, 48, false, false, -PI * 0.5f, PI * 0.5f);
    b.resetTransform();
    cone = b.build();

    // concrete pedestals under the casing
    b.clear();
    for (int k = 0; k < 6; k++) {
        float th = radians(30.0f) + k * radians(55.0f);
        Vec3 p = spiralPoint(th, spiralR(th), CENTRE.y);
        float a = spiralA(th);
        b.box(Vec3(p.x - 0.6f, 3.5f, p.z - 0.6f), Vec3(p.x + 0.6f, CENTRE.y - a * 0.8f, p.z + 0.6f));
    }
    b.box(Vec3(ValveDims::X - 0.7f, 3.5f, 61.5f), Vec3(ValveDims::X + 0.7f, CENTRE.y - 1.7f, 62.7f));
    pedestals = b.build();

    // ---- streamlines: inlet -> casing -> vanes -> runner -> draft tube -------
    lines.begin();
    sectionLines.begin();
    for (int i = 0; i < 24; i++) {
        std::vector<Vec3> path;
        float rr = 1.25f * std::sqrt((i + 0.5f) / 24.0f), ang = i * 2.39996f;
        float ox = rr * std::cos(ang), oy = rr * std::sin(ang);
        float thExit = radians(12.0f) + (i / 24.0f) * radians(315.0f);
        int last = firstSpiral;
        for (int k = 0; k < n; k++) {
            if (cp.theta[k] > thExit) break;
            float sc = cp.radius[k] / ValveDims::R;
            path.push_back(f[k].pos + f[k].side * ox * sc + f[k].up * oy * sc * 0.6f);
            last = k;
        }
        float th0 = cp.theta[last] < 0 ? 0.0f : cp.theta[last];
        float r0 = spiralR(th0) - spiralA(th0) * 0.5f;
        float y0 = path.back().y;
        for (int k = 1; k <= 12; k++) {                    // through stay + guide vanes
            float u = k / 12.0f;
            float th = th0 + radians(55.0f) * u;
            path.push_back(spiralPoint(th, mixf(r0, RUNNER_R - 0.05f, u), mixf(y0, CENTRE.y + oy * 0.25f, u)));
        }
        float th1 = th0 + radians(55.0f);
        for (int k = 1; k <= 16; k++) {                    // through the runner, turning down
            float u = k / 16.0f;
            float th = th1 + radians(110.0f) * u;
            path.push_back(spiralPoint(th, mixf(RUNNER_R - 0.05f, 0.9f + 0.25f * std::fabs(oy), u),
                                       mixf(CENTRE.y + oy * 0.25f, CENTRE.y - 1.3f, u * u)));
        }
        float th2 = th1 + radians(110.0f);
        for (int k = 1; k <= 12; k++) {                    // down the draft tube cone
            float u = k / 12.0f;
            float th = th2 + radians(50.0f) * u;
            path.push_back(spiralPoint(th, mixf(0.9f + 0.25f * std::fabs(oy), 1.0f, u), mixf(CENTRE.y - 1.3f, 0.5f, u)));
        }
        lines.add(path, 0.035f);

        // section view: the near half (x < centre) is cut away, so keep only the
        // pieces of the line that lie on the far half
        std::vector<Vec3> piece;
        for (const Vec3& p : path) {
            if (p.x >= CENTRE.x) piece.push_back(p);
            else { sectionLines.add(piece, 0.035f); piece.clear(); }
        }
        sectionLines.add(piece, 0.035f);
    }
    lines.finish();
    sectionLines.finish();
}

void TurbineUnit::draw(Renderer& r, float runnerAngle, float vaneOpen01, bool section) {
    Mat4 I;
    Material casing = Mat::paint(Vec3(0.36f, 0.45f, 0.55f), 0.5f);     // blue-grey like the video
    casing.reflect = 0.08f;
    casing.twoSided = true;
    r.drawMesh(shellFar, I, casing);
    r.drawMesh(rimsFar, I, Mat::paint(Vec3(0.5f, 0.58f, 0.66f), 0.5f));
    if (!section) {
        r.drawMesh(shellNear, I, casing);
        r.drawMesh(rimsNear, I, Mat::paint(Vec3(0.5f, 0.58f, 0.66f), 0.5f));
        r.drawMesh(flangeRings, I, Mat::paint(Vec3(0.3f, 0.38f, 0.46f), 0.5f));
    }
    r.drawMesh(pedestals, I, Mat::concrete(1.5f, Vec3(0.45f, 0.45f, 0.44f)));
    Material steel = Mat::steel(Vec3(0.62f, 0.64f, 0.67f));
    r.drawMesh(stayRing, I, steel);
    r.drawMesh(stayVanes, I, steel);

    // TRANSFORMATION: every guide vane turns about its own pivot by the same opening angle
    float beta = radians(8.0f) + vaneOpen01 * radians(38.0f);
    for (int k = 0; k < 24; k++) {
        float th = k * 2 * PI / 24;
        Vec3 p(CENTRE.x + GUIDE_R * std::cos(th), CENTRE.y, CENTRE.z + GUIDE_R * std::sin(th));
        r.drawMesh(guideVane, translate(p) * rotateY(-(th + PI * 0.5f + beta)), Mat::steel(Vec3(0.7f, 0.71f, 0.73f)));
    }
    r.drawMesh(operatingRing, translate(Vec3(CENTRE.x, CENTRE.y + 1.7f, CENTRE.z)) * rotateY(-vaneOpen01 * 0.12f),
               Mat::paint(Vec3(0.75f, 0.6f, 0.1f), 0.5f));

    // the runner spins with the shaft
    Mat4 rm = translate(CENTRE) * rotateY(runnerAngle);
    Material runnerMat = Mat::steel(Vec3(0.78f, 0.79f, 0.8f));
    runnerMat.reflect = 0.45f;
    runnerMat.twoSided = true;
    r.drawMesh(runner, rm, runnerMat);
    r.drawMesh(runnerBlades, rm, runnerMat);
    r.drawMesh(cone, I, casing);
}

void TurbineUnit::drawWater(Renderer& r, float casingFront, bool section, float glow) {
    if (casingFront <= 0.05f) return;
    WaterStyle w;
    w.mode = 2;
    w.deep = Vec3(0.02f, 0.13f, 0.24f);
    w.shallow = Vec3(0.12f, 0.45f, 0.65f);
    w.alpha = 0.8f;
    w.foam = 0.45f;
    w.flowSpeed = 7.0f;
    w.ripple = 0.9f;
    w.front = casingFront;
    w.glow = glow;
    r.drawWater(waterFar, Mat4(), w);
    if (!section) r.drawWater(waterNear, Mat4(), w);
}

void TurbineUnit::drawFlowLines(Renderer& r, float strength, float speed, float front, bool section) {
    if (section)   // only the far half, a little dimmer so the runner stays visible
        sectionLines.draw(r, Mat4(), Vec3(0.35f, 0.75f, 1.6f), strength * 0.7f, speed, 1.6f, front);
    else
        lines.draw(r, Mat4(), Vec3(0.35f, 0.75f, 1.6f), strength, speed, 1.6f, front);
}
