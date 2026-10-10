#include "MainInletValve.h"
#include "Materials.h"

using namespace ValveDims;

// cylinder whose axis runs along +z from z0 to z1 (only angles a0..a1, measured from +x)
static void zCylinder(MeshBuilder& b, float r, float z0, float z1, float a0, float a1, bool caps) {
    Mat4 saved = b.xf;
    b.setTransform(alignY(Vec3(X, Y, z0), Vec3(0, 0, 1)));
    b.cylinder(r, r, z1 - z0, 48, caps, caps, a0, a1);
    b.setTransform(saved);
}

void MainInletValve::build() {
    MeshBuilder b;
    // ---- flanges at both ends, with bolts ---------------------------------
    for (int e = 0; e < 2; e++) {
        float z = e == 0 ? Z0 : Z1 - 0.4f;
        zCylinder(b, R + 0.75f, z, z + 0.4f, 0, 2 * PI, false);
        zCylinder(b, R, z, z + 0.4f, 0, 2 * PI, false);
        for (int s = 0; s < 2; s++) {
            b.setTransform(alignY(Vec3(X, Y, z + s * 0.4f), Vec3(0, 0, 1)));
            b.annulus(R, R + 0.75f, 48);
        }
        b.resetTransform();
    }
    flanges = b.build();
    b.clear();
    for (int e = 0; e < 2; e++)
        for (int k = 0; k < 24; k++) {
            float a = k * 2 * PI / 24, z = e == 0 ? Z0 - 0.1f : Z1 - 0.5f;
            b.cylinderBetween(Vec3(X + std::cos(a) * (R + 0.45f), Y + std::sin(a) * (R + 0.45f), z),
                              Vec3(X + std::cos(a) * (R + 0.45f), Y + std::sin(a) * (R + 0.45f), z + 0.6f), 0.09f, 6);
        }
    bolts = b.build();

    // ---- valve body: only the far half (x > 58) so we can look inside -------
    b.clear();
    // in zCylinder's frame local x = world +x, local z = world -y, so a in [-90, 90] = far half
    zCylinder(b, R + 0.25f, Z0 + 0.4f, Z1 - 0.4f, -PI * 0.5f, PI * 0.5f, false);
    b.box(Vec3(X, Y - 2.6f, Z0 + 0.7f), Vec3(X + 2.5f, Y + 2.8f, Z1 - 0.7f));          // gate chest
    b.box(Vec3(X - 2.5f, Y - 2.6f, Z0 + 0.7f), Vec3(X - 2.2f, Y + 2.8f, Z1 - 0.7f));   // near rim of the chest (cut edge)
    b.box(Vec3(X - 2.5f, Y - 2.9f, Z0 + 0.7f), Vec3(X + 2.5f, Y - 2.6f, Z1 - 0.7f));   // chest bottom
    body = b.build();

    // bonnet (the tall housing the gate slides into) - far half + frame
    b.clear();
    b.box(Vec3(X, Y + 2.8f, Z0 + 1.0f), Vec3(X + 2.3f, Y + 6.8f, Z1 - 1.0f));
    b.box(Vec3(X - 2.3f, Y + 2.8f, Z0 + 1.0f), Vec3(X - 2.05f, Y + 6.8f, Z1 - 1.0f));
    b.box(Vec3(X - 2.5f, Y + 6.8f, Z0 + 0.8f), Vec3(X + 2.5f, Y + 7.1f, Z1 - 0.8f));    // bonnet cover
    b.box(Vec3(X - 0.9f, Y + 7.1f, Z0 + 1.5f), Vec3(X - 0.7f, Y + 8.6f, Z1 - 1.5f));    // yoke posts
    b.box(Vec3(X + 0.7f, Y + 7.1f, Z0 + 1.5f), Vec3(X + 0.9f, Y + 8.6f, Z1 - 1.5f));
    b.box(Vec3(X - 1.0f, Y + 8.6f, Z0 + 1.4f), Vec3(X + 1.0f, Y + 8.8f, Z1 - 1.4f));
    bonnet = b.build();

    // ---- the gate (moves up) + its stem ----------------------------------
    b.clear();
    b.box(Vec3(X - 2.15f, Y - 2.25f, (Z0 + Z1) * 0.5f - 0.22f), Vec3(X + 2.15f, Y + 2.25f, (Z0 + Z1) * 0.5f + 0.22f));
    for (int k = 0; k < 3; k++)                                                           // stiffening ribs
        b.box(Vec3(X - 2.1f, Y - 1.6f + k * 1.6f, (Z0 + Z1) * 0.5f + 0.22f), Vec3(X + 2.1f, Y - 1.4f + k * 1.6f, (Z0 + Z1) * 0.5f + 0.36f));
    gate = b.build();
    b.clear();
    b.cylinderBetween(Vec3(X, Y + 2.25f, (Z0 + Z1) * 0.5f), Vec3(X, Y + 9.2f, (Z0 + Z1) * 0.5f), 0.12f, 10);
    stem = b.build();

    // ---- hand-wheel (turns around the stem) ------------------------------
    b.clear();
    b.torus(1.0f, 0.07f, 40, 8);
    for (int k = 0; k < 4; k++) b.beam(Vec3(0, 0, 0), Vec3(std::cos(k * PI * 0.5f), 0, std::sin(k * PI * 0.5f)) * 0.98f, 0.06f, 0.06f);
    b.cylinder(0.15f, 0.15f, 0.3f, 12, true, true);
    wheel = b.build();

    // blue electric actuator beside the wheel (as in the video)
    b.clear();
    b.box(Vec3(X + 1.1f, Y + 8.8f, Z0 + 1.2f), Vec3(X + 2.2f, Y + 9.8f, Z0 + 2.4f));
    b.setTransform(translate(Vec3(X + 1.65f, Y + 9.3f, Z0 + 1.2f)) * rotateX(-PI * 0.5f));
    b.cylinder(0.35f, 0.35f, 0.5f, 16, true, true);
    b.resetTransform();
    actuator = b.build();
}

void MainInletValve::draw(Renderer& r, float open01, float wheelAngle) {
    Mat4 I;
    Material steel = Mat::steel(Vec3(0.45f, 0.47f, 0.5f));
    r.drawMesh(flanges, I, steel);
    r.drawMesh(bolts, I, Mat::steel(Vec3(0.3f, 0.31f, 0.33f)));
    r.drawMesh(body, I, Mat::paint(Vec3(0.55f, 0.57f, 0.6f), 0.6f));
    r.drawMesh(bonnet, I, Mat::paint(Vec3(0.55f, 0.57f, 0.6f), 0.6f));
    // TRANSFORMATION: gate and stem rise together; the wheel spins about the stem
    Mat4 lift = translate(Vec3(0, open01 * STROKE, 0));
    r.drawMesh(gate, lift, Mat::paint(Vec3(0.32f, 0.34f, 0.37f), 0.6f));
    r.drawMesh(stem, lift, Mat::steel(Vec3(0.75f, 0.76f, 0.78f)));
    r.drawMesh(wheel, translate(Vec3(X, Y + 9.2f, (Z0 + Z1) * 0.5f)) * rotateY(wheelAngle), Mat::steel(Vec3(0.6f, 0.62f, 0.65f)));
    r.drawMesh(actuator, I, Mat::paint(Vec3(0.08f, 0.25f, 0.62f), 0.6f));
}
