#include "Powerhouse.h"
#include "TurbineUnit.h"
#include "Materials.h"

using namespace HouseDims;

void Powerhouse::build() {
    // penstock: starts at the bulkhead of the intake conduit inside the dam (z = 14),
    // leaves the downstream face, runs down the slope into the turbine pit and ends
    // at the main inlet valve (z = 57).
    std::vector<Vec3> corners = {Vec3(58, 39, 14), Vec3(58, 39, 23), Vec3(58, 9.5f, 47.5f), Vec3(58, 6.0f, 51.5f), Vec3(58, 6.0f, 57.0f)};
    penstock.build(corners, 2.0f, 5.0f, Vec3(-1, 0, 0), 90.0f, 6.0f);

    const float GX = TurbineDims::CENTRE.x, GZ = TurbineDims::CENTRE.z;
    MeshBuilder b;
    // ---- foundation (the pit floor) -----------------------------------------
    b.box(Vec3(X0, -6, Z0), Vec3(X1 + 2, PIT_Y, Z1));
    foundation = b.build();

    // ---- walls + roof (the -x wall is missing: cut-away) --------------------
    b.clear();
    b.box(Vec3(X1, PIT_Y, Z0), Vec3(X1 + 2, ROOF_Y, Z1));                 // back wall
    b.box(Vec3(X0, PIT_Y, Z0), Vec3(X1, ROOF_Y, Z0 + 1.6f));              // dam-side wall
    b.box(Vec3(X0, PIT_Y, Z1 - 1.6f), Vec3(X1, ROOF_Y, Z1));              // river-side wall
    b.box(Vec3(X0 + 6, ROOF_Y, Z0), Vec3(X1 + 2, ROOF_Y + 1.4f, Z1));     // roof (cut back)
    b.box(Vec3(X0 + 6, ROOF_Y + 1.4f, Z0), Vec3(X1 + 2, ROOF_Y + 2.4f, Z0 + 0.3f));
    b.box(Vec3(X0 + 6, ROOF_Y + 1.4f, Z1 - 0.3f), Vec3(X1 + 2, ROOF_Y + 2.4f, Z1));
    for (int k = 0; k < 4; k++) {                                         // window frames outside
        float x = X0 + 4 + k * 5.5f;
        b.box(Vec3(x, 18, Z1), Vec3(x + 3.2f, 18.3f, Z1 + 0.3f));
        b.box(Vec3(x, 24.5f, Z1), Vec3(x + 3.2f, 24.8f, Z1 + 0.3f));
    }
    walls = b.build();

    // ---- big windows in the back wall: a bright hall like the video ---------
    b.clear();
    MeshBuilder m;
    for (int k = 0; k < 8; k++) {
        float z0 = Z0 + 3.0f + k * 3.9f;
        b.box(Vec3(X1 - 0.05f, 17.0f, z0), Vec3(X1, 25.5f, z0 + 3.2f));
        m.box(Vec3(X1 - 0.18f, 17.0f, z0 + 1.55f), Vec3(X1 - 0.02f, 25.5f, z0 + 1.65f));      // mullions
        m.box(Vec3(X1 - 0.18f, 21.2f, z0), Vec3(X1 - 0.02f, 21.3f, z0 + 3.2f));
        m.box(Vec3(X1 - 0.2f, 16.8f, z0 - 0.1f), Vec3(X1 - 0.02f, 17.0f, z0 + 3.3f));
    }
    windows = b.build();
    mullions = m.build();

    // ---- generator floor (with an opening for the generator) ----------------
    b.clear();
    float hx0 = GX - 5.1f, hx1 = GX + 5.1f, hz0 = GZ - 5.1f, hz1 = GZ + 5.1f;
    b.box(Vec3(X0, GEN_FLOOR_Y - 1, Z0 + 1.6f), Vec3(X1, GEN_FLOOR_Y, hz0));
    b.box(Vec3(X0, GEN_FLOOR_Y - 1, hz1), Vec3(X1, GEN_FLOOR_Y, Z1 - 1.6f));
    b.box(Vec3(X0, GEN_FLOOR_Y - 1, hz0), Vec3(hx0, GEN_FLOOR_Y, hz1));
    b.box(Vec3(hx1, GEN_FLOOR_Y - 1, hz0), Vec3(X1, GEN_FLOOR_Y, hz1));
    // columns under the floor
    for (int i = 0; i < 2; i++)
        for (int j = 0; j < 3; j++)
            b.box(Vec3(X0 + 1.0f + i * 22.0f, PIT_Y, Z0 + 4 + j * 12.0f), Vec3(X0 + 2.2f + i * 22.0f, GEN_FLOOR_Y - 1, Z0 + 5.2f + j * 12.0f));
    floors = b.build();

    // cut faces: a low lip where the front wall was sliced
    b.clear();
    b.box(Vec3(X0 - 0.2f, PIT_Y, Z0), Vec3(X0 + 0.6f, PIT_Y + 0.8f, Z1));
    b.box(Vec3(X0 - 0.2f, ROOF_Y - 0.8f, Z0), Vec3(X0 + 0.6f, ROOF_Y, Z0 + 3.0f));
    cutFaces = b.build();

    // ---- yellow railings along the open edge of the floor + stairs ---------
    b.clear();
    for (float z = Z0 + 2; z <= Z1 - 2; z += 1.5f) b.box(Vec3(X0 + 0.3f, GEN_FLOOR_Y, z), Vec3(X0 + 0.4f, GEN_FLOOR_Y + 1.1f, z + 0.08f));
    b.box(Vec3(X0 + 0.25f, GEN_FLOOR_Y + 1.05f, Z0 + 2), Vec3(X0 + 0.45f, GEN_FLOOR_Y + 1.15f, Z1 - 2));
    b.box(Vec3(X0 + 0.28f, GEN_FLOOR_Y + 0.55f, Z0 + 2), Vec3(X0 + 0.42f, GEN_FLOOR_Y + 0.62f, Z1 - 2));
    for (float x = hx0; x <= hx1; x += 1.7f) {                           // around the generator opening
        b.box(Vec3(x, GEN_FLOOR_Y, hz0 - 0.4f), Vec3(x + 0.08f, GEN_FLOOR_Y + 1.1f, hz0 - 0.32f));
        b.box(Vec3(x, GEN_FLOOR_Y, hz1 + 0.32f), Vec3(x + 0.08f, GEN_FLOOR_Y + 1.1f, hz1 + 0.4f));
    }
    b.box(Vec3(hx0, GEN_FLOOR_Y + 1.05f, hz0 - 0.42f), Vec3(hx1, GEN_FLOOR_Y + 1.15f, hz0 - 0.3f));
    b.box(Vec3(hx0, GEN_FLOOR_Y + 1.05f, hz1 + 0.3f), Vec3(hx1, GEN_FLOOR_Y + 1.15f, hz1 + 0.42f));
    railings = b.build();
    b.clear();
    for (int k = 0; k < 18; k++) {
        float y = PIT_Y + k * (GEN_FLOOR_Y - PIT_Y) / 18.0f;
        float z = Z1 - 3.0f - k * 0.55f;
        b.box(Vec3(X0 + 1.0f, PIT_Y, z - 0.55f), Vec3(X0 + 3.0f, y + 0.58f, z));
    }
    stairs = b.build();

    // ---- control cabinets with LEDs on the generator floor ------------------
    b.clear();
    MeshBuilder led;
    for (int k = 0; k < 6; k++) {
        float z = Z0 + 3 + k * 1.4f;
        b.box(Vec3(X1 - 2.6f, GEN_FLOOR_Y, z), Vec3(X1 - 0.6f, GEN_FLOOR_Y + 2.4f, z + 1.2f));
        led.box(Vec3(X1 - 2.65f, GEN_FLOOR_Y + 1.6f, z + 0.2f), Vec3(X1 - 2.6f, GEN_FLOOR_Y + 1.75f, z + 0.5f));
        led.box(Vec3(X1 - 2.65f, GEN_FLOOR_Y + 1.2f, z + 0.6f), Vec3(X1 - 2.6f, GEN_FLOOR_Y + 1.3f, z + 1.0f));
    }
    cabinets = b.build();
    leds = led.build();

    // ---- overhead crane (yellow) and ceiling lights -------------------------
    b.clear();
    b.box(Vec3(X0 + 6, 24.2f, Z0 + 1.6f), Vec3(X0 + 7, 24.8f, Z1 - 1.6f));
    b.box(Vec3(X1 - 1, 24.2f, Z0 + 1.6f), Vec3(X1, 24.8f, Z1 - 1.6f));
    b.box(Vec3(X0 + 6, 24.8f, 50.5f), Vec3(X1, 25.8f, 52.0f));
    b.box(Vec3(X0 + 6, 24.8f, 53.0f), Vec3(X1, 25.8f, 54.5f));
    b.box(Vec3(60, 25.8f, 50.2f), Vec3(64, 26.8f, 54.8f));
    crane = b.build();
    b.clear();
    for (int k = 0; k < 4; k++) b.box(Vec3(54 + k * 4.5f, ROOF_Y - 0.2f, 50), Vec3(55.5f + k * 4.5f, ROOF_Y, 76));
    lights = b.build();

    // ---- shaft (turns) from the runner up to the generator rotor ------------
    b.clear();
    b.setTransform(translate(Vec3(0, TurbineDims::CENTRE.y + 0.9f, 0)));
    b.cylinder(0.42f, 0.42f, 14.5f - (TurbineDims::CENTRE.y + 0.9f), 24, true, true);
    b.setTransform(translate(Vec3(0, 10.4f, 0)));
    b.cylinder(0.85f, 0.85f, 0.35f, 32, true, true);                   // coupling flange
    for (int k = 0; k < 12; k++) {
        float a = k * 2 * PI / 12;
        b.setTransform(translate(Vec3(std::cos(a) * 0.68f, 10.3f, std::sin(a) * 0.68f)));
        b.cylinder(0.06f, 0.06f, 0.55f, 6, true, true);
    }
    for (int k = 0; k < 4; k++) {                                       // marks so you can see it turn
        b.setTransform(rotateY(k * PI * 0.5f));
        b.box(Vec3(0.41f, 8.2f, -0.05f), Vec3(0.44f, 9.8f, 0.05f));
        b.box(Vec3(0.41f, 11.2f, -0.05f), Vec3(0.44f, 12.6f, 0.05f));
    }
    b.resetTransform();
    shaft = b.build();

    // bearings (fixed): turbine guide bearing + bracket under the generator
    b.clear();
    b.setTransform(translate(Vec3(GX, 7.6f, GZ)));
    b.cylinder(1.05f, 1.05f, 0.9f, 32, true, true);
    for (int k = 0; k < 4; k++) {
        b.setTransform(translate(Vec3(GX, 7.6f, GZ)) * rotateY(k * PI * 0.5f + PI * 0.25f));
        b.box(Vec3(1.0f, 0.2f, -0.12f), Vec3(2.4f, 0.6f, 0.12f));
    }
    b.setTransform(translate(Vec3(GX, 12.3f, GZ)));
    b.cylinder(1.2f, 1.2f, 0.9f, 32, true, true);
    for (int k = 0; k < 6; k++) {
        b.setTransform(translate(Vec3(GX, 12.3f, GZ)) * rotateY(k * PI / 3));
        b.box(Vec3(1.1f, 0.25f, -0.15f), Vec3(5.1f, 0.75f, 0.15f));
    }
    b.resetTransform();
    bearings = b.build();

    // draft-tube outlet into the tailrace
    b.clear();
    b.box(Vec3(GX - 3.5f, -6, Z1), Vec3(GX - 2.5f, 3.2f, Z1 + 5));
    b.box(Vec3(GX + 2.5f, -6, Z1), Vec3(GX + 3.5f, 3.2f, Z1 + 5));
    outlet = b.build();
}

void Powerhouse::draw(Renderer& r, float shaftAngle) {
    Mat4 I;
    r.drawMesh(foundation, I, Mat::concrete(1.5f, Vec3(0.5f, 0.5f, 0.49f)));
    r.drawMesh(walls, I, Mat::concrete(1.0f, Vec3(0.62f, 0.62f, 0.6f)));
    r.drawMesh(windows, I, Mat::glow(Vec3(1.5f, 1.65f, 1.8f)));
    r.drawMesh(mullions, I, Mat::paint(Vec3(0.3f, 0.32f, 0.35f)));
    r.drawMesh(floors, I, Mat::concrete(1.5f, Vec3(0.56f, 0.56f, 0.55f)));
    r.drawMesh(cutFaces, I, Mat::concrete(1.0f, Vec3(0.5f, 0.5f, 0.48f)));
    r.drawMesh(railings, I, Mat::paint(Vec3(0.85f, 0.65f, 0.08f), 0.5f));
    r.drawMesh(stairs, I, Mat::concrete(2.0f, Vec3(0.5f, 0.5f, 0.5f)));
    r.drawMesh(cabinets, I, Mat::paint(Vec3(0.5f, 0.52f, 0.54f)));
    r.drawMesh(leds, I, Mat::glow(Vec3(0.2f, 3.0f, 0.8f)));
    r.drawMesh(crane, I, Mat::paint(Vec3(0.85f, 0.62f, 0.08f)));
    r.drawMesh(lights, I, Mat::glow(Vec3(5.0f, 4.6f, 3.8f)));
    r.drawMesh(outlet, I, Mat::concrete(1.0f));
    // TRANSFORMATION: the shaft turns with the runner and the generator rotor
    Mat4 sm = translate(Vec3(TurbineDims::CENTRE.x, 0, TurbineDims::CENTRE.z)) * rotateY(shaftAngle);
    r.drawMesh(shaft, sm, Mat::steel(Vec3(0.7f, 0.72f, 0.75f)));
    r.drawMesh(bearings, I, Mat::paint(Vec3(0.3f, 0.42f, 0.55f), 0.5f));
    penstock.drawShell(r, Mat::steel(Vec3(0.42f, 0.44f, 0.47f)));
}
