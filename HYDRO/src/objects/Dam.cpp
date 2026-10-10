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

// crest furniture (road, centre line, parapet, railing) between x0 and x1
static void crestRoad(MeshBuilder& road, MeshBuilder& lines, MeshBuilder& parapet, MeshBuilder& rail, float x0, float x1) {
    road.box(Vec3(x0, CREST_Y, -0.8f), Vec3(x1, CREST_Y + 0.06f, 6.8f));
    for (float x = std::ceil(x0 / 6.0f) * 6.0f; x + 3.0f <= x1; x += 6.0f)
        lines.box(Vec3(x, CREST_Y + 0.06f, 2.92f), Vec3(x + 3.0f, CREST_Y + 0.07f, 3.08f));
    parapet.box(Vec3(x0, CREST_Y, UPSTREAM_Z), Vec3(x1, CREST_Y + 1.1f, UPSTREAM_Z + 0.45f));
    for (float x = std::ceil(x0 / 2.5f) * 2.5f; x <= x1; x += 2.5f)
        rail.box(Vec3(x - 0.05f, CREST_Y, 7.65f), Vec3(x + 0.05f, CREST_Y + 1.1f, 7.75f));
    rail.box(Vec3(x0, CREST_Y + 1.05f, 7.62f), Vec3(x1, CREST_Y + 1.15f, 7.78f));
    rail.box(Vec3(x0, CREST_Y + 0.55f, 7.66f), Vec3(x1, CREST_Y + 0.62f, 7.74f));
}

static void lampPost(MeshBuilder& poles, MeshBuilder& heads, float x) {
    poles.setTransform(translate(Vec3(x, CREST_Y, 7.3f)));
    poles.cylinder(0.12f, 0.09f, 7.0f, 8, false, true);
    poles.resetTransform();
    poles.box(Vec3(x - 0.06f, CREST_Y + 6.8f, 5.9f), Vec3(x + 0.06f, CREST_Y + 6.95f, 7.3f));
    poles.box(Vec3(x - 0.3f, CREST_Y + 6.6f, 5.6f), Vec3(x + 0.3f, CREST_Y + 6.9f, 6.4f));
    heads.box(Vec3(x - 0.25f, CREST_Y + 6.55f, 5.65f), Vec3(x + 0.25f, CREST_Y + 6.6f, 6.35f));
}

void Dam::build() {
    MeshBuilder b;
    std::vector<Vec2> nonOverflow = {
        Vec2(UPSTREAM_Z, 0), Vec2(UPSTREAM_Z, CREST_Y), Vec2(8.0f, CREST_Y), Vec2(8.0f, 57.0f), Vec2(faceZ(0.0f), 0)};

    // ---- non-overflow body: left section + right section beyond the intake ---
    b.extrudeOutlineX(nonOverflow, DAM_X0, SPILL_X0, -4.0f);
    b.extrudeOutlineX(nonOverflow, CONDUIT_X1, DAM_X1, -4.0f);
    body = b.build();

    // ---- the cut block (x 24.5 .. 52.5): lifted away in the cut-away shots ----
    b.clear();
    b.extrudeOutlineX(nonOverflow, SPILL_X1, CUT_X, -4.0f);
    cutBody = b.build();

    // ---- the intake block (x 52.5 .. 63.5) with the conduit and gate shaft ---
    // It is made of four solid pieces; the empty space between them IS the
    // conduit (y 34..44, z -2..16) and the gate shaft (z 0.4..2.6, above it).
    b.clear();
    {
        std::vector<Vec2> below = {Vec2(UPSTREAM_Z, 0), Vec2(UPSTREAM_Z, CONDUIT_Y0), Vec2(faceZ(CONDUIT_Y0), CONDUIT_Y0), Vec2(faceZ(0), 0)};
        b.extrudeOutlineX(below, CONDUIT_X0, CONDUIT_X1, -4.0f);
        b.box(Vec3(CONDUIT_X0, CONDUIT_Y1, UPSTREAM_Z), Vec3(CONDUIT_X1, CREST_Y, SHAFT_Z0));      // in front of the shaft
        std::vector<Vec2> behind = {Vec2(SHAFT_Z1, CONDUIT_Y1), Vec2(SHAFT_Z1, CREST_Y), Vec2(8.0f, CREST_Y),
                                    Vec2(8.0f, 57.0f), Vec2(faceZ(CONDUIT_Y1), CONDUIT_Y1)};
        b.extrudeOutlineX(behind, CONDUIT_X0, CONDUIT_X1, CONDUIT_Y1);
        std::vector<Vec2> plug = {Vec2(BULKHEAD_Z, CONDUIT_Y0), Vec2(BULKHEAD_Z, CONDUIT_Y1),
                                  Vec2(faceZ(CONDUIT_Y1), CONDUIT_Y1), Vec2(faceZ(CONDUIT_Y0), CONDUIT_Y0)};
        b.extrudeOutlineX(plug, CONDUIT_X0, CONDUIT_X1, CONDUIT_Y0);
    }
    intakeBlock = b.build();

    // vertical ribs on the downstream face (contraction joints) - give scale
    b.clear();
    std::vector<Vec2> rib;
    for (Vec2 v : nonOverflow) rib.push_back(Vec2(v.x + (v.x > 7.0f ? 0.6f : 0.0f), v.y));
    for (float x = DAM_X0 + 12.0f; x < DAM_X1 - 6.0f; x += 18.0f) {
        if (x > SPILL_X0 - 3.0f && x < CONDUIT_X1 + 1.0f) continue;
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

    // ---- bridge deck over the spillway gates --------------------------------
    b.clear();
    b.box(Vec3(SPILL_X0, 61.6f, 2.0f), Vec3(SPILL_X1, CREST_Y, 8.0f));
    for (int i = 0; i < BAYS; i++)
        for (int k = 0; k < 3; k++) {
            float z = 2.6f + k * 2.4f;
            b.box(Vec3(bayX0(i), 60.4f, z - 0.3f), Vec3(bayX0(i) + BAY_W, 61.6f, z + 0.3f));
        }
    deck = b.build();

    // ---- crest road / parapet / railing: fixed parts and cut-block parts ----
    {
        MeshBuilder rd, ln, pp, rl;
        crestRoad(rd, ln, pp, rl, DAM_X0, SPILL_X1);
        crestRoad(rd, ln, pp, rl, CUT_X, DAM_X1);
        road = rd.build(); roadLines = ln.build(); parapet = pp.build(); railing = rl.build();
        MeshBuilder crd, cln, cpp, crl;
        crestRoad(crd, cln, cpp, crl, SPILL_X1, CUT_X);
        cutRoad = crd.build(); cutLines = cln.build(); cutParapet = cpp.build(); cutRailing = crl.build();
    }

    // ---- spillway gate hoist frames on the piers + hoist houses -------------
    b.clear();
    for (int i = 0; i <= BAYS; i++) {
        float xc = SPILL_X0 + i * (BAY_W + PIER_W) + PIER_W * 0.5f;
        b.box(Vec3(xc - 0.7f, CREST_Y, -3.2f), Vec3(xc + 0.7f, 73.0f, -1.8f));
        b.box(Vec3(xc - 0.7f, CREST_Y, 0.2f), Vec3(xc + 0.7f, 73.0f, 1.6f));
        b.beam(Vec3(xc, CREST_Y + 1.0f, -2.5f), Vec3(xc, 72.0f, 0.9f), 0.35f, 0.35f);
    }
    b.box(Vec3(SPILL_X0, 73.0f, -3.4f), Vec3(SPILL_X1, 74.2f, 1.8f));
    for (float x = SPILL_X0; x <= SPILL_X1; x += 1.6f)
        b.box(Vec3(x - 0.04f, 74.2f, 1.7f), Vec3(x + 0.04f, 75.2f, 1.78f));
    b.box(Vec3(SPILL_X0, 75.1f, 1.68f), Vec3(SPILL_X1, 75.2f, 1.8f));
    hoistFrame = b.build();

    b.clear();
    for (int i = 0; i < BAYS; i++) {
        float xc = bayX0(i) + BAY_W * 0.5f;
        b.box(Vec3(xc - 2.2f, 74.2f, -2.8f), Vec3(xc + 2.2f, 76.2f, 0.4f));
        b.box(Vec3(xc - 3.6f, 74.2f, -1.6f), Vec3(xc - 2.2f, 75.4f, -0.6f));
        b.box(Vec3(xc + 2.2f, 74.2f, -1.6f), Vec3(xc + 3.6f, 75.4f, -0.6f));
    }
    hoistHouses = b.build();

    // ---- one spillway lift gate (drawn three times) ------------------------
    b.clear();
    b.box(Vec3(0.15f, 0.0f, -1.05f), Vec3(BAY_W - 0.15f, 7.2f, -0.65f));
    for (int k = 0; k < 5; k++) {
        float y = 0.5f + k * 1.6f;
        b.box(Vec3(0.3f, y, -0.65f), Vec3(BAY_W - 0.3f, y + 0.45f, 0.25f));
    }
    for (int k = 0; k <= 6; k++) {
        float x = 0.4f + k * (BAY_W - 0.8f) / 6.0f;
        b.box(Vec3(x - 0.08f, 0.1f, -0.65f), Vec3(x + 0.08f, 7.1f, 0.1f));
    }
    b.box(Vec3(BAY_W * 0.5f - 0.15f, 7.2f, -0.95f), Vec3(BAY_W * 0.5f + 0.15f, 16.0f, -0.75f));
    gate = b.build();

    // ---- yellow gantry crane over the power intake --------------------------
    b.clear();
    const float cx = INTAKE_X + 1.0f;
    for (int side = -1; side <= 1; side += 2) {
        float x = cx + side * 6.0f;
        b.beam(Vec3(x, CREST_Y, -1.6f), Vec3(x, CREST_Y + 13.0f, 1.2f), 0.7f, 0.7f);
        b.beam(Vec3(x, CREST_Y, 7.4f), Vec3(x, CREST_Y + 13.0f, 4.6f), 0.7f, 0.7f);
        b.box(Vec3(x - 0.5f, CREST_Y + 4.5f, -0.8f), Vec3(x + 0.5f, CREST_Y + 5.2f, 6.6f));
        b.box(Vec3(x - 0.6f, CREST_Y, -2.0f), Vec3(x + 0.6f, CREST_Y + 0.9f, 7.8f));
    }
    b.box(Vec3(cx - 7.5f, CREST_Y + 13.0f, 0.6f), Vec3(cx + 7.5f, CREST_Y + 14.6f, 1.6f));
    b.box(Vec3(cx - 7.5f, CREST_Y + 13.0f, 4.2f), Vec3(cx + 7.5f, CREST_Y + 14.6f, 5.2f));
    b.box(Vec3(cx - 2.0f, CREST_Y + 14.6f, 0.3f), Vec3(cx + 2.0f, CREST_Y + 16.6f, 5.5f));
    craneMesh = b.build();
    b.clear();
    b.box(Vec3(CUT_X, CREST_Y + 0.06f, -1.5f), Vec3(90.0f, CREST_Y + 0.2f, -1.2f));
    b.box(Vec3(CUT_X, CREST_Y + 0.06f, 7.2f), Vec3(90.0f, CREST_Y + 0.2f, 7.5f));
    craneRails = b.build();
    b.clear();
    b.box(Vec3(30.0f, CREST_Y + 0.06f, -1.5f), Vec3(CUT_X, CREST_Y + 0.2f, -1.2f));
    b.box(Vec3(30.0f, CREST_Y + 0.06f, 7.2f), Vec3(CUT_X, CREST_Y + 0.2f, 7.5f));
    cutRails = b.build();

    // ---- lamp posts along the crest ----------------------------------------
    MeshBuilder poles, heads, cPoles, cHeads;
    for (float x = DAM_X0 + 10.0f; x < DAM_X1; x += 20.0f) {
        if (x > SPILL_X0 - 2.0f && x < SPILL_X1 + 2.0f) continue;
        if (x > SPILL_X1 && x < CUT_X) lampPost(cPoles, cHeads, x);
        else lampPost(poles, heads, x);
    }
    lampPoles = poles.build();
    lampHeads = heads.build();
    cutLampPoles = cPoles.build();
    cutLampHeads = cHeads.build();
}

void Dam::draw(Renderer& r, float gateOpen, float cutLift) {
    Mat4 I;
    Material conc = Mat::concrete(0.5f);
    r.drawMesh(body, I, conc);
    r.drawMesh(intakeBlock, I, conc);
    r.drawMesh(ribs, I, conc);
    r.drawMesh(spillBody, I, Mat::concrete(0.5f, Vec3(0.53f, 0.53f, 0.51f)));
    r.drawMesh(piers, I, Mat::concrete(0.5f, Vec3(0.55f, 0.55f, 0.53f)));
    r.drawMesh(deck, I, Mat::concrete(1.0f));
    r.drawMesh(road, I, Mat::plain(Vec3(0.12f, 0.12f, 0.13f), 0.25f, 30.0f));
    r.drawMesh(roadLines, I, Mat::plain(Vec3(0.8f, 0.8f, 0.75f)));
    r.drawMesh(parapet, I, Mat::concrete(1.0f, Vec3(0.56f, 0.56f, 0.54f)));
    r.drawMesh(railing, I, Mat::steel(Vec3(0.75f, 0.75f, 0.72f)));
    r.drawMesh(hoistFrame, I, Mat::concrete(1.0f, Vec3(0.53f, 0.53f, 0.51f)));
    r.drawMesh(hoistHouses, I, Mat::paint(Vec3(0.08f, 0.22f, 0.55f)));

    // TRANSFORMATION: each spillway gate is the same mesh, translated to its bay
    // and lifted by gateOpen * GATE_LIFT metres.
    for (int i = 0; i < BAYS; i++) {
        Mat4 m = translate(Vec3(bayX0(i), GATE_BOTTOM_Y + gateOpen * GATE_LIFT, GATE_Z + 0.8f));
        r.drawMesh(gate, m, Mat::paint(Vec3(0.2f, 0.24f, 0.27f), 0.5f));
    }
    r.drawMesh(craneMesh, I, Mat::paint(Vec3(0.85f, 0.62f, 0.05f), 0.5f));
    r.drawMesh(craneRails, I, Mat::darkSteel());
    r.drawMesh(lampPoles, I, Mat::steel(Vec3(0.5f, 0.5f, 0.5f)));
    r.drawMesh(lampHeads, I, Mat::glow(Vec3(6.0f, 5.0f, 3.6f)));

    // the cut block rises out of the way (an "exploded" cut-away animation)
    if (cutLift < 0.999f) {
        float e = cutLift * cutLift * (3.0f - 2.0f * cutLift);
        Mat4 lift = translate(Vec3(0, e * 160.0f, 0));
        r.drawMesh(cutBody, lift, conc);
        r.drawMesh(cutRoad, lift, Mat::plain(Vec3(0.12f, 0.12f, 0.13f), 0.25f, 30.0f));
        r.drawMesh(cutLines, lift, Mat::plain(Vec3(0.8f, 0.8f, 0.75f)));
        r.drawMesh(cutParapet, lift, Mat::concrete(1.0f, Vec3(0.56f, 0.56f, 0.54f)));
        r.drawMesh(cutRailing, lift, Mat::steel(Vec3(0.75f, 0.75f, 0.72f)));
        r.drawMesh(cutRails, lift, Mat::darkSteel());
        r.drawMesh(cutLampPoles, lift, Mat::steel(Vec3(0.5f, 0.5f, 0.5f)));
        r.drawMesh(cutLampHeads, lift, Mat::glow(Vec3(6.0f, 5.0f, 3.6f)));
    }
}
