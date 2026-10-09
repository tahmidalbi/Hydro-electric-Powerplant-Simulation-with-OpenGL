#include "Powerhouse.h"
#include "Materials.h"

// Building footprint (world metres)
static const float X0 = 48, X1 = 68, Z0 = 48, Z1 = 74;
static const float FLOOR_Y = 4, GEN_FLOOR_Y = 12, ROOF_Y = 24;
static const Vec3 UNIT(58, 6.5f, 61);      // centre of the turbine (on the pipe axis)

void Powerhouse::build() {
    // penstock: out of the dam face, down the slope, horizontal through the building
    std::vector<Vec3> corners = {Vec3(58, 38, 24), Vec3(58, 9, 48.9f), Vec3(58, 6.5f, 55), Vec3(58, 6.5f, 80)};
    penstock.build(corners, 2.0f, 5.0f, Vec3(-1, 0, 0), 90.0f, 6.0f);

    MeshBuilder b;
    // ---- substructure + walls (the -x wall is missing: cut-away) ------------
    b.box(Vec3(X0, -3, Z0), Vec3(X1 + 2, FLOOR_Y, Z1));                 // foundation block
    b.box(Vec3(X1, FLOOR_Y, Z0), Vec3(X1 + 2, ROOF_Y, Z1));             // back wall
    b.box(Vec3(X0, FLOOR_Y, Z0), Vec3(X1, ROOF_Y, Z0 + 1.6f));          // dam-side wall
    b.box(Vec3(X0, FLOOR_Y, Z1 - 1.6f), Vec3(X1, ROOF_Y, Z1));          // river-side wall
    b.box(Vec3(X0 + 6, ROOF_Y, Z0), Vec3(X1 + 2, ROOF_Y + 1.4f, Z1));   // roof (cut back)
    b.box(Vec3(X0 + 6, ROOF_Y + 1.4f, Z0), Vec3(X1 + 2, ROOF_Y + 2.4f, Z0 + 0.3f)); // roof parapet
    b.box(Vec3(X0 + 6, ROOF_Y + 1.4f, Z1 - 0.3f), Vec3(X1 + 2, ROOF_Y + 2.4f, Z1));
    // windows frames on the river-side wall (outside)
    for (int k = 0; k < 4; k++) {
        float x = X0 + 3 + k * 4.5f;
        b.box(Vec3(x, 15, Z1), Vec3(x + 2.6f, 15.3f, Z1 + 0.3f));
        b.box(Vec3(x, 19.5f, Z1), Vec3(x + 2.6f, 19.8f, Z1 + 0.3f));
    }
    walls = b.build();

    // ---- generator floor with a square hole for the unit --------------------
    b.clear();
    b.box(Vec3(X0, GEN_FLOOR_Y - 1, Z0 + 1.6f), Vec3(X1, GEN_FLOOR_Y, 56));
    b.box(Vec3(X0, GEN_FLOOR_Y - 1, 66), Vec3(X1, GEN_FLOOR_Y, Z1 - 1.6f));
    b.box(Vec3(X0, GEN_FLOOR_Y - 1, 56), Vec3(53, GEN_FLOOR_Y, 66));
    b.box(Vec3(63, GEN_FLOOR_Y - 1, 56), Vec3(X1, GEN_FLOOR_Y, 66));
    // columns supporting the floor + pipe saddles
    b.box(Vec3(X0 + 0.5f, FLOOR_Y, Z0 + 2), Vec3(X0 + 1.5f, GEN_FLOOR_Y - 1, Z0 + 3));
    b.box(Vec3(X0 + 0.5f, FLOOR_Y, Z1 - 3), Vec3(X0 + 1.5f, GEN_FLOOR_Y - 1, Z1 - 2));
    for (float z = 52; z < 74; z += 7) b.box(Vec3(55.5f, FLOOR_Y, z), Vec3(60.5f, 5.0f, z + 1.2f));
    floors = b.build();

    // cut faces: a low lip where the front wall was sliced
    b.clear();
    b.box(Vec3(X0 - 0.2f, FLOOR_Y, Z0), Vec3(X0 + 0.6f, FLOOR_Y + 1.0f, Z1));
    b.box(Vec3(X0 - 0.2f, ROOF_Y - 0.8f, Z0), Vec3(X0 + 0.6f, ROOF_Y, Z0 + 3.0f));
    cutFaces = b.build();

    // ---- generator ---------------------------------------------------------
    // stator housing: cylinder with a 120 degree window facing the camera (-x)
    b.clear();
    b.setTransform(translate(Vec3(UNIT.x, GEN_FLOOR_Y, UNIT.z)));
    float w0 = PI + radians(60), w1 = PI - radians(60) + 2 * PI;   // keep everything except the -x window
    b.cylinder(4.6f, 4.6f, 5.2f, 64, false, false, w0 - 2 * PI, w1 - 2 * PI);
    b.cylinder(4.1f, 4.1f, 5.2f, 64, false, false, w0 - 2 * PI, w1 - 2 * PI);
    b.setTransform(translate(Vec3(UNIT.x, GEN_FLOOR_Y + 5.2f, UNIT.z)));
    b.annulus(1.2f, 4.6f, 64);
    b.setTransform(translate(Vec3(UNIT.x, GEN_FLOOR_Y, UNIT.z)));
    b.annulus(3.9f, 4.6f, 64);
    // cooling vents (ribs) on the outside
    for (int k = 0; k < 24; k++) {
        float a = k * 2 * PI / 24;
        float ad = std::fmod(a - PI + 4 * PI, 2 * PI);
        if (ad < radians(64) || ad > 2 * PI - radians(64)) continue;      // inside the window
        b.setTransform(translate(Vec3(UNIT.x, GEN_FLOOR_Y, UNIT.z)) * rotateY(-a));
        b.box(Vec3(4.55f, 0.4f, -0.12f), Vec3(4.9f, 4.8f, 0.12f));
    }
    b.resetTransform();
    stator = b.build();

    b.clear();     // top bracket + exciter (blue-grey cap like the video)
    b.setTransform(translate(Vec3(UNIT.x, GEN_FLOOR_Y + 5.2f, UNIT.z)));
    b.cylinder(3.0f, 2.4f, 1.4f, 48, false, true);
    b.setTransform(translate(Vec3(UNIT.x, GEN_FLOOR_Y + 6.6f, UNIT.z)));
    b.cylinder(2.2f, 2.2f, 2.0f, 48, false, true);
    for (int k = 0; k < 6; k++) {                                       // bracket arms
        float a = k * PI / 3;
        b.setTransform(translate(Vec3(UNIT.x, GEN_FLOOR_Y + 5.2f, UNIT.z)) * rotateY(a));
        b.box(Vec3(2.6f, 0, -0.25f), Vec3(4.6f, 0.9f, 0.25f));
    }
    b.resetTransform();
    statorTop = b.build();

    // rotor (copper) and its poles - built around the origin so it can spin
    b.clear();
    b.cylinder(3.5f, 3.5f, 4.6f, 48, true, true);
    rotor = b.build();
    b.clear();
    for (int k = 0; k < 14; k++) {
        float a = k * 2 * PI / 14;
        b.setTransform(rotateY(a));
        b.box(Vec3(3.4f, 0.15f, -0.45f), Vec3(3.95f, 4.45f, 0.45f));
    }
    b.resetTransform();
    poles = b.build();

    // shaft from the turbine up to the generator
    b.clear();
    b.cylinder(0.45f, 0.45f, GEN_FLOOR_Y + 0.4f - UNIT.y, 20, true, true);
    for (int k = 0; k < 4; k++) {                                       // coupling flanges
        b.setTransform(translate(Vec3(0, 2.0f + k * 1.2f, 0)));
        b.cylinder(0.75f, 0.75f, 0.25f, 20, true, true);
    }
    b.resetTransform();
    shaft = b.build();

    // ---- bulb turbine inside the pipe (axis = +z) --------------------------
    b.clear();
    b.setTransform(rotateX(PI * 0.5f));                                   // local +Y -> world +z
    b.cylinder(0.25f, 0.95f, 1.6f, 32, false, false);                     // nose cone
    b.setTransform(rotateX(PI * 0.5f) * translate(Vec3(0, 1.6f, 0)));
    b.cylinder(0.95f, 0.95f, 2.4f, 32, false, false);
    b.setTransform(rotateX(PI * 0.5f) * translate(Vec3(0, 4.0f, 0)));
    b.cylinder(0.95f, 0.3f, 1.4f, 32, false, true);
    b.setTransform(translate(Vec3(0, 0, 0)));
    b.sphere(0.26f, 16, 8);
    b.resetTransform();
    bulb = b.build();

    b.clear();       // runner blades around the z axis
    for (int k = 0; k < 5; k++) {
        float a = k * 2 * PI / 5;
        b.setTransform(rotateZ(a) * rotateY(0.5f));
        b.box(Vec3(-0.08f, 0.85f, -0.5f), Vec3(0.08f, 1.7f, 0.5f));
    }
    b.resetTransform();
    blades = b.build();

    // ---- equipment on the generator floor -----------------------------------
    b.clear();
    MeshBuilder led;
    for (int k = 0; k < 4; k++) {
        float z = 51 + k * 1.4f;
        b.box(Vec3(64.5f, GEN_FLOOR_Y, z), Vec3(66.5f, GEN_FLOOR_Y + 2.4f, z + 1.2f));
        led.box(Vec3(64.45f, GEN_FLOOR_Y + 1.6f, z + 0.2f), Vec3(64.5f, GEN_FLOOR_Y + 1.75f, z + 0.5f));
        led.box(Vec3(64.45f, GEN_FLOOR_Y + 1.2f, z + 0.6f), Vec3(64.5f, GEN_FLOOR_Y + 1.3f, z + 1.0f));
    }
    b.box(Vec3(50, GEN_FLOOR_Y, 50.5f), Vec3(51.5f, GEN_FLOOR_Y + 1.8f, 53));
    b.box(Vec3(50, GEN_FLOOR_Y, 53.4f), Vec3(51.5f, GEN_FLOOR_Y + 1.8f, 55.4f));
    // railings around the floor hole
    for (float z = 56; z <= 66; z += 1.25f) b.box(Vec3(52.9f, GEN_FLOOR_Y, z), Vec3(53.0f, GEN_FLOOR_Y + 1.1f, z + 0.08f));
    b.box(Vec3(52.85f, GEN_FLOOR_Y + 1.05f, 56), Vec3(53.05f, GEN_FLOOR_Y + 1.15f, 66));
    cabinets = b.build();
    leds = led.build();

    // big cooling motor / exciter transformer (round, blue-grey) next to the unit
    b.clear();
    b.setTransform(translate(Vec3(60.5f, GEN_FLOOR_Y + 2.0f, 69.0f)) * rotateX(PI * 0.5f));
    b.cylinder(1.8f, 1.8f, 3.2f, 40, true, true);
    for (int k = 0; k < 10; k++) {
        b.setTransform(translate(Vec3(60.5f, GEN_FLOOR_Y + 2.0f, 69.2f + k * 0.3f)) * rotateX(PI * 0.5f));
        b.cylinder(1.95f, 1.95f, 0.1f, 40, false, false);
    }
    b.resetTransform();
    b.box(Vec3(58.9f, GEN_FLOOR_Y, 68.6f), Vec3(62.1f, GEN_FLOOR_Y + 0.4f, 72.6f));
    motor = b.build();

    // overhead crane
    b.clear();
    b.box(Vec3(X0 + 6, 21.2f, Z0 + 1.6f), Vec3(X0 + 7, 21.8f, Z1 - 1.6f));
    b.box(Vec3(X1 - 1, 21.2f, Z0 + 1.6f), Vec3(X1, 21.8f, Z1 - 1.6f));
    b.box(Vec3(X0 + 6, 21.8f, 59.5f), Vec3(X1, 22.8f, 61.0f));
    b.box(Vec3(X0 + 6, 21.8f, 62.0f), Vec3(X1, 22.8f, 63.5f));
    b.box(Vec3(56, 22.8f, 59.2f), Vec3(60, 23.8f, 63.8f));
    crane = b.build();

    b.clear();
    for (int k = 0; k < 3; k++) b.box(Vec3(54 + k * 5.0f, ROOF_Y - 0.2f, 52), Vec3(55.5f + k * 5.0f, ROOF_Y, 70));
    lights = b.build();
}

void Powerhouse::draw(Renderer& r, float rotorAngle) {
    Mat4 I;
    r.drawMesh(walls, I, Mat::concrete(1.0f, Vec3(0.64f, 0.64f, 0.62f)));
    r.drawMesh(floors, I, Mat::concrete(1.5f, Vec3(0.55f, 0.55f, 0.53f)));
    r.drawMesh(cutFaces, I, Mat::concrete(1.0f, Vec3(0.5f, 0.5f, 0.48f)));
    r.drawMesh(stator, I, Mat::paint(Vec3(0.42f, 0.5f, 0.56f), 0.5f));
    r.drawMesh(statorTop, I, Mat::paint(Vec3(0.45f, 0.58f, 0.66f), 0.6f));

    // TRANSFORMATIONS: the rotor, shaft and turbine runner all spin together
    Mat4 rotorM = translate(Vec3(UNIT.x, GEN_FLOOR_Y + 0.3f, UNIT.z)) * rotateY(rotorAngle);
    r.drawMesh(rotor, rotorM, Mat::copper());
    r.drawMesh(poles, rotorM, Mat::paint(Vec3(0.55f, 0.18f, 0.12f), 0.7f));
    r.drawMesh(shaft, translate(UNIT) * rotateY(rotorAngle), Mat::steel());
    r.drawMesh(bulb, translate(Vec3(UNIT.x, UNIT.y, UNIT.z - 2.6f)), Mat::steel(Vec3(0.75f, 0.76f, 0.78f)));
    r.drawMesh(blades, translate(Vec3(UNIT.x, UNIT.y, UNIT.z + 1.0f)) * rotateZ(rotorAngle * 1.0f), Mat::steel(Vec3(0.7f, 0.68f, 0.6f)));

    r.drawMesh(cabinets, I, Mat::paint(Vec3(0.5f, 0.52f, 0.54f)));
    r.drawMesh(leds, I, Mat::glow(Vec3(0.2f, 3.0f, 0.8f)));
    r.drawMesh(motor, I, Mat::paint(Vec3(0.35f, 0.47f, 0.55f), 0.6f));
    r.drawMesh(crane, I, Mat::paint(Vec3(0.8f, 0.6f, 0.08f)));
    r.drawMesh(lights, I, Mat::glow(Vec3(5.0f, 4.6f, 3.8f)));
    penstock.drawShell(r, Mat::steel(Vec3(0.42f, 0.44f, 0.47f)));
}
