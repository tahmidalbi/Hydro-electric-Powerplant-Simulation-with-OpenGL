// =============================================================================
//  raytrace.frag  -  a real RAY TRACER running on the GPU (scene 7).
//
//  For every pixel a ray is shot from the camera.  The scene is described
//  with exact maths (no triangles):
//      * a glass penstock pipe   (infinite cylinder, split into glass / steel parts)
//      * steel flanges and bands (capped cylinders)
//      * concrete supports       (boxes)
//      * a glossy floor and a back wall (planes)
//
//  What the ray tracer does (Whitted style):
//      1. primary ray  -> closest hit
//      2. SHADOW ray   -> is the light blocked?  (glass lets some blue light through)
//      3. REFLECTION   -> new ray off the floor / glass / steel
//      4. REFRACTION   -> ray bends into the water (Snell's law, n = 1.33),
//                         travels through it, bends out again
//      5. inside the water the ray MARCHES in small steps and collects the
//         glow of the fast water streaks (volume rendering)
// =============================================================================
#version 330 core
#include "common.glsl"

uniform vec3  uCamRight;
uniform vec3  uCamUp;
uniform vec3  uCamForward;
uniform float uTanHalfFov;
uniform float uAspect;
uniform vec3  uPipeA;        // a point on the pipe axis
uniform vec3  uPipeDir;      // axis direction (flow direction)
uniform float uPipeR;        // pipe radius
uniform float uFlowSpeed;    // how fast the streaks move (m/s, visual)
uniform float uFlowOn;       // 0..1 water present
uniform float uScan;         // position of the scanning ring along the pipe

in vec2 vNdc;
out vec4 FragColor;

#define MAT_NONE    0
#define MAT_FLOOR   1
#define MAT_WALL    2
#define MAT_GLASS   3
#define MAT_STEEL   4
#define MAT_SUPPORT 5

const float PERIOD = 16.0;   // one glass section + one steel sleeve
const float GLASS_LEN = 11.0;
const vec3  KEY_DIR = normalize(vec3(-0.35, 1.0, 0.55));
const vec3  KEY_COL = vec3(2.2, 2.25, 2.4);

struct Hit { float t; vec3 n; int mat; };

float axialS(vec3 p) { return dot(p - uPipeA, uPipeDir); }
bool inGlass(float s) { return mod(s, PERIOD) < GLASS_LEN; }

// ---------------------------------------------------------------- shapes ----
// infinite cylinder around the pipe axis, returns entry/exit distances
vec2 cylinder(vec3 ro, vec3 rd, float r) {
    vec3 oc = ro - uPipeA;
    vec3 rdp = rd - uPipeDir * dot(rd, uPipeDir);
    vec3 ocp = oc - uPipeDir * dot(oc, uPipeDir);
    float a = dot(rdp, rdp), b = dot(ocp, rdp), c = dot(ocp, ocp) - r * r;
    float h = b * b - a * c;
    if (h < 0.0 || a < 1e-8) return vec2(-1.0);
    h = sqrt(h);
    return vec2((-b - h) / a, (-b + h) / a);
}
vec3 cylNormal(vec3 p) {
    vec3 q = p - uPipeA;
    return normalize(q - uPipeDir * dot(q, uPipeDir));
}
// capped cylinder (Inigo Quilez's formula): returns t and normal
vec4 cappedCyl(vec3 ro, vec3 rd, vec3 pa, vec3 pb, float ra) {
    vec3 ba = pb - pa, oc = ro - pa;
    float baba = dot(ba, ba), bard = dot(ba, rd), baoc = dot(ba, oc);
    float k2 = baba - bard * bard;
    float k1 = baba * dot(oc, rd) - baoc * bard;
    float k0 = baba * dot(oc, oc) - baoc * baoc - ra * ra * baba;
    float h = k1 * k1 - k2 * k0;
    if (h < 0.0) return vec4(-1.0);
    h = sqrt(h);
    float t = (-k1 - h) / k2;
    float y = baoc + t * bard;
    if (y > 0.0 && y < baba) return vec4(t, (oc + t * rd - ba * y / baba) / ra);
    t = (((y < 0.0) ? 0.0 : baba) - baoc) / bard;
    if (abs(k1 + k2 * t) < h) return vec4(t, ba * sign(y) / sqrt(baba));
    return vec4(-1.0);
}
// axis-aligned box (slab method)
vec4 box(vec3 ro, vec3 rd, vec3 bmin, vec3 bmax) {
    vec3 inv = 1.0 / rd;
    vec3 t0 = (bmin - ro) * inv, t1 = (bmax - ro) * inv;
    vec3 tn = min(t0, t1), tf = max(t0, t1);
    float tN = max(max(tn.x, tn.y), tn.z), tF = min(min(tf.x, tf.y), tf.z);
    if (tN > tF || tF < 0.0) return vec4(-1.0);
    vec3 n = -sign(rd) * step(tn.yzx, tn.xyz) * step(tn.zxy, tn.xyz);
    return vec4(tN, n);
}

// ------------------------------------------------------------ the scene ----
// skipPipe = true when the ray starts inside / just left the pipe glass
Hit trace(vec3 ro, vec3 rd, bool skipPipe) {
    Hit h;
    h.t = 1e9;
    h.mat = MAT_NONE;
    // floor y = 0
    if (rd.y < 0.0) {
        float t = -ro.y / rd.y;
        if (t > 0.0 && t < h.t) { h.t = t; h.n = vec3(0, 1, 0); h.mat = MAT_FLOOR; }
    }
    // back wall z = -32
    if (rd.z < 0.0) {
        float t = (-32.0 - ro.z) / rd.z;
        if (t > 0.0 && t < h.t) { h.t = t; h.n = vec3(0, 0, 1); h.mat = MAT_WALL; }
    }
    // pipe body
    if (!skipPipe) {
        vec2 c = cylinder(ro, rd, uPipeR);
        if (c.x > 0.001 && c.x < h.t) {
            vec3 p = ro + rd * c.x;
            h.t = c.x;
            h.n = cylNormal(p);
            h.mat = inGlass(axialS(p)) ? MAT_GLASS : MAT_STEEL;
        }
    }
    // flanges at both ends of every steel sleeve, thin bands on the glass,
    // and the supports under the sleeves
    float sRo = axialS(ro);
    int k0 = int(floor((sRo - 100.0) / PERIOD)), k1 = int(floor((sRo + 100.0) / PERIOD));
    for (int k = k0; k <= k1; k++) {
        float base = float(k) * PERIOD;
        for (int f = 0; f < 4; f++) {
            float sc = f == 0 ? base + GLASS_LEN : (f == 1 ? base + PERIOD : (f == 2 ? base + 3.7 : base + 7.4));
            float hw = f < 2 ? 0.28 : 0.09;
            float rr = uPipeR * (f < 2 ? 1.14 : 1.035);
            vec4 r = cappedCyl(ro, rd, uPipeA + uPipeDir * (sc - hw), uPipeA + uPipeDir * (sc + hw), rr);
            if (r.x > 0.001 && r.x < h.t) {
                vec3 p = ro + rd * r.x;
                // caps are rings: ignore the part inside the pipe
                if (length((p - uPipeA) - uPipeDir * axialS(p)) > uPipeR * 0.999) {
                    h.t = r.x; h.n = r.yzw; h.mat = MAT_STEEL;
                }
            }
        }
        vec3 cpt = uPipeA + uPipeDir * (base + 13.5);
        vec4 b = box(ro, rd, vec3(cpt.x - 1.1, 0.0, -1.5), vec3(cpt.x + 1.1, cpt.y - uPipeR * 0.75, 1.5));
        if (b.x > 0.001 && b.x < h.t) { h.t = b.x; h.n = b.yzw; h.mat = MAT_SUPPORT; }
    }
    return h;
}

// how much key light reaches p (shadow ray)
vec3 shadowRay(vec3 p) {
    Hit h = trace(p + KEY_DIR * 0.02, KEY_DIR, false);
    if (h.mat == MAT_NONE || h.mat == MAT_WALL || h.mat == MAT_FLOOR) return vec3(1.0);
    if (h.mat == MAT_GLASS) return vec3(0.35, 0.6, 0.85);      // light through blue water
    return vec3(0.12);
}

// ------------------------------------------------ glowing water streaks ----
vec2 rot2(vec2 v, float a) { float c = cos(a), s = sin(a); return vec2(c * v.x - s * v.y, s * v.x + c * v.y); }

// density of light-emitting water at point p (inside the pipe)
float waterGlow(vec3 p, out float core) {
    vec3 q = p - uPipeA;
    float s = dot(q, uPipeDir);
    vec3 radial = q - uPipeDir * s;
    vec3 U = normalize(vec3(0, 1, 0) - uPipeDir * uPipeDir.y);
    vec3 W = cross(uPipeDir, U);
    vec2 c = vec2(dot(radial, U), dot(radial, W)) / uPipeR;
    float sf = s - uTime * uFlowSpeed;
    // the streams twist slowly while they race along the pipe
    c = rot2(c, sf * 0.05 + 1.5 * vnoise2(vec2(sf * 0.04, 3.0)));
    float n1 = vnoise3(vec3(sf * 0.11, c.x * 2.2, c.y * 2.2));
    float n2 = vnoise3(vec3(sf * 0.23 + 17.0, c.x * 4.5, c.y * 4.5));
    float fil = pow(1.0 - abs(n1 * 2.0 - 1.0), 14.0) + 0.7 * pow(1.0 - abs(n2 * 2.0 - 1.0), 20.0);
    // a wavy "sheet" (the top of the rushing water body)
    float surf = c.x - (0.25 + 0.18 * sin(sf * 0.21 + c.y * 2.0) + 0.1 * sin(sf * 0.57));
    float sheet = exp(-surf * surf * 60.0) * 0.8;
    // tiny sparkling droplets
    vec3 g = vec3(sf * 1.3, c * 7.0);
    vec3 id = floor(g), fr = fract(g) - 0.5;
    float sp = hash13(id) > 0.9 ? smoothstep(0.22, 0.0, length(fr * vec3(0.35, 1.0, 1.0))) : 0.0;
    float r = length(c);
    float body = smoothstep(1.0, 0.9, r) * (0.02 + 0.05 * step(c.x, 0.25));   // faint body of the water
    core = clamp(fil * 0.6 + sp, 0.0, 1.0);
    return (fil + sheet + sp * 2.5) * smoothstep(1.0, 0.88, r) + body;
}

// march through the water between t0 and t1 along the ray, collecting light
vec3 marchWater(vec3 ro, vec3 rd, float t0, float t1, int steps, out float transmit) {
    vec3 col = vec3(0.0);
    transmit = 1.0;
    float dt = (t1 - t0) / float(steps);
    float jitter = hash12(gl_FragCoord.xy + uTime);
    for (int i = 0; i < 64; i++) {
        if (i >= steps) break;
        float t = t0 + (float(i) + jitter) * dt;
        float core;
        float d = waterGlow(ro + rd * t, core) * uFlowOn;
        vec3 emit = mix(vec3(0.08, 0.42, 1.0), vec3(0.75, 0.95, 1.0), core);
        col += emit * d * dt * 0.8 * transmit;
        transmit *= exp(-dt * (0.05 + d * 0.12));
    }
    return col;
}

// ------------------------------------------------------------ materials ----
vec3 background(vec3 rd) {
    float h = clamp(rd.y * 0.5 + 0.5, 0.0, 1.0);
    return mix(vec3(0.05, 0.06, 0.075), vec3(0.16, 0.18, 0.21), h);
}

float gridLines(vec2 p, float spacing, float width) {
    vec2 g = abs(fract(p / spacing - 0.5) - 0.5) * spacing;
    vec2 w = fwidth(p) * 1.2 + width;
    vec2 l = 1.0 - smoothstep(vec2(0.0), w, g);
    return max(l.x, l.y);
}

vec3 floorAlbedo(vec3 p) {
    vec2 cell = floor(p.xz / 4.0);
    float panel = 0.75 + 0.5 * hash12(cell);
    vec3 c = vec3(0.07, 0.075, 0.085) * panel;
    c += vec3(0.25, 0.55, 0.75) * gridLines(p.xz, 4.0, 0.015) * 0.35;
    c += vec3(0.2, 0.45, 0.6) * gridLines(p.xz, 20.0, 0.04) * 0.5;
    return c;
}
vec3 wallColor(vec3 p) {
    vec3 c = vec3(0.06, 0.065, 0.075);
    c += vec3(0.25, 0.5, 0.7) * gridLines(p.xy, 5.0, 0.02) * 0.25;
    float panel = step(0.82, hash12(floor(p.xy / vec2(10.0, 5.0))));
    c += vec3(0.03, 0.035, 0.04) * panel;
    return c;
}

// light that the glowing water throws onto nearby surfaces
vec3 waterBounce(vec3 p) {
    vec3 q = p - uPipeA;
    float s = dot(q, uPipeDir);
    float d = length(q - uPipeDir * s) - uPipeR;
    return vec3(0.1, 0.4, 0.9) * 0.9 * exp(-max(d, 0.0) * 0.45) * (inGlass(s) ? 1.0 : 0.25) * uFlowOn;
}

vec3 phongSteel(vec3 p, vec3 n, vec3 rd, vec3 base) {
    vec3 sh = shadowRay(p + n * 0.01);
    float ndl = max(dot(n, KEY_DIR), 0.0);
    vec3 H = normalize(KEY_DIR - rd);
    float spec = pow(max(dot(n, H), 0.0), 60.0);
    vec3 env = mix(background(reflect(rd, n)), vec3(0.1, 0.11, 0.12), 0.3);
    float fres = 0.25 + 0.75 * pow(1.0 - max(dot(n, -rd), 0.0), 5.0);
    return base * (0.18 + ndl * KEY_COL * sh * 0.6) + spec * KEY_COL * sh * 0.6 + env * fres * 0.6 + base * waterBounce(p);
}

// cheap shading for secondary rays (reflections, through-glass views)
vec3 shadeSimple(vec3 ro, vec3 rd, Hit h, bool skipPipe) {
    if (h.mat == MAT_NONE) return background(rd);
    vec3 p = ro + rd * h.t;
    if (h.mat == MAT_FLOOR) return floorAlbedo(p) * (0.6 + 0.8 * max(dot(h.n, KEY_DIR), 0.0)) + waterBounce(p) * 0.25;
    if (h.mat == MAT_WALL) return wallColor(p);
    if (h.mat == MAT_SUPPORT) return vec3(0.25, 0.26, 0.28) * (0.3 + max(dot(h.n, KEY_DIR), 0.0));
    if (h.mat == MAT_STEEL) {
        float ndl = max(dot(h.n, KEY_DIR), 0.0);
        return vec3(0.3, 0.32, 0.35) * (0.25 + ndl * 1.4) + background(reflect(rd, h.n)) * 0.3;
    }
    // glass seen in a reflection: short march straight through
    vec2 c = cylinder(ro, rd, uPipeR * 0.97);
    float tr;
    vec3 glow = marchWater(ro, rd, max(c.x, 0.0), c.y, 14, tr);
    return glow + vec3(0.04, 0.08, 0.12) * tr;
}

// ----------------------------------------------------------------- main ----
void main() {
    vec3 ro = uCamPos;
    vec3 rd = normalize(uCamForward + vNdc.x * uTanHalfFov * uAspect * uCamRight + vNdc.y * uTanHalfFov * uCamUp);
    Hit h = trace(ro, rd, false);
    vec3 col;
    vec3 p = ro + rd * h.t;

    if (h.mat == MAT_NONE) {
        col = background(rd);
    } else if (h.mat == MAT_FLOOR) {
        // glossy floor: lit + shadow ray + ray-traced reflection
        vec3 base = floorAlbedo(p);
        vec3 sh = shadowRay(p);
        col = base * (0.35 + 0.9 * max(dot(h.n, KEY_DIR), 0.0) * sh * KEY_COL * 0.5) + waterBounce(p) * 0.35;
        vec3 rr = reflect(rd, h.n);
        Hit hr = trace(p + h.n * 0.01, rr, false);
        float fres = 0.08 + 0.55 * pow(1.0 - max(dot(h.n, -rd), 0.0), 4.0);
        col += shadeSimple(p + h.n * 0.01, rr, hr, false) * fres;
    } else if (h.mat == MAT_WALL) {
        col = wallColor(p) + waterBounce(p) * 0.2;
    } else if (h.mat == MAT_SUPPORT) {
        vec3 base = vec3(0.3, 0.31, 0.33) * (0.85 + 0.3 * vnoise2(p.xy * 3.0));
        col = base * (0.25 + max(dot(h.n, KEY_DIR), 0.0) * shadowRay(p + h.n * 0.01) * 1.2) + base * waterBounce(p) * 1.5;
    } else if (h.mat == MAT_STEEL) {
        col = phongSteel(p, h.n, rd, vec3(0.2, 0.21, 0.24));
    } else {
        // ---- GLASS: reflection + refraction into the water + glow + exit ----
        vec3 n = h.n;
        float cosi = max(dot(n, -rd), 0.0);
        float F = 0.04 + 0.96 * pow(1.0 - cosi, 5.0);              // Fresnel
        vec3 rr = reflect(rd, n);
        Hit hr = trace(p + n * 0.01, rr, true);
        vec3 refl = shadeSimple(p + n * 0.01, rr, hr, true);
        vec3 Hh = normalize(KEY_DIR - rd);
        refl += KEY_COL * pow(max(dot(n, Hh), 0.0), 220.0) * 1.5;   // highlight on the glass

        vec3 rdIn = refract(rd, n, 1.0 / 1.33);                     // Snell: air -> water
        vec2 inner = cylinder(p + rdIn * 0.01, rdIn, uPipeR * 0.97);
        float tExit = max(inner.y, 0.05);
        float tr;
        vec3 glow = marchWater(p + rdIn * 0.01, rdIn, 0.0, tExit, 36, tr);
        vec3 pe = p + rdIn * (0.01 + tExit);
        vec3 ne = -cylNormal(pe);                                   // normal facing back inside
        vec3 rdOut = refract(rdIn, ne, 1.33);                       // water -> air
        if (dot(rdOut, rdOut) < 0.01) rdOut = reflect(rdIn, ne);    // total internal reflection
        Hit hb = trace(pe + rdOut * 0.02, rdOut, true);
        vec3 behind = shadeSimple(pe + rdOut * 0.02, rdOut, hb, true);
        vec3 waterTint = vec3(0.55, 0.8, 1.0);
        vec3 trans = glow + behind * waterTint * tr * 0.3 + vec3(0.0, 0.025, 0.07);
        // moving scan ring (hologram effect)
        float sp = axialS(p);
        trans += vec3(0.3, 0.8, 1.0) * exp(-pow((sp - uScan) * 1.5, 2.0)) * 0.6;
        col = mix(trans, refl, F) + vec3(0.02, 0.04, 0.06);
    }

    // depth haze and a soft vignette
    float dist = h.mat == MAT_NONE ? 200.0 : h.t;
    col = mix(col, background(rd) * 1.2, 1.0 - exp(-dist * 0.006));
    vec2 uv = vNdc;
    col *= 1.0 - 0.25 * dot(uv * 0.7, uv * 0.7);
    FragColor = vec4(finalColor(col), 1.0);
}
