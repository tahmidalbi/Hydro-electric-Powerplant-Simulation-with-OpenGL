#include "IntakeStructure.h"
#include "Dam.h"
#include "Rocks.h"
#include "Materials.h"

using namespace IntakeDims;
using namespace DamDims;

void IntakeStructure::build() {
    MeshBuilder b;
    const float zB = UPSTREAM_Z;               // the structure touches the dam face

    // ---- far-side (always visible) concrete ---------------------------------
    b.box(Vec3(CUT_X, 14.0f, FRONT_Z), Vec3(70.0f, FLOOR_Y, zB));                 // foundation
    foundation = b.build();
    b.clear();
    b.box(Vec3(CAGE_X1, FLOOR_Y, FRONT_Z), Vec3(70.0f, TOP_Y, zB));               // far pier
    piersFar = b.build();
    b.clear();
    b.box(Vec3(CUT_X, TOP_Y, FRONT_Z), Vec3(70.0f, CREST_Y, zB));                 // rake deck
    for (float x = CUT_X + 0.5f; x < 70.0f; x += 1.6f)                            // deck railing
        b.box(Vec3(x, CREST_Y, FRONT_Z + 0.1f), Vec3(x + 0.07f, CREST_Y + 1.1f, FRONT_Z + 0.17f));
    b.box(Vec3(CUT_X, CREST_Y + 1.05f, FRONT_Z + 0.08f), Vec3(70.0f, CREST_Y + 1.15f, FRONT_Z + 0.2f));
    deckFar = b.build();

    // ---- near-side parts (hidden in the cut-away) ---------------------------
    b.clear();
    b.box(Vec3(46.0f, 14.0f, FRONT_Z), Vec3(CUT_X, FLOOR_Y, zB));
    b.box(Vec3(46.0f, FLOOR_Y, FRONT_Z), Vec3(CAGE_X0 - 0.1f, TOP_Y, zB));
    b.box(Vec3(46.0f, TOP_Y, FRONT_Z), Vec3(CUT_X, CREST_Y, zB));
    for (float x = 46.5f; x < CUT_X; x += 1.6f)
        b.box(Vec3(x, CREST_Y, FRONT_Z + 0.1f), Vec3(x + 0.07f, CREST_Y + 1.1f, FRONT_Z + 0.17f));
    b.box(Vec3(46.0f, CREST_Y + 1.05f, FRONT_Z + 0.08f), Vec3(CUT_X, CREST_Y + 1.15f, FRONT_Z + 0.2f));
    nearParts = b.build();

    // tiled floor inside the cage and the conduit (the floor seen in the video)
    b.clear();
    b.box(Vec3(CONDUIT_X0, FLOOR_Y, FRONT_Z), Vec3(CONDUIT_X1, FLOOR_Y + 0.02f, BULKHEAD_Z));
    conduitFloor = b.build();

    // steel frame around the intake mouth on the dam face
    b.clear();
    b.box(Vec3(CONDUIT_X0 - 0.6f, CONDUIT_Y1, UPSTREAM_Z - 0.4f), Vec3(CONDUIT_X1 + 0.6f, CONDUIT_Y1 + 0.8f, UPSTREAM_Z));
    b.box(Vec3(CONDUIT_X0 - 0.6f, FLOOR_Y, UPSTREAM_Z - 0.4f), Vec3(CONDUIT_X0, CONDUIT_Y1, UPSTREAM_Z));
    b.box(Vec3(CONDUIT_X1, FLOOR_Y, UPSTREAM_Z - 0.4f), Vec3(CONDUIT_X1 + 0.6f, CONDUIT_Y1, UPSTREAM_Z));
    mouthFrame = b.build();

    // ---- two blue rake motors on the deck ------------------------------------
    b.clear();
    for (int i = -1; i <= 1; i += 2) {
        float x = INTAKE_X + i * 2.6f;
        b.setTransform(translate(Vec3(x - 0.75f, CREST_Y + 0.9f, -14.5f)) * rotateZ(-PI * 0.5f));
        b.cylinder(0.75f, 0.75f, 1.5f, 24, true, true);
        b.resetTransform();
        b.box(Vec3(x - 0.9f, CREST_Y, -15.5f), Vec3(x + 0.9f, CREST_Y + 0.25f, -13.5f));
        b.box(Vec3(x - 0.4f, CREST_Y, -16.4f), Vec3(x + 0.4f, CREST_Y + 1.1f, -15.5f));
    }
    motors = b.build();
    b.clear();
    for (int i = -1; i <= 1; i += 2) {
        b.setTransform(translate(Vec3(INTAKE_X + i * 2.6f, CREST_Y + 0.1f, -16.0f)));
        b.cylinder(0.12f, 0.12f, 1.4f, 10, true, true);
    }
    motorShafts = b.build();

    // ---- the trash-rack cage ----------------------------------------------
    RackSpec front;                      // upstream screen: bars spread along x
    front.width = CAGE_X1 - CAGE_X0;
    front.y0 = FLOOR_Y;
    front.y1 = TOP_Y;
    front.spacing = 0.8f;
    front.barW = 0.24f;
    front.barD = 0.3f;
    front.tips = false;
    front.brackets = false;
    cage.build(front, translate(Vec3(INTAKE_X, 0, CAGE_FRONT_Z)));
    RackSpec side = front;               // side screens: bars spread along z
    side.width = UPSTREAM_Z - CAGE_FRONT_Z - 0.4f;
    side.spacing = 0.6f;
    side.barW = 0.16f;
    float zc = (UPSTREAM_Z + CAGE_FRONT_Z) * 0.5f;
    cage.build(side, translate(Vec3(CAGE_X0 + 0.1f, 0, zc)) * rotateY(PI * 0.5f));
    cage.build(side, translate(Vec3(CAGE_X1 - 0.1f, 0, zc)) * rotateY(PI * 0.5f));
    cage.finish();

    // ---- boulders on the reservoir bed around the foundation -----------------
    b.clear();
    Rng rr(17);
    for (int i = 0; i < 30; i++) {
        float x = INTAKE_X + rr.range(-28.0f, 28.0f);
        float z = rr.range(-40.0f, -4.0f);
        if (x > 44 && x < 72 && z > FRONT_Z - 1.0f) continue;      // not inside the foundation
        addRock(b, Vec3(x, 15.5f, z), rr.range(1.0f, 3.5f), 500 + i);
    }
    rocks = b.build();
}

void IntakeStructure::draw(Renderer& r, bool showNearSide) {
    Mat4 I;
    Material conc = Mat::concrete(1.0f, Vec3(0.5f, 0.52f, 0.52f));
    r.drawMesh(foundation, I, conc);
    r.drawMesh(piersFar, I, conc);
    r.drawMesh(deckFar, I, conc);
    if (showNearSide) r.drawMesh(nearParts, I, conc);
    Material tiles;
    tiles.color = Vec3(0.42f, 0.43f, 0.44f);
    tiles.pattern = PAT_TILES;
    tiles.patternScale = 0.5f;
    tiles.specular = 0.3f;
    r.drawMesh(conduitFloor, I, tiles);
    r.drawMesh(mouthFrame, I, Mat::paint(Vec3(0.2f, 0.21f, 0.23f), 0.5f));
    r.drawMesh(motors, I, Mat::paint(Vec3(0.05f, 0.22f, 0.65f), 0.7f));
    r.drawMesh(motorShafts, I, Mat::steel());
    cage.draw(r, I);
    Material rock;
    rock.color = Vec3(0.3f, 0.31f, 0.3f);
    rock.pattern = PAT_ROCK;
    rock.patternScale = 0.6f;
    rock.specular = 0.1f;
    r.drawMesh(rocks, I, rock);
}
