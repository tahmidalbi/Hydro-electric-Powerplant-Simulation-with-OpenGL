#include "Dam.h"
#include "Materials.h"

using namespace DamDims;

// ----------------------------------------------------------------------------
//  The spillway floor: approach -> rounded rise -> OGEE crest -> straight
//  53 degree chute -> circular BUCKET that turns the water horizontal -> apron.
// ----------------------------------------------------------------------------
std::vector<Vec2> DamDims::chuteProfile() {
    std::vector<Vec2> p;
    p.push_back(Vec2(-10.0f, 44.0f));
    p.push_back(Vec2(-6.0f, 44.0f));
    for (int i = 1; i <= 14; i++) {                       // smooth rise to the crest
        float u = i / 14.0f;
        p.push_back(Vec2(-6.0f + 7.0f * u, 44.0f + 8.0f * (0.5f - 0.5f * std::cos(PI * u))));
    }
    for (int i = 1; i <= 16; i++) {                       // ogee: y = 52 - 2 (x/4)^1.85
        float z = 1.0f + 6.15f * i / 16.0f;
        p.push_back(Vec2(z, 52.0f - 2.0f * std::pow((z - 1.0f) / 4.0f, 1.85f)));
    }
    float zs = 7.15f, ys = 52.0f - 2.0f * std::pow(6.15f / 4.0f, 1.85f);
    float yEnd = 10.0f, zEnd = zs + (ys - yEnd) / 1.3333f;
    for (int i = 1; i <= 20; i++) {                       // straight chute
        float u = i / 20.0f;
        p.push_back(Vec2(zs + (zEnd - zs) * u, ys + (yEnd - ys) * u));
    }
    float R = 14.0f, th = std::atan(1.3333f);
    Vec2 C(zEnd + R * std::sin(th), yEnd + R * std::cos(th));
    for (int i = 1; i <= 16; i++) {                       // bucket (arc)
        float a = th * (1.0f - i / 16.0f);
        p.push_back(Vec2(C.x - R * std::sin(a), C.y - R * std::cos(a)));
    }
    p.push_back(Vec2(54.0f, p.back().y));                 // apron
    return p;
}

float DamDims::chuteY(float z) {
    static std::vector<Vec2> p = chuteProfile();
    if (z <= p.front().x) return p.front().y;
    for (size_t i = 1; i < p.size(); i++)
        if (z <= p[i].x) {
            float u = (z - p[i - 1].x) / (p[i].x - p[i - 1].x);
            return mixf(p[i - 1].y, p[i].y, u);
        }
    return p.back().y;
}

void Dam::build() {
    MeshBuilder b;

    // ---- non-overflow dam body (left and right of the spillway) -------------
    std::vector<Vec2> nonOverflow = {
        Vec2(UPSTREAM_Z, 0), Vec2(UPSTREAM_Z, CREST_Y), Vec2(8.0f, CREST_Y), Vec2(8.0f, 57.0f), Vec2(faceZ(0.0f), 0)};
    b.extrudeOutlineX(nonOverflow, DAM_X0, SPILL_X0, -4.0f);
    b.extrudeOutlineX(nonOverflow, SPILL_X1, DAM_X1, -4.0f);
    body = b.build();

    // vertical ribs on the downstream face (contraction joints) - give scale
    b.clear();
    std::vector<Vec2> rib;
    for (Vec2 v : nonOverflow) rib.push_back(Vec2(v.x + (v.x > 7.0f ? 0.6f : 0.0f), v.y));
    for (float x = DAM_X0 + 12.0f; x < DAM_X1 - 6.0f; x += 18.0f) {
        if (x > SPILL_X0 - 3.0f && x < SPILL_X1 + 1.0f) continue;
        b.extrudeOutlineX(rib, x, x + 1.4f, -4.0f);
    }
    ribs = b.build();

    // ---- spillway body under the chute --------------------------------------
    b.clear();
    std::vector<Vec2> chute = chuteProfile();
    std::vector<Vec2> spill;
    spill.push_back(Vec2(-10.0f, 0.0f));
    for (Vec2 v : chute) spill.push_back(v);
    spill.push_back(Vec2(54.0f, 0.0f));
    b.extrudeOutlineX(spill, SPILL_X0, SPILL_X1, -4.0f);
    spillBody = b.build();

    // ---- piers / training walls between the bays ----------------------------
    b.clear();
    std::vector<Vec2> pier;
    pier.push_back(Vec2(-12.0f, 0.0f));
    pier.push_back(Vec2(-12.0f, CREST_Y));
    pier.push_back(Vec2(9.0f, CREST_Y));
    for (Vec2 v : chute)
        if (v.x >= 15.0f) pier.push_back(Vec2(v.x, v.y + 4.5f));
    pier.push_back(Vec2(54.0f, 0.0f));
    for (int i = 0; i <= BAYS; i++) {
        float x0 = SPILL_X0 + i * (BAY_W + PIER_W);
        b.extrudeOutlineX(pier, x0, x0 + PIER_W, -4.0f);
    }
    piers = b.build();

    // ---- bridge deck over the gates + crest road -----------------------------
    b.clear();
    b.box(Vec3(SPILL_X0, 61.6f, 2.0f), Vec3(SPILL_X1, CREST_Y, 8.0f));
    for (int i = 0; i < BAYS; i++)                               // deck girders
        for (int k = 0; k < 3; k++) {
            float z = 2.6f + k * 2.4f;
            b.box(Vec3(bayX0(i), 60.4f, z - 0.3f), Vec3(bayX0(i) + BAY_W, 61.6f, z + 0.3f));
        }
    deck = b.build();

    b.clear();
    b.box(Vec3(DAM_X0, CREST_Y, -0.8f), Vec3(DAM_X1, CREST_Y + 0.06f, 6.8f));
    road = b.build();
    b.clear();
    for (float x = DAM_X0; x < DAM_X1; x += 6.0f) b.box(Vec3(x, CREST_Y + 0.06f, 2.92f), Vec3(x + 3.0f, CREST_Y + 0.07f, 3.08f));
    roadLines = b.build();

    // upstream parapet wall
    b.clear();
    b.box(Vec3(DAM_X0, CREST_Y, UPSTREAM_Z), Vec3(DAM_X1, CREST_Y + 1.1f, UPSTREAM_Z + 0.45f));
    parapet = b.build();

    // downstream railing: posts + two rails
    b.clear();
    for (float x = DAM_X0; x <= DAM_X1; x += 2.5f) b.box(Vec3(x - 0.05f, CREST_Y, 7.65f), Vec3(x + 0.05f, CREST_Y + 1.1f, 7.75f));
    b.box(Vec3(DAM_X0, CREST_Y + 1.05f, 7.62f), Vec3(DAM_X1, CREST_Y + 1.15f, 7.78f));
    b.box(Vec3(DAM_X0, CREST_Y + 0.55f, 7.66f), Vec3(DAM_X1, CREST_Y + 0.62f, 7.74f));
    railing = b.build();

    // ---- gate hoist frames on the piers + hoist houses ----------------------
    b.clear();
    for (int i = 0; i <= BAYS; i++) {
        float xc = SPILL_X0 + i * (BAY_W + PIER_W) + PIER_W * 0.5f;
        b.box(Vec3(xc - 0.7f, CREST_Y, -3.2f), Vec3(xc + 0.7f, 73.0f, -1.8f));
        b.box(Vec3(xc - 0.7f, CREST_Y, 0.2f), Vec3(xc + 0.7f, 73.0f, 1.6f));
        b.beam(Vec3(xc, CREST_Y + 1.0f, -2.5f), Vec3(xc, 72.0f, 0.9f), 0.35f, 0.35f);
    }
    b.box(Vec3(SPILL_X0, 73.0f, -3.4f), Vec3(SPILL_X1, 74.2f, 1.8f));      // top platform
    for (float x = SPILL_X0; x <= SPILL_X1; x += 1.6f)                      // platform railing posts
        b.box(Vec3(x - 0.04f, 74.2f, 1.7f), Vec3(x + 0.04f, 75.2f, 1.78f));
    b.box(Vec3(SPILL_X0, 75.1f, 1.68f), Vec3(SPILL_X1, 75.2f, 1.8f));
    hoistFrame = b.build();

    b.clear();
    for (int i = 0; i < BAYS; i++) {
        float xc = bayX0(i) + BAY_W * 0.5f;
        b.box(Vec3(xc - 2.2f, 74.2f, -2.8f), Vec3(xc + 2.2f, 76.2f, 0.4f));   // hoist motor housing
        b.box(Vec3(xc - 3.6f, 74.2f, -1.6f), Vec3(xc - 2.2f, 75.4f, -0.6f));  // drums
        b.box(Vec3(xc + 2.2f, 74.2f, -1.6f), Vec3(xc + 3.6f, 75.4f, -0.6f));
    }
    hoistHouses = b.build();

    // ---- one vertical lift gate (drawn three times) ------------------------
    b.clear();
    b.box(Vec3(0.15f, 0.0f, -1.05f), Vec3(BAY_W - 0.15f, 7.2f, -0.65f));             // skin plate
    for (int k = 0; k < 5; k++) {                                                    // horizontal girders
        float y = 0.5f + k * 1.6f;
        b.box(Vec3(0.3f, y, -0.65f), Vec3(BAY_W - 0.3f, y + 0.45f, 0.25f));
    }
    for (int k = 0; k <= 6; k++) {                                                   // vertical stiffeners
        float x = 0.4f + k * (BAY_W - 0.8f) / 6.0f;
        b.box(Vec3(x - 0.08f, 0.1f, -0.65f), Vec3(x + 0.08f, 7.1f, 0.1f));
    }
    b.box(Vec3(BAY_W * 0.5f - 0.15f, 7.2f, -0.95f), Vec3(BAY_W * 0.5f + 0.15f, 16.0f, -0.75f)); // lifting rod
    gate = b.build();

    // ---- yellow gantry crane over the power intake (x = 58) ---------------
    b.clear();
    const float cx = 58.0f;
    for (int side = -1; side <= 1; side += 2) {
        float x = cx + side * 6.0f;
        b.beam(Vec3(x, CREST_Y, -1.6f), Vec3(x, CREST_Y + 13.0f, 1.2f), 0.7f, 0.7f);
        b.beam(Vec3(x, CREST_Y, 7.4f), Vec3(x, CREST_Y + 13.0f, 4.6f), 0.7f, 0.7f);
        b.box(Vec3(x - 0.5f, CREST_Y + 4.5f, -0.8f), Vec3(x + 0.5f, CREST_Y + 5.2f, 6.6f));   // cross tie
        b.box(Vec3(x - 0.6f, CREST_Y, -2.0f), Vec3(x + 0.6f, CREST_Y + 0.9f, 7.8f));          // bogie beam
    }
    b.box(Vec3(cx - 7.5f, CREST_Y + 13.0f, 0.6f), Vec3(cx + 7.5f, CREST_Y + 14.6f, 1.6f));     // main girders
    b.box(Vec3(cx - 7.5f, CREST_Y + 13.0f, 4.2f), Vec3(cx + 7.5f, CREST_Y + 14.6f, 5.2f));
    b.box(Vec3(cx - 2.0f, CREST_Y + 14.6f, 0.3f), Vec3(cx + 2.0f, CREST_Y + 16.6f, 5.5f));     // trolley
    craneMesh = b.build();
    b.clear();
    b.box(Vec3(30.0f, CREST_Y + 0.06f, -1.5f), Vec3(90.0f, CREST_Y + 0.2f, -1.2f));
    b.box(Vec3(30.0f, CREST_Y + 0.06f, 7.2f), Vec3(90.0f, CREST_Y + 0.2f, 7.5f));
    craneRails = b.build();

    // ---- lamp posts along the crest ----------------------------------------
    b.clear();
    MeshBuilder heads;
    for (float x = DAM_X0 + 10.0f; x < DAM_X1; x += 20.0f) {
        if (x > SPILL_X0 - 2.0f && x < SPILL_X1 + 2.0f) continue;
        b.setTransform(translate(Vec3(x, CREST_Y, 7.3f)));
        b.cylinder(0.12f, 0.09f, 7.0f, 8, false, true);
        b.resetTransform();
        b.box(Vec3(x - 0.06f, CREST_Y + 6.8f, 5.9f), Vec3(x + 0.06f, CREST_Y + 6.95f, 7.3f));
        b.box(Vec3(x - 0.3f, CREST_Y + 6.6f, 5.6f), Vec3(x + 0.3f, CREST_Y + 6.9f, 6.4f));
        heads.box(Vec3(x - 0.25f, CREST_Y + 6.55f, 5.65f), Vec3(x + 0.25f, CREST_Y + 6.6f, 6.35f));
    }
    lampPoles = b.build();
    lampHeads = heads.build();
}

void Dam::draw(Renderer& r, float gateOpen) {
    Mat4 I;
    Material conc = Mat::concrete(0.5f);
    conc.gouraud = false;
    r.drawMesh(body, I, conc);
    r.drawMesh(ribs, I, conc);
    r.drawMesh(spillBody, I, Mat::concrete(0.5f, Vec3(0.66f, 0.66f, 0.64f)));
    r.drawMesh(piers, I, Mat::concrete(0.5f, Vec3(0.68f, 0.68f, 0.66f)));
    r.drawMesh(deck, I, Mat::concrete(1.0f));
    r.drawMesh(road, I, Mat::plain(Vec3(0.12f, 0.12f, 0.13f), 0.25f, 30.0f));
    r.drawMesh(roadLines, I, Mat::plain(Vec3(0.8f, 0.8f, 0.75f)));
    r.drawMesh(parapet, I, Mat::concrete(1.0f, Vec3(0.7f, 0.7f, 0.68f)));
    r.drawMesh(railing, I, Mat::steel(Vec3(0.75f, 0.75f, 0.72f)));
    r.drawMesh(hoistFrame, I, Mat::concrete(1.0f, Vec3(0.66f, 0.66f, 0.64f)));
    r.drawMesh(hoistHouses, I, Mat::paint(Vec3(0.08f, 0.22f, 0.55f)));

    // TRANSFORMATION: each gate is the same mesh, translated to its bay and
    // lifted by gateOpen * GATE_LIFT metres.
    for (int i = 0; i < BAYS; i++) {
        Mat4 m = translate(Vec3(bayX0(i), GATE_BOTTOM_Y + gateOpen * GATE_LIFT, GATE_Z + 0.8f));
        r.drawMesh(gate, m, Mat::paint(Vec3(0.2f, 0.24f, 0.27f), 0.5f));
    }
    r.drawMesh(craneMesh, I, Mat::paint(Vec3(0.85f, 0.62f, 0.05f), 0.5f));
    r.drawMesh(craneRails, I, Mat::darkSteel());
    r.drawMesh(lampPoles, I, Mat::steel(Vec3(0.5f, 0.5f, 0.5f)));
    r.drawMesh(lampHeads, I, Mat::glow(Vec3(6.0f, 5.0f, 3.6f)));
}
