#include "Generator.h"
#include "Materials.h"

using namespace GenDims;

// kept angle range of every stator part (everything except the window at -x)
static const float A0 = -PI + WINDOW, A1 = PI - WINDOW;

// flat ring piece (annulus) between angles a0..a1 at height y
static void ringPiece(MeshBuilder& b, float rIn, float rOut, float y, float a0, float a1, bool up) {
    unsigned int start = (unsigned int)b.verts.size();
    const int seg = 64;
    Vec3 n(0, up ? 1.0f : -1.0f, 0);
    for (int i = 0; i <= seg; i++) {
        float a = a0 + (a1 - a0) * i / seg, c = std::cos(a), s = std::sin(a);
        b.addVertex(Vec3(c * rIn, y, s * rIn), n, Vec2(c * rIn, s * rIn));
        b.addVertex(Vec3(c * rOut, y, s * rOut), n, Vec2(c * rOut, s * rOut));
    }
    for (int i = 0; i < seg; i++) {
        unsigned int k = start + i * 2;
        b.tri(k, k + 1, k + 3);
        b.tri(k, k + 3, k + 2);
    }
}
// the flat cut face where the window slices a ring part
static void cutFace(MeshBuilder& b, float a, float rIn, float rOut, float y0, float y1) {
    Vec3 dir(std::cos(a), 0, std::sin(a));
    Vec3 n = normalize(cross(Vec3(0, 1, 0), dir));
    if (a < 0) n = -n;
    b.quad(dir * rIn + Vec3(0, y0, 0), dir * rOut + Vec3(0, y0, 0), dir * rOut + Vec3(0, y1, 0), dir * rIn + Vec3(0, y1, 0));
    (void)n;
}
// a thick ring segment: outer + inner wall + top + bottom + cut faces
static void ringSolid(MeshBuilder& b, float rIn, float rOut, float y0, float y1) {
    Mat4 saved = b.xf;
    b.setTransform(saved * translate(Vec3(0, y0, 0)));
    b.cylinder(rOut, rOut, y1 - y0, 72, false, false, A0, A1);
    b.cylinder(rIn, rIn, y1 - y0, 72, false, false, A0, A1);
    b.setTransform(saved);
    ringPiece(b, rIn, rOut, y1, A0, A1, true);
    ringPiece(b, rIn, rOut, y0, A0, A1, false);
    cutFace(b, A0, rIn, rOut, y0, y1);
    cutFace(b, A1, rIn, rOut, y0, y1);
}

void Generator::build() {
    MeshBuilder b;
    const Mat4 at = translate(Vec3(BASE.x, 0, BASE.z));

    // ---- housing + cooling ribs --------------------------------------------
    b.setTransform(at);
    ringSolid(b, 4.6f, 4.78f, BASE.y, 19.4f);
    ringPiece(b, 1.2f, 4.78f, 19.4f, A0, A1, true);
    housing = b.build();
    b.clear();
    for (int k = 0; k < 30; k++) {
        float a = A0 + (A1 - A0) * (k + 0.5f) / 30.0f;
        b.setTransform(at * rotateY(-a));
        b.box(Vec3(4.75f, BASE.y + 0.4f, -0.1f), Vec3(5.05f, 19.0f, 0.1f));
    }
    ribs = b.build();

    // top bracket and exciter cap (full, above the window)
    b.clear();
    b.setTransform(at * translate(Vec3(0, 19.4f, 0)));
    b.cylinder(3.0f, 2.4f, 1.4f, 48, false, true);
    b.setTransform(at * translate(Vec3(0, 20.8f, 0)));
    b.cylinder(2.2f, 2.2f, 2.0f, 48, false, true);
    for (int k = 0; k < 6; k++) {
        b.setTransform(at * translate(Vec3(0, 19.4f, 0)) * rotateY(k * PI / 3));
        b.box(Vec3(2.6f, 0, -0.25f), Vec3(4.7f, 0.9f, 0.25f));
    }
    topCap = b.build();

    // ---- stator core (laminations), teeth, slot copper, end windings ------
    b.clear();
    b.setTransform(at);
    ringSolid(b, 4.05f, 4.55f, CORE_Y0, CORE_Y1);
    core = b.build();
    b.clear();
    MeshBuilder cu;
    for (int k = 0; k < 72; k++) {
        float a = -PI + k * 2 * PI / 72;
        bool inWindow = a < A0 || a > A1;
        if (!inWindow) {
            b.setTransform(at * rotateY(-a));
            b.box(Vec3(3.8f, CORE_Y0, -0.085f), Vec3(4.05f, CORE_Y1, 0.085f));
        }
        float ac = a + PI / 72;
        if (ac > A0 && ac < A1) {
            cu.setTransform(at * rotateY(-ac));
            cu.box(Vec3(3.84f, CORE_Y0 - 0.15f, -0.06f), Vec3(4.03f, CORE_Y1 + 0.15f, 0.06f));
        }
    }
    teeth = b.build();
    b.clear();
    // end windings: each coil leaves one slot and returns 6 slots further on
    for (int k = 0; k < 72; k++) {
        float a0 = -PI + (k + 0.5f) * 2 * PI / 72, a1 = a0 + 6 * 2 * PI / 72;
        if (a0 < A0 || a1 > A1) continue;
        for (int side = 0; side < 2; side++) {
            Vec3 prev;
            for (int i = 0; i <= 8; i++) {
                float u = i / 8.0f, a = mixf(a0, a1, u);
                float r = 3.95f + 0.28f * std::sin(PI * u);
                float dy = 0.12f + 0.45f * std::sin(PI * u);
                float y = side == 0 ? CORE_Y1 + dy : CORE_Y0 - dy;
                Vec3 p = Vec3(BASE.x + std::cos(a) * r, y, BASE.z + std::sin(a) * r);
                if (i > 0) cu.beam(prev, p, 0.07f, 0.07f);
                prev = p;
            }
        }
    }
    cu.resetTransform();
    slotCopper = cu.build();
    endWindings = b.build();

    // ---- three output terminals on the +x side ------------------------------
    b.clear();
    MeshBuilder ins;
    for (int p = 0; p < 3; p++) {
        float z = BASE.z + (p - 1) * 1.3f;
        b.box(Vec3(BASE.x + 4.78f, 16.1f, z - 0.4f), Vec3(BASE.x + 5.4f, 17.1f, z + 0.4f));
        for (int d = 0; d < 4; d++) {
            ins.setTransform(translate(Vec3(BASE.x + 5.4f + d * 0.14f, 16.6f, z)) * rotateZ(-PI * 0.5f));
            ins.cylinder(0.28f, 0.22f, 0.12f, 14, true, true);
        }
    }
    ins.resetTransform();
    terminals = b.build();
    insulators = ins.build();

    // ---- rotor (local frame: origin on the shaft axis, real heights) --------
    b.clear();
    b.setTransform(translate(Vec3(0, 14.75f, 0)));
    b.cylinder(3.3f, 3.3f, 3.9f, 72, false, false);
    b.cylinder(3.0f, 3.0f, 3.9f, 72, false, false);
    b.annulus(3.0f, 3.3f, 72);
    b.setTransform(translate(Vec3(0, 18.65f, 0)));
    b.annulus(3.0f, 3.3f, 72);
    b.setTransform(translate(Vec3(0, 14.4f, 0)));
    b.cylinder(0.7f, 0.7f, 4.9f, 32, true, true);                       // hub
    for (int k = 0; k < 8; k++) {                                       // spider arms
        b.setTransform(rotateY(k * PI / 4));
        b.box(Vec3(0.7f, 15.2f, -0.12f), Vec3(3.0f, 18.2f, 0.12f));
    }
    b.resetTransform();
    rotorRim = b.build();
    b.clear();
    MeshBuilder coils;
    for (int k = 0; k < 40; k++) {
        float a = k * 2 * PI / 40;
        b.setTransform(rotateY(-a));
        b.box(Vec3(3.3f, 14.95f, -0.15f), Vec3(3.68f, 18.45f, 0.15f));      // pole core
        b.box(Vec3(3.66f, 14.9f, -0.21f), Vec3(3.74f, 18.5f, 0.21f));       // pole shoe
        coils.setTransform(rotateY(-a));
        coils.box(Vec3(3.36f, 15.0f, 0.15f), Vec3(3.62f, 18.4f, 0.22f));    // field coil (copper)
        coils.box(Vec3(3.36f, 15.0f, -0.22f), Vec3(3.62f, 18.4f, -0.15f));
        coils.box(Vec3(3.36f, 18.4f, -0.22f), Vec3(3.62f, 18.5f, 0.22f));
        coils.box(Vec3(3.36f, 14.9f, -0.22f), Vec3(3.62f, 15.0f, 0.22f));
    }
    b.resetTransform();
    coils.resetTransform();
    rotorPoles = b.build();
    fieldCoils = coils.build();
    b.clear();
    for (int k = 0; k < 2; k++) {
        b.setTransform(translate(Vec3(0, 19.5f + k * 0.35f, 0)));
        b.torus(0.82f, 0.08f, 32, 8);
    }
    b.resetTransform();
    slipRings = b.build();

    // ---- magnetic field lines (rotor frame): pole -> air gap -> stator -> next pole
    fieldLines.begin();
    for (int k = 0; k < 40; k++) {
        float a0 = k * 2 * PI / 40, d = 2 * PI / 40;
        for (int h = 0; h < 3; h++) {
            float y = 15.6f + h * 1.1f;
            std::vector<Vec3> path;
            for (int i = 0; i <= 14; i++) {
                float u = i / 14.0f;
                float a = a0 + d * u, r = 3.76f + 0.62f * std::sin(PI * u);
                path.push_back(Vec3(std::cos(a) * r, y + 0.15f * std::sin(2 * PI * u), std::sin(a) * r));
            }
            fieldLines.add(path, 0.02f);
        }
    }
    fieldLines.finish();

    // ---- induced current (stator frame): slot conductors + ring buses + leads -
    currentLines.begin();
    for (int p = 0; p < 3; p++) {
        for (int ring = 0; ring < 2; ring++) {
            std::vector<Vec3> path;
            float y = ring == 0 ? CORE_Y1 + 0.35f + 0.12f * p : CORE_Y0 - 0.35f - 0.12f * p;
            for (int i = 0; i <= 120; i++) {
                float a = i * 2 * PI / 120 + p * 2 * PI / 3;
                float r = 4.22f + 0.04f * p;
                path.push_back(Vec3(BASE.x + std::cos(a) * r, y + 0.07f * std::sin(a * 20.0f), BASE.z + std::sin(a) * r));
            }
            currentLines.add(path, 0.03f);
        }
    }
    for (int k = 0; k < 36; k++) {                                      // conductors in the slots
        float a = k * 2 * PI / 36;
        std::vector<Vec3> path;
        for (int i = 0; i <= 8; i++)
            path.push_back(Vec3(BASE.x + std::cos(a) * 3.94f, mixf(CORE_Y0 - 0.3f, CORE_Y1 + 0.3f, i / 8.0f), BASE.z + std::sin(a) * 3.94f));
        currentLines.add(path, 0.025f);
    }
    for (int p = 0; p < 3; p++) {                                        // leads to the terminals
        std::vector<Vec3> path;
        float y = CORE_Y1 + 0.35f + 0.12f * p;
        Vec3 t = terminal(p);
        path.push_back(Vec3(BASE.x + 4.22f, y, BASE.z));
        path.push_back(Vec3(BASE.x + 4.5f, y, (BASE.z + t.z) * 0.5f));
        path.push_back(Vec3(BASE.x + 4.9f, 17.2f, t.z));
        path.push_back(t);
        currentLines.add(path, 0.05f);
    }
    currentLines.finish();
}

Vec3 Generator::terminal(int phase) const {
    return Vec3(BASE.x + 6.0f, 16.6f, BASE.z + (phase - 1) * 1.3f);
}

void Generator::draw(Renderer& r, float rotorAngle, float, float, float) {
    Mat4 I;
    Material laminated = Mat::copper();                 // banded pattern = stacked steel sheets
    laminated.color = Vec3(0.34f, 0.36f, 0.39f);
    laminated.patternScale = 6.0f;
    laminated.twoSided = true;
    Material housingMat = Mat::paint(Vec3(0.4f, 0.48f, 0.55f), 0.5f);
    housingMat.twoSided = true;
    r.drawMesh(housing, I, housingMat);
    r.drawMesh(ribs, I, Mat::paint(Vec3(0.36f, 0.44f, 0.5f), 0.5f));
    r.drawMesh(topCap, I, Mat::paint(Vec3(0.45f, 0.58f, 0.66f), 0.6f));
    r.drawMesh(core, I, laminated);
    r.drawMesh(teeth, I, laminated);
    r.drawMesh(slotCopper, I, Mat::copper());
    r.drawMesh(endWindings, I, Mat::copper());
    r.drawMesh(terminals, I, Mat::paint(Vec3(0.3f, 0.32f, 0.35f)));
    r.drawMesh(insulators, I, Mat::plain(Vec3(0.42f, 0.2f, 0.12f), 0.7f, 70.0f));

    // TRANSFORMATION: the rotor turns about the shaft axis
    Mat4 rot = translate(Vec3(BASE.x, 0, BASE.z)) * rotateY(rotorAngle);
    Material darkSteel = laminated;
    darkSteel.color = Vec3(0.18f, 0.19f, 0.21f);
    r.drawMesh(rotorRim, rot, Mat::steel(Vec3(0.4f, 0.42f, 0.45f)));
    r.drawMesh(rotorPoles, rot, darkSteel);
    r.drawMesh(fieldCoils, rot, Mat::copper());
    r.drawMesh(slipRings, rot, Mat::copper());
}

void Generator::drawGlow(Renderer& r, float rotorAngle, float excitation, float current, float freqVisual) {
    Mat4 rot = translate(Vec3(BASE.x, 0, BASE.z)) * rotateY(rotorAngle);
    fieldLines.draw(r, rot, Vec3(0.35f, 0.8f, 1.7f), excitation, 1.2f, 0.9f);
    currentLines.draw(r, Mat4(), Vec3(2.4f, 1.0f, 0.22f), current, 2.0f + 6.0f * freqVisual, 1.2f);
}
