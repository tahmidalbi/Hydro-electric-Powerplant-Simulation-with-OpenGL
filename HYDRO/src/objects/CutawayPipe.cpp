#include "CutawayPipe.h"
#include "Materials.h"

void CutawayPipe::build(const std::vector<Vec3>& corners, float R, float bendRadius,
                        const Vec3& cutDir, float cutHalfDeg, float ringSpacing) {
    radius = R;
    std::vector<Vec3> path = roundedPath(corners, bendRadius, 0.5f);
    frames = buildPathFrames(path);
    length = frames.back().s;

    // angle (in the pipe's up/side frame) that points to the cut side
    Vec3 up0 = frames[0].up, side0 = frames[0].side;
    float ac = std::atan2(dot(cutDir, side0), dot(cutDir, up0));
    float h = radians(cutHalfDeg);
    float keep0 = ac + h, keep1 = ac + 2.0f * PI - h;   // the part that stays

    MeshBuilder b;
    b.sweep(path, R, 48, keep0, keep1, false);          // outside of the steel wall
    b.sweep(path, R - wall, 48, keep0, keep1, true);    // inside of the steel wall
    shell = b.build();

    // cut edges: thin strips joining the outer and inner wall
    b.clear();
    for (int e = 0; e < 2; e++) {
        float a = e == 0 ? keep0 : keep1;
        unsigned int start = (unsigned int)b.verts.size();
        for (const PathFrame& f : frames) {
            Vec3 radial = f.up * std::cos(a) + f.side * std::sin(a);
            Vec3 tangentDir = f.up * -std::sin(a) + f.side * std::cos(a);
            Vec3 n = e == 0 ? -tangentDir : tangentDir;
            b.addVertex(f.pos + radial * R, n, Vec2(0, f.s));
            b.addVertex(f.pos + radial * (R - wall), n, Vec2(wall, f.s));
        }
        for (size_t i = 0; i + 1 < frames.size(); i++) {
            unsigned int k = start + (unsigned int)i * 2;
            b.tri(k, k + 1, k + 3);
            b.tri(k, k + 3, k + 2);
        }
    }
    rims = b.build();

    // stiffener rings (full circle) with bolts
    b.clear();
    MeshBuilder bb;
    for (float s = ringSpacing * 0.5f; s < length - 0.5f; s += ringSpacing) {
        Vec3 p = pointAt(s), t = tangentAt(s);
        b.setTransform(alignY(p - t * 0.22f, t));
        b.cylinder(R * 1.13f, R * 1.13f, 0.44f, 56, false, false);
        b.cylinder(R * 1.0f, R * 1.0f, 0.44f, 56, false, false);
        b.annulus(R * 1.0f, R * 1.13f, 56);
        b.setTransform(alignY(p + t * 0.22f, t));
        b.annulus(R * 1.0f, R * 1.13f, 56);
        for (int k = 0; k < 20; k++) {
            float a = k * 2.0f * PI / 20.0f;
            bb.setTransform(alignY(p - t * 0.32f, t) * translate(Vec3(std::cos(a) * R * 1.07f, 0, std::sin(a) * R * 1.07f)));
            bb.cylinder(0.07f, 0.07f, 0.64f, 6, true, true);
        }
    }
    rings = b.build();
    bolts = bb.build();

    // the removed part, used for the x-ray hologram look
    b.clear();
    b.resetTransform();
    b.sweep(path, R + 0.02f, 40, keep1, keep0 + 2.0f * PI, false);
    holo = b.build();

    // water: a full tube a bit smaller than the pipe
    b.clear();
    b.sweep(path, R * 0.84f, 40, 0.0f, 2.0f * PI, false);
    water = b.build();
}

Vec3 CutawayPipe::pointAt(float s) const {
    if (s <= 0) return frames.front().pos;
    for (size_t i = 1; i < frames.size(); i++)
        if (frames[i].s >= s) {
            float u = (s - frames[i - 1].s) / (frames[i].s - frames[i - 1].s);
            return mix(frames[i - 1].pos, frames[i].pos, u);
        }
    return frames.back().pos;
}

Vec3 CutawayPipe::tangentAt(float s) const {
    for (size_t i = 1; i < frames.size(); i++)
        if (frames[i].s >= s) return frames[i].tangent;
    return frames.back().tangent;
}

void CutawayPipe::drawShell(Renderer& r, const Material& steel, float holoAmount) {
    Mat4 I;
    r.drawMesh(shell, I, steel);
    r.drawMesh(rims, I, Mat::plain(Vec3(0.75f, 0.76f, 0.78f), 0.6f, 50.0f));
    Material ring = steel;
    ring.color = steel.color * 0.85f;
    r.drawMesh(rings, I, ring);
    r.drawMesh(bolts, I, Mat::steel(Vec3(0.35f, 0.36f, 0.38f)));
    (void)holoAmount;
}

void CutawayPipe::drawHolo(Renderer& r, float amount) {
    if (amount <= 0.01f || r.inShadowPass()) return;
    Material m;
    m.color = Vec3(0.05f, 0.15f, 0.25f);
    m.pattern = PAT_HOLO;
    m.patternScale = 0.8f;
    m.emissive = Vec3(0.3f, 0.8f, 1.6f) * amount;
    m.alpha = 0.6f * amount;
    m.twoSided = true;
    m.castShadow = false;
    r.drawMesh(holo, Mat4(), m);
}

void CutawayPipe::drawWater(Renderer& r, WaterStyle w) {
    w.mode = 2;
    w.tubeRadius = radius * 0.84f;
    r.drawWater(water, Mat4(), w);
}
