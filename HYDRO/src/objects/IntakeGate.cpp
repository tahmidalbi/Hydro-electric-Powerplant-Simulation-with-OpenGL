#include "IntakeGate.h"
#include "Materials.h"

void IntakeGate::build() {
    const float hw = width * 0.5f - 0.1f;
    MeshBuilder b;
    // ---- gate leaf: skin plate (upstream side) + girders (downstream side) --
    b.box(Vec3(-hw, 0, -0.4f), Vec3(hw, height, 0.0f));
    for (int k = 0; k < 5; k++) {
        float y = 0.6f + k * (height - 1.2f) / 4.0f;
        b.box(Vec3(-hw + 0.1f, y - 0.22f, 0.0f), Vec3(hw - 0.1f, y + 0.22f, 0.7f));     // web
        b.box(Vec3(-hw + 0.1f, y - 0.4f, 0.6f), Vec3(hw - 0.1f, y + 0.4f, 0.78f));      // flange
    }
    for (int k = 0; k < 5; k++) {                                                        // stiffeners
        float x = -hw + 0.2f + k * (2.0f * hw - 0.4f) / 4.0f;
        b.box(Vec3(x - 0.12f, 0.1f, 0.0f), Vec3(x + 0.12f, height - 0.1f, 0.65f));
    }
    b.box(Vec3(-hw - 0.05f, height - 0.3f, -0.45f), Vec3(hw + 0.05f, height, 0.85f));     // top beam
    b.box(Vec3(-hw - 0.05f, 0, -0.45f), Vec3(hw + 0.05f, 0.25f, 0.85f));                  // bottom seal
    for (int i = -1; i <= 1; i += 2)                                                     // rod clevises
        b.box(Vec3(i * 3.0f - 0.35f, height, -0.3f), Vec3(i * 3.0f + 0.35f, height + 0.6f, 0.5f));
    leaf = b.build();

    // ---- piston rods: from the gate top up into the cylinders --------------
    b.clear();
    for (int i = -1; i <= 1; i += 2) {
        b.setTransform(translate(Vec3(i * 3.0f, height + 0.6f, 0.1f)));
        b.cylinder(0.2f, 0.2f, 20.0f, 16, false, true);
    }
    rods = b.build();

    // ---- steel guide rails along both walls of the shaft --------------------
    b.clear();
    for (int i = -1; i <= 1; i += 2) {
        float x = i * (width * 0.5f - 0.15f);
        b.box(Vec3(x - 0.15f, 0, -0.6f), Vec3(x + 0.15f, crestAbove, -0.45f));
        b.box(Vec3(x - 0.15f, 0, 0.9f), Vec3(x + 0.15f, crestAbove, 1.05f));
    }
    guides = b.build();

    // ---- hydraulic cylinders standing on a steel portal on the crest --------
    b.clear();
    for (int i = -1; i <= 1; i += 2) {
        b.setTransform(translate(Vec3(i * 3.0f, crestAbove, 0.1f)));
        b.cylinder(0.55f, 0.55f, 11.5f, 24, false, true);                    // barrel (longer than the stroke)
        b.setTransform(translate(Vec3(i * 3.0f, crestAbove, 0.1f)));
        b.cylinder(0.8f, 0.8f, 0.4f, 24, true, true);                        // base flange
        b.setTransform(translate(Vec3(i * 3.0f, crestAbove + 11.5f, 0.1f)));
        b.cylinder(0.75f, 0.75f, 0.45f, 24, true, true);                     // head
        b.resetTransform();
        b.box(Vec3(i * 3.0f + 0.5f, crestAbove + 0.6f, -0.05f), Vec3(i * 3.0f + 0.75f, crestAbove + 11.0f, 0.2f));   // hose
    }
    b.resetTransform();
    cylinders = b.build();
    b.clear();
    b.box(Vec3(-width * 0.5f, crestAbove, -1.2f), Vec3(width * 0.5f, crestAbove + 0.5f, 1.4f));     // plate over the shaft
    for (int i = -1; i <= 1; i += 2)
        b.box(Vec3(i * 5.0f - 0.3f, crestAbove + 0.5f, -0.3f), Vec3(i * 5.0f + 0.3f, crestAbove + 12.5f, 0.5f));
    b.box(Vec3(-5.3f, crestAbove + 12.0f, -0.3f), Vec3(5.3f, crestAbove + 12.6f, 0.5f));
    frame = b.build();

    // ---- wire-mesh screen on the far wall, just upstream of the gate --------
    // (the diamond-mesh panel standing beside the gate in the video)
    b.clear();
    {
        float x = width * 0.5f - 0.05f, z0 = -3.3f, z1 = -1.0f;
        unsigned int k = (unsigned int)b.verts.size();
        Vec3 n(-1, 0, 0);
        b.addVertex(Vec3(x, 0, z0), n, Vec2(z0, 0));
        b.addVertex(Vec3(x, 0, z1), n, Vec2(z1, 0));
        b.addVertex(Vec3(x, height, z1), n, Vec2(z1, height));
        b.addVertex(Vec3(x, height, z0), n, Vec2(z0, height));
        b.tri(k, k + 1, k + 2);
        b.tri(k, k + 2, k + 3);
    }
    ladder = b.build();
    b.clear();
    {
        float x = width * 0.5f - 0.12f;
        b.beam(Vec3(x, 0, -3.3f), Vec3(x, height, -3.3f), 0.15f, 0.15f);
        b.beam(Vec3(x, 0, -1.0f), Vec3(x, height, -1.0f), 0.15f, 0.15f);
        for (float y = 0.8f; y < height; y += 2.2f) b.beam(Vec3(x, y, -3.3f), Vec3(x, y, -1.0f), 0.1f, 0.1f);
    }
    ladderFrame = b.build();
}

void IntakeGate::draw(Renderer& r, const Mat4& base, float lift) {
    Material gateMat = Mat::paint(Vec3(0.3f, 0.32f, 0.34f), 0.55f);
    gateMat.reflect = 0.1f;
    Mat4 up = base * translate(Vec3(0, lift, 0));          // the moving part
    r.drawMesh(leaf, up, gateMat);
    r.drawMesh(rods, up, Mat::steel(Vec3(0.8f, 0.82f, 0.85f)));   // rods move WITH the gate
    r.drawMesh(guides, base, Mat::paint(Vec3(0.22f, 0.23f, 0.25f)));
    r.drawMesh(cylinders, base, Mat::paint(Vec3(0.55f, 0.57f, 0.6f), 0.7f));
    r.drawMesh(frame, base, Mat::paint(Vec3(0.25f, 0.27f, 0.3f)));
    Material mesh = Mat::steel(Vec3(0.5f, 0.52f, 0.55f));
    mesh.pattern = PAT_GRATING;
    mesh.patternScale = 4.0f;
    mesh.twoSided = true;
    r.drawMesh(ladder, base, mesh);
    r.drawMesh(ladderFrame, base, Mat::steel(Vec3(0.45f, 0.47f, 0.5f)));
}
