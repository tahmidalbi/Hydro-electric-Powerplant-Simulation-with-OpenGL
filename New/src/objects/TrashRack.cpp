#include "TrashRack.h"
#include "Materials.h"

void TrashRack::build(const RackSpec& s, const Mat4& place) {
    barsB.setTransform(place);
    frameB.setTransform(place);
    float half = s.width * 0.5f;

    // vertical bars: positions (k + 0.5) * spacing so the centre line is a gap
    int n = (int)(half / s.spacing);
    for (int k = -n; k < n; k++) {
        float x = (k + 0.5f) * s.spacing;
        barsB.box(Vec3(x - s.barW * 0.5f, s.y0, -s.barD * 0.5f), Vec3(x + s.barW * 0.5f, s.y1, s.barD * 0.5f));
        if (s.tips) {   // pointed bar tops sticking out above the top beam
            barsB.box(Vec3(x - s.barW * 0.5f, s.y1, -0.06f), Vec3(x + s.barW * 0.5f, s.y1 + 0.45f, 0.06f));
            barsB.box(Vec3(x - s.barW * 0.25f, s.y1 + 0.45f, -0.04f), Vec3(x + s.barW * 0.25f, s.y1 + 0.7f, 0.04f));
        }
    }
    if (s.beams) {
        // top + bottom beams and a middle tie
        float bh = 0.55f;
        frameB.box(Vec3(-half - 0.4f, s.y1 - bh, -s.barD * 0.5f - 0.15f), Vec3(half + 0.4f, s.y1, s.barD * 0.5f + 0.15f));
        frameB.box(Vec3(-half - 0.4f, s.y0, -s.barD * 0.5f - 0.15f), Vec3(half + 0.4f, s.y0 + bh, s.barD * 0.5f + 0.15f));
        frameB.box(Vec3(-half, s.y0 + bh + 0.9f, -0.1f), Vec3(half, s.y0 + bh + 1.1f, 0.1f));
        if (s.brackets) {
            for (int side = -1; side <= 1; side += 2) {
                for (int lvl = 0; lvl < 2; lvl++) {
                    float yc = lvl == 0 ? s.y0 + bh * 0.5f : s.y1 - bh * 0.5f;
                    float x0 = side < 0 ? -half - 1.6f : half + 0.4f;
                    frameB.box(Vec3(x0, yc - 0.75f, -0.2f), Vec3(x0 + 1.2f, yc + 0.75f, 0.25f));
                    for (int b = 0; b < 4; b++) {     // bolts
                        float bx = x0 + 0.3f + (b % 2) * 0.6f, by = yc - 0.4f + (b / 2) * 0.8f;
                        boltsB.setTransform(place * translate(Vec3(bx, by, -0.2f)) * rotateX(-PI * 0.5f));
                        boltsB.cylinder(0.09f, 0.09f, 0.18f, 8, false, true);
                    }
                }
                // vertical side post
                float px = side < 0 ? -half - 0.4f : half;
                frameB.box(Vec3(px, s.y0, -0.25f), Vec3(px + 0.4f, s.y1, 0.25f));
            }
            // lifting lug on top (where the crane hooks on)
            frameB.box(Vec3(-0.9f, s.y1, -0.12f), Vec3(-0.7f, s.y1 + 1.0f, 0.12f));
            frameB.box(Vec3(0.7f, s.y1, -0.12f), Vec3(0.9f, s.y1 + 1.0f, 0.12f));
            frameB.box(Vec3(-0.9f, s.y1 + 0.85f, -0.12f), Vec3(0.9f, s.y1 + 1.05f, 0.12f));
        }
    }
}

void TrashRack::finish() {
    bars = barsB.build();
    frame = frameB.build();
    bolts = boltsB.build();
}

void TrashRack::draw(Renderer& r, const Mat4& model) {
    Material steel = Mat::paint(Vec3(0.17f, 0.18f, 0.2f), 0.6f);
    steel.reflect = 0.12f;
    r.drawMesh(bars, model, steel);
    r.drawMesh(frame, model, Mat::paint(Vec3(0.15f, 0.16f, 0.18f), 0.5f));
    r.drawMesh(bolts, model, Mat::steel(Vec3(0.5f, 0.5f, 0.52f)));
}
