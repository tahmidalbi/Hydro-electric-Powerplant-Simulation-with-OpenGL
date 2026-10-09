#include "Transformer.h"
#include "Materials.h"

using namespace TrafoDims;

void Transformer::build(const Vec3 terminals[3]) {
    MeshBuilder b;
    // ---- tank, cover, radiators ---------------------------------------------
    b.box(Vec3(X0, BASE_Y + 0.4f, Z0), Vec3(X1, TOP_Y, Z1));
    b.box(Vec3(X0 + 0.5f, BASE_Y, Z0 + 0.5f), Vec3(X0 + 1.0f, BASE_Y + 0.4f, Z1 - 0.5f));   // rails
    b.box(Vec3(X1 - 1.0f, BASE_Y, Z0 + 0.5f), Vec3(X1 - 0.5f, BASE_Y + 0.4f, Z1 - 0.5f));
    for (int k = 0; k < 6; k++) {                                                          // stiffening ribs
        float x = X0 + 0.6f + k * 1.15f;
        b.box(Vec3(x, BASE_Y + 0.6f, Z0 - 0.12f), Vec3(x + 0.15f, TOP_Y - 0.2f, Z0));
        b.box(Vec3(x, BASE_Y + 0.6f, Z1), Vec3(x + 0.15f, TOP_Y - 0.2f, Z1 + 0.12f));
    }
    tank = b.build();
    b.clear();
    b.box(Vec3(X0 - 0.15f, TOP_Y, Z0 - 0.15f), Vec3(X1 + 0.15f, TOP_Y + 0.3f, Z1 + 0.15f));
    tankCover = b.build();
    b.clear();
    for (int side = 0; side < 2; side++) {
        float z0 = side == 0 ? Z0 - 2.0f : Z1 + 0.3f;
        for (int k = 0; k < 12; k++) {
            float x = X0 + 0.4f + k * 0.55f;
            b.box(Vec3(x, BASE_Y + 1.8f, z0), Vec3(x + 0.12f, TOP_Y - 0.6f, z0 + 1.7f));
        }
        b.box(Vec3(X0 + 0.3f, TOP_Y - 0.7f, z0 + 0.7f), Vec3(X1 - 0.3f, TOP_Y - 0.4f, z0 + 1.0f));   // headers
        b.box(Vec3(X0 + 0.3f, BASE_Y + 1.6f, z0 + 0.7f), Vec3(X1 - 0.3f, BASE_Y + 1.9f, z0 + 1.0f));
    }
    radiators = b.build();

    // fans under the radiators (blades turn)
    b.clear();
    for (int i = 0; i < 4; i++) {
        float z = (i < 2) ? Z0 - 1.15f : Z1 + 1.15f;
        fanPos[i] = Vec3(X0 + 1.8f + (i % 2) * 3.4f, BASE_Y + 1.05f, z);
        b.setTransform(translate(fanPos[i]) * rotateX(PI * 0.5f));
        b.torus(0.62f, 0.06f, 32, 6);
        b.resetTransform();
        b.box(fanPos[i] + Vec3(-0.05f, -0.7f, -0.05f), fanPos[i] + Vec3(0.05f, 0.0f, 0.05f));
    }
    fanFrames = b.build();
    b.clear();
    for (int k = 0; k < 5; k++) {                                   // one fan rotor (drawn 4 times)
        b.setTransform(rotateZ(k * 2 * PI / 5) * rotateY(0.35f));
        b.box(Vec3(-0.09f, 0.1f, -0.01f), Vec3(0.09f, 0.58f, 0.01f));
    }
    b.resetTransform();
    b.sphere(0.12f, 12, 6);
    fanBlades = b.build();

    // conservator (oil expansion tank) on two legs
    b.clear();
    b.setTransform(translate(Vec3(X0 + 0.5f, TOP_Y + 2.2f, Z0 + 1.2f)) * rotateZ(-PI * 0.5f));
    b.cylinder(0.75f, 0.75f, 5.5f, 32, true, true);
    b.resetTransform();
    b.box(Vec3(X0 + 1.2f, TOP_Y + 0.3f, Z0 + 1.0f), Vec3(X0 + 1.4f, TOP_Y + 1.6f, Z0 + 1.4f));
    b.box(Vec3(X0 + 5.0f, TOP_Y + 0.3f, Z0 + 1.0f), Vec3(X0 + 5.2f, TOP_Y + 1.6f, Z0 + 1.4f));
    conservator = b.build();

    // ---- bushings: short LV ones (generator side), tall HV ones (grid side) --
    b.clear();
    MeshBuilder hv, cp;
    for (int p = 0; p < 3; p++) {
        float z = phaseZ(p);
        for (int d = 0; d < 4; d++) {
            b.setTransform(translate(Vec3(X0 + 0.8f, TOP_Y + 0.3f + d * 0.28f, z)));
            b.cylinder(0.32f, 0.2f, 0.26f, 16, true, true);
        }
        for (int d = 0; d < 14; d++) {
            hv.setTransform(translate(Vec3(85.6f, TOP_Y + 0.3f + d * 0.27f, z)));
            hv.cylinder(0.38f - (d % 2) * 0.1f, 0.24f, 0.25f, 16, true, true);
        }
        cp.setTransform(translate(Vec3(85.6f, TOP_Y + 4.0f, z)));
        cp.cylinder(0.18f, 0.18f, 0.35f, 12, true, true);
        cp.setTransform(translate(Vec3(X0 + 0.8f, TOP_Y + 1.4f, z)));
        cp.cylinder(0.14f, 0.14f, 0.3f, 12, true, true);
    }
    b.resetTransform(); hv.resetTransform(); cp.resetTransform();
    lvBushings = b.build();
    hvBushings = hv.build();
    caps = cp.build();

    // ---- three isolated-phase bus ducts from the generator terminals ----------
    // path: out of the terminal (+x) -> through the back wall -> drop onto the LV bushing
    b.clear();
    MeshBuilder fl, st;
    ductCurrent.begin();
    for (int p = 0; p < 3; p++) {
        Vec3 t = terminals[p];
        Vec3 lv(X0 + 0.8f, TOP_Y + 1.6f, phaseZ(p));
        std::vector<Vec3> raw = {t, Vec3(76.5f, t.y, t.z), Vec3(76.5f, t.y, lv.z), Vec3(lv.x, t.y, lv.z), lv};
        std::vector<Vec3> corners;                                  // drop repeated corners
        for (const Vec3& c : raw) if (corners.empty() || length(c - corners.back()) > 0.05f) corners.push_back(c);
        std::vector<Vec3> path = roundedPath(corners, 1.0f, 0.4f);
        b.sweep(path, 0.38f, 20, 0, 2 * PI, false);
        std::vector<PathFrame> f = buildPathFrames(path);
        for (size_t i = 6; i + 3 < f.size(); i += 6) {               // flanges along the duct
            fl.setTransform(alignY(f[i].pos - f[i].tangent * 0.1f, f[i].tangent));
            fl.cylinder(0.5f, 0.5f, 0.2f, 20, true, true);
        }
        ductCurrent.add(path, 0.08f);
    }
    fl.resetTransform();
    ductCurrent.finish();
    for (int k = 0; k < 2; k++) st.box(Vec3(75.8f + k * 1.4f, BASE_Y, 61.0f), Vec3(76.0f + k * 1.4f, 16.2f, 67.6f));
    ducts = b.build();
    ductFlanges = fl.build();
    ductStands = st.build();

    // ---- inside the tank (shown by the x-ray view) --------------------------
    b.clear();
    MeshBuilder lw, hw;
    for (int p = 0; p < 3; p++) {
        float z = phaseZ(p);
        b.box(Vec3(83.0f - 0.35f, BASE_Y + 0.8f, z - 0.35f), Vec3(83.0f + 0.35f, TOP_Y - 0.6f, z + 0.35f));   // leg
        lw.setTransform(translate(Vec3(83.0f, BASE_Y + 1.3f, z)));
        lw.cylinder(0.62f, 0.62f, 3.8f, 24, true, true);
        for (int t = 0; t < 14; t++) {                                       // many HV turns
            hw.setTransform(translate(Vec3(83.0f, BASE_Y + 1.35f + t * 0.27f, z)));
            hw.torus(0.98f, 0.1f, 24, 6);
        }
    }
    b.box(Vec3(82.65f, TOP_Y - 0.9f, Z0 + 0.6f), Vec3(83.35f, TOP_Y - 0.5f, Z1 - 0.6f));                     // yokes
    b.box(Vec3(82.65f, BASE_Y + 0.6f, Z0 + 0.6f), Vec3(83.35f, BASE_Y + 1.0f, Z1 - 0.6f));
    lw.resetTransform(); hw.resetTransform();
    core = b.build();
    lvWindings = lw.build();
    hvWindings = hw.build();

    // HV current leaving the bushings towards the switchyard gantry
    hvCurrent.begin();
    for (int p = 0; p < 3; p++) {
        Vec3 top = hvTop(p);
        std::vector<Vec3> path;
        Vec3 g(93.0f, 16.0f, 59.0f + p * 3.0f);
        for (int i = 0; i <= 16; i++) {
            float u = i / 16.0f;
            path.push_back(mix(top, g, u) - Vec3(0, 1.2f * 4 * u * (1 - u), 0));
        }
        hvCurrent.add(path, 0.05f);
    }
    hvCurrent.finish();
}

void Transformer::draw(Renderer& r, float fanAngle, float xray) {
    Mat4 I;
    Material tankMat = Mat::paint(Vec3(0.38f, 0.46f, 0.42f), 0.45f);      // grey-green like the video
    if (xray > 0.01f && !r.inShadowPass()) {
        // x-ray: the tank becomes a see-through hologram, windings visible inside
        Material holo;
        holo.color = Vec3(0.05f, 0.12f, 0.12f);
        holo.pattern = PAT_HOLO;
        holo.patternScale = 1.2f;
        holo.emissive = Vec3(0.3f, 0.9f, 1.4f) * xray;
        holo.alpha = 0.55f;
        holo.twoSided = true;
        holo.castShadow = false;
        r.drawMesh(core, I, Mat::steel(Vec3(0.3f, 0.32f, 0.35f)));
        r.drawMesh(lvWindings, I, Mat::copper());
        r.drawMesh(hvWindings, I, Mat::copper());
        r.drawMesh(tank, I, holo);
    } else {
        r.drawMesh(tank, I, tankMat);
    }
    r.drawMesh(tankCover, I, tankMat);
    r.drawMesh(radiators, I, tankMat);
    r.drawMesh(fanFrames, I, Mat::paint(Vec3(0.3f, 0.32f, 0.34f)));
    for (int i = 0; i < 4; i++)                                         // TRANSFORMATION: fans spin
        r.drawMesh(fanBlades, translate(fanPos[i]) * rotateZ(fanAngle + i), Mat::paint(Vec3(0.75f, 0.77f, 0.8f)));
    r.drawMesh(conservator, I, tankMat);
    Material porcelain = Mat::plain(Vec3(0.4f, 0.19f, 0.11f), 0.8f, 80.0f);   // brown glazed porcelain
    r.drawMesh(lvBushings, I, porcelain);
    r.drawMesh(hvBushings, I, porcelain);
    r.drawMesh(caps, I, Mat::steel());
    r.drawMesh(ducts, I, Mat::steel(Vec3(0.66f, 0.68f, 0.7f)));
    r.drawMesh(ductFlanges, I, Mat::steel(Vec3(0.5f, 0.52f, 0.55f)));
    r.drawMesh(ductStands, I, Mat::steel(Vec3(0.4f, 0.42f, 0.45f)));
}

void Transformer::drawGlow(Renderer& r, float lv, float hv, float freqVisual, float xray) {
    ductCurrent.draw(r, Mat4(), Vec3(2.4f, 1.0f, 0.22f), lv, 3.0f + 6.0f * freqVisual, 1.4f);
    hvCurrent.draw(r, Mat4(), Vec3(0.6f, 0.9f, 2.4f), hv, 3.0f + 6.0f * freqVisual, 1.4f);
    (void)xray;
}
