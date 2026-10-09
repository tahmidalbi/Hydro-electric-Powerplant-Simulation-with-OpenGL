#include "IntakeGate.h"
#include "Materials.h"

// Gate plane is x = 0 (water flows towards +x), channel z = -7 .. 7.
void IntakeGate::build() {
    MeshBuilder b;
    // ---- gate leaf: skin plate + horizontal girders + stiffeners -----------
    b.box(Vec3(-0.35f, 0, -6.8f), Vec3(0.05f, height, 6.8f));
    for (int k = 0; k < 5; k++) {
        float y = 0.6f + k * (height - 1.2f) / 4.0f;
        b.box(Vec3(0.05f, y - 0.22f, -6.7f), Vec3(0.75f, y + 0.22f, 6.7f));        // web
        b.box(Vec3(0.65f, y - 0.4f, -6.7f), Vec3(0.8f, y + 0.4f, 6.7f));           // flange
    }
    for (int k = 0; k < 5; k++) {
        float z = -6.6f + k * 13.2f / 4.0f;
        b.box(Vec3(0.05f, 0.1f, z - 0.12f), Vec3(0.7f, height - 0.1f, z + 0.12f));
    }
    b.box(Vec3(-0.4f, height - 0.3f, -6.9f), Vec3(0.85f, height, 6.9f));             // top beam
    b.box(Vec3(-0.4f, 0, -6.9f), Vec3(0.85f, 0.25f, 6.9f));                          // bottom seal beam
    for (int i = -1; i <= 1; i++)                                                    // rod clevises
        b.box(Vec3(-0.3f, height, i * 4.5f - 0.35f), Vec3(0.5f, height + 0.6f, i * 4.5f + 0.35f));
    leaf = b.build();

    // ---- guide frame on both sides + front post ----------------------------
    b.clear();
    for (int side = -1; side <= -1; side += 2) {     // back guide only (front one would hide the gate)
        float z = side * 7.4f;
        b.box(Vec3(-0.9f, -1, z - 0.6f), Vec3(1.3f, 21.0f, z + 0.6f));
    }
    guides = b.build();

    // ---- hoist deck with three hydraulic cylinders --------------------------
    b.clear();
    b.box(Vec3(-3.5f, 21.0f, -9), Vec3(3.5f, 22.4f, 8.2f));
    for (float z = -8.5f; z <= 8.0f; z += 1.4f) b.box(Vec3(3.3f, 22.4f, z), Vec3(3.4f, 23.5f, z + 0.08f));
    b.box(Vec3(3.28f, 23.4f, -8.5f), Vec3(3.42f, 23.5f, 8.0f));
    deck = b.build();
    b.clear();
    for (int i = -1; i <= 1; i++) {
        b.setTransform(translate(Vec3(0.2f, 22.4f, i * 4.5f)));
        b.cylinder(0.55f, 0.55f, 10.0f, 24, false, true);                    // barrel (longer than the stroke)
        b.setTransform(translate(Vec3(0.2f, 22.4f, i * 4.5f)));
        b.cylinder(0.8f, 0.8f, 0.4f, 24, true, true);                        // base flange
        b.setTransform(translate(Vec3(0.2f, 32.4f, i * 4.5f)));
        b.cylinder(0.75f, 0.75f, 0.45f, 24, true, true);                     // head
        b.resetTransform();
        b.box(Vec3(0.6f, 23.5f, i * 4.5f - 0.1f), Vec3(1.0f, 31.0f, i * 4.5f + 0.1f));   // hose
    }
    cylinders = b.build();
    b.clear();
    for (int i = -1; i <= 1; i++) {      // rods hang from the gate top up into the barrels
        b.setTransform(translate(Vec3(0.1f, height + 0.6f, i * 4.5f)));
        b.cylinder(0.2f, 0.2f, 12.0f, 16, false, true);
    }
    b.resetTransform();
    rods = b.build();

    // ---- wire-mesh screen on the reservoir side (the "ladder" in the video) -
    b.clear();
    {
        Vec3 a(-4.2f, 0, 7.2f), c(-1.0f, 0, 7.2f), d(-1.0f, 19.0f, 7.2f), e(-2.0f, 19.0f, 7.2f);
        unsigned int k = (unsigned int)b.verts.size();
        Vec3 n(0, 0, 1);
        b.addVertex(a, n, Vec2(a.x, a.y));
        b.addVertex(c, n, Vec2(c.x, c.y));
        b.addVertex(d, n, Vec2(d.x, d.y));
        b.addVertex(e, n, Vec2(e.x, e.y));
        b.tri(k, k + 1, k + 2);
        b.tri(k, k + 2, k + 3);
    }
    grating = b.build();
    b.clear();
    b.beam(Vec3(-4.2f, 0, 7.2f), Vec3(-2.0f, 19.0f, 7.2f), 0.18f, 0.18f);
    b.beam(Vec3(-1.0f, 0, 7.2f), Vec3(-1.0f, 19.0f, 7.2f), 0.22f, 0.22f);
    for (float y = 0.2f; y < 19.0f; y += 2.4f) {
        float x0 = mixf(-4.2f, -2.0f, y / 19.0f);
        b.beam(Vec3(x0, y, 7.2f), Vec3(-1.0f, y, 7.2f), 0.12f, 0.12f);
    }
    gratingFrame = b.build();
}

void IntakeGate::draw(Renderer& r, float lift) {
    Mat4 I;
    Material gateMat = Mat::paint(Vec3(0.3f, 0.32f, 0.34f), 0.55f);
    gateMat.reflect = 0.1f;
    Mat4 up = translate(Vec3(0, lift, 0));         // the moving part
    r.drawMesh(leaf, up, gateMat);
    r.drawMesh(rods, up, Mat::steel(Vec3(0.8f, 0.82f, 0.85f)));   // rods move WITH the gate
    r.drawMesh(guides, I, Mat::paint(Vec3(0.22f, 0.23f, 0.25f)));
    r.drawMesh(cylinders, I, Mat::paint(Vec3(0.55f, 0.57f, 0.6f), 0.7f));
    r.drawMesh(deck, I, Mat::darkConcrete(1.5f));
    Material mesh = Mat::steel(Vec3(0.5f, 0.52f, 0.55f));
    mesh.pattern = PAT_GRATING;
    mesh.patternScale = 4.0f;
    mesh.twoSided = true;
    r.drawMesh(grating, I, mesh);
    r.drawMesh(gratingFrame, I, Mat::steel(Vec3(0.45f, 0.47f, 0.5f)));
}
