#include "HudOverlay.h"
#include "BitmapFont.h"
#include "Hud.h"
#include "HudRenderer.h"
#include "../App.h"
#include "../input/Picking.h"
#include "../objects/SolarArray.h"
#include "../systems/BoatSystem.h"
#include "../systems/Environment.h"
#include "../systems/HydroSystem.h"
#include "../systems/WindSystem.h"
#include <algorithm>
#include <cmath>
#include <cstdarg>
#include <cstdio>
#include <string>
#include <vector>

namespace {

using glm::vec2;
using glm::vec4;

const vec4 TEXT(0.95f, 0.97f, 1.00f, 1.0f);
const vec4 LABEL(0.60f, 0.76f, 0.90f, 1.0f);
const vec4 TITLE(0.55f, 0.88f, 1.00f, 1.0f);
const vec4 WIND_COLOR(0.40f, 0.82f, 1.00f, 1.0f);
const vec4 GATE_COLOR(1.00f, 0.76f, 0.28f, 1.0f);
const vec4 FLOW_COLOR(0.35f, 0.62f, 1.00f, 1.0f);
const vec4 WARN(1.00f, 0.55f, 0.25f, 1.0f);
const vec4 GOOD(0.55f, 0.95f, 0.55f, 1.0f);
const vec4 PANEL(0.03f, 0.06f, 0.10f, 0.58f);
const vec4 BAR_BACK(1.0f, 1.0f, 1.0f, 0.14f);

constexpr float RPM_PER_RAD_PER_SECOND = 60.0f / 6.28318530718f;

std::string format(const char* f, ...) {
    char buffer[256];
    va_list args;
    va_start(args, f);
    std::vsnprintf(buffer, sizeof(buffer), f, args);
    va_end(args);
    return buffer;
}

// One line of the status panel: a label column, a value, and optionally a bar.
struct Line {
    std::string label, value;
    vec4 valueColor = TEXT;
    float bar = -1.0f;          // 0..1 fill; < 0 = no bar
    float marker = -1.0f;       // 0..1 target marker on the bar; < 0 = none
    vec4 barColor = WIND_COLOR;
};

// Draws a panel and returns its size. alignRight: `x` is the margin from the
// right edge of the screen instead of the left.
vec2 drawPanel(HudRenderer& hud, float x, float y, float pixel, const std::string& title, const std::vector<Line>& lines,
               bool alignRight = false) {
    const float lineH = BitmapFont::LINE_HEIGHT * pixel;
    const float labelW = 7 * BitmapFont::ADVANCE * pixel;
    bool anyBar = false;
    for (const Line& l : lines) anyBar = anyBar || l.bar >= 0.0f;
    const float barW = anyBar ? 16 * BitmapFont::ADVANCE * pixel * 0.5f : 0.0f;
    float valueW = 0.0f;
    for (const Line& l : lines) valueW = std::max(valueW, (float)BitmapFont::textWidth(l.value) * pixel);
    float titleW = (float)BitmapFont::textWidth(title) * pixel;
    float width = std::max(titleW, labelW + valueW + barW + 4 * pixel) + 2 * 6 * pixel;
    float height = lineH * (lines.size() + 1) + 2 * 5 * pixel;
    if (alignRight) x = hud.width() - width - x;

    hud.rect(x, y, width, height, PANEL);
    hud.rect(x, y, 2 * pixel, height, vec4(TITLE.r, TITLE.g, TITLE.b, 0.9f));   // accent stripe
    float cx = x + 6 * pixel, cy = y + 5 * pixel;
    hud.text(title, cx, cy, pixel, TITLE);
    cy += lineH;
    for (const Line& l : lines) {
        hud.text(l.label, cx, cy, pixel, LABEL);
        hud.text(l.value, cx + labelW, cy, pixel, l.valueColor);
        if (l.bar >= 0.0f) {
            float bx = cx + labelW + valueW + 3 * pixel, by = cy + 1 * pixel, bh = 5 * pixel;
            hud.rect(bx, by, barW, bh, BAR_BACK);
            hud.rect(bx, by, barW * std::clamp(l.bar, 0.0f, 1.0f), bh, l.barColor);
            if (l.marker >= 0.0f) hud.rect(bx + barW * std::clamp(l.marker, 0.0f, 1.0f) - pixel * 0.5f, by - pixel, pixel, bh + 2 * pixel, TEXT);
        }
        cy += lineH;
    }
    return {width, height};
}

std::string windStatus(const PlantState& p) {
    float w = p.windMetersPerSecond;
    if (w > WindSystem::CUT_OUT) return "STORM: ROTORS STOPPED (CUT-OUT)";
    if (w < WindSystem::CUT_IN) return "TOO CALM TO TURN";
    return format("ROTORS %.1f RPM", p.rotorSpeedRadPerSecond[0] * RPM_PER_RAD_PER_SECOND);
}

std::vector<Line> plantLines(const App& app) {
    const PlantState& p = app.state.plant;
    const ViewSettings& v = app.state.view;
    std::vector<Line> lines;

    lines.push_back({"VIEW", std::string(CameraViews::name(app.director.view)) + (v.orthographic ? "  ORTHOGRAPHIC" : "  PERSPECTIVE")});
    std::string shading = std::string(Hud::shadingName(v.shading)) + (v.useBlinn ? " (BLINN)" : "");
    if (v.terms != LightingTerms::All) shading += std::string("  ") + Hud::termsName(v.terms);
    lines.push_back({"SHADE", shading + "  LIGHTS " + Environment::lightSetName(v.lights)});
    lines.push_back({"TIME", format("%.1f S  X%g%s", p.simulationTimeSeconds, app.clock.getTimeScale(), app.clock.isPaused() ? "  PAUSED" : ""),
                     app.clock.isPaused() ? WARN : TEXT});

    Line wind{"WIND", format("%4.1f M/S", p.windMetersPerSecond)};
    if (std::fabs(p.windTargetMetersPerSecond - p.windMetersPerSecond) > 0.1f)
        wind.value += format(" > %.0f", p.windTargetMetersPerSecond);
    wind.bar = p.windMetersPerSecond / WindSystem::MAX_WIND;
    wind.marker = WindSystem::CUT_OUT / WindSystem::MAX_WIND;   // the white tick = cut-out
    wind.barColor = p.windMetersPerSecond > WindSystem::CUT_OUT ? WARN : WIND_COLOR;
    lines.push_back(wind);
    lines.push_back({"", windStatus(p), p.windMetersPerSecond > WindSystem::CUT_OUT ? WARN : TEXT});

    Line gate{"GATE", format("%3.0f%%", p.gateOpening * 100.0f)};
    if (std::fabs(p.gateTarget - p.gateOpening) > 0.005f) gate.value += format(" > %.0f%%", p.gateTarget * 100.0f);
    if (p.gateAutoDemo) gate.value += "  AUTO";
    gate.bar = p.gateOpening;
    gate.marker = p.gateTarget;
    gate.barColor = GATE_COLOR;
    lines.push_back(gate);

    Line flow{"FLOW", format("%3.0f%%  WHEEL %.1f RPM", HydroSystem::flowStrength(p) * 100.0f,
                             p.wheelSpeedRadPerSecond * RPM_PER_RAD_PER_SECOND)};
    flow.bar = HydroSystem::flowStrength(p);
    flow.barColor = FLOW_COLOR;
    lines.push_back(flow);

    float headingDeg = std::fmod(p.boatHeadingRad * 57.29578f + 360.0f, 360.0f);
    lines.push_back({"BOAT", format("%s %.1f M/S  HDG %03.0f~", p.boatMode == BoatMode::Patrol ? "PATROL" : "MANUAL",
                                    p.boatSpeed, headingDeg)});
    if (p.boatBlockedByShore) lines.push_back({"", "STOPPED: SHORE OR DAM AHEAD", WARN});
    lines.push_back({"SOLAR", format("TILT %.0f~", p.solarTiltDeg)});
    return lines;
}

std::vector<Line> galleryLines(const App& app, const std::string& detail) {
    const ViewSettings& v = app.state.view;
    std::vector<Line> lines;
    lines.push_back({"SHADE", std::string(Hud::shadingName(v.shading)) + (v.useBlinn ? " (BLINN)" : "") + "  TERMS " + Hud::termsName(v.terms)});
    lines.push_back({"LIGHTS", Environment::lightSetName(v.lights)});
    lines.push_back({"MESH", detail});
    lines.push_back({"ORDER", v.galleryWrongMatrixOrder ? "WRONG: S*R*T (O)" : "CORRECT: T*R*S (O)", v.galleryWrongMatrixOrder ? WARN : GOOD});
    lines.push_back({"NORMAL", v.galleryWrongNormalMatrix ? "WRONG: MODEL MATRIX (M)" : "CORRECT: INVERSE-TRANSPOSE (M)",
                     v.galleryWrongNormalMatrix ? WARN : GOOD});
    lines.push_back({"TIME", format("%.1f S%s", app.state.plant.simulationTimeSeconds, app.clock.isPaused() ? "  PAUSED" : "")});
    return lines;
}

// Which keys matter for the selected object.
std::vector<Line> selectionLines(const App& app) {
    const PlantState& p = app.state.plant;
    std::vector<Line> lines;
    switch (app.state.view.selected) {
        case Selectable::Turbine1:
        case Selectable::Turbine2: {
            int i = app.state.view.selected == Selectable::Turbine1 ? 0 : 1;
            lines.push_back({"KEYS", "Q / E  WIND DOWN / UP"});
            lines.push_back({"ROTOR", format("%.1f RPM  (CUT-IN %.0f, RATED %.0f, CUT-OUT %.0f M/S)",
                                             p.rotorSpeedRadPerSecond[i] * RPM_PER_RAD_PER_SECOND,
                                             WindSystem::CUT_IN, WindSystem::RATED, WindSystem::CUT_OUT)});
            break;
        }
        case Selectable::Gate:
            lines.push_back({"KEYS", "G OPEN/SHUT   F / H LOWER/RAISE   J AUTO"});
            lines.push_back({"GATE", format("%.0f%% > %.0f%%  (MAX %.1f%% PER S)", p.gateOpening * 100.0f, p.gateTarget * 100.0f,
                                            HydroSystem::GATE_SPEED_PER_SECOND * 100.0f)});
            break;
        case Selectable::Wheel:
            lines.push_back({"KEYS", "FOLLOWS THE GATE (G, F/H, J)"});
            lines.push_back({"WHEEL", format("%.1f RPM  > %.1f RPM", p.wheelSpeedRadPerSecond * RPM_PER_RAD_PER_SECOND,
                                             HydroSystem::targetWheelSpeed(p) * RPM_PER_RAD_PER_SECOND)});
            break;
        case Selectable::Solar: {
            float lo, hi;
            SolarArray::tiltLimits(lo, hi);
            lines.push_back({"KEYS", "Z / C  TILT DOWN / UP"});
            lines.push_back({"TILT", format("%.0f~  (%.0f~ - %.0f~, TURNS ABOUT THE TUBE)", p.solarTiltDeg, lo, hi)});
            break;
        }
        case Selectable::Boat:
            lines.push_back({"KEYS", "V PATROL/MANUAL   W/S THROTTLE   A/D RUDDER"});
            lines.push_back({"BOAT", format("%s  %.1f M/S", p.boatMode == BoatMode::Patrol ? "AUTOPILOT" : "YOU STEER", p.boatSpeed)});
            break;
        default:
            break;
    }
    return lines;
}

// A small compass: the arrow shows where the wind blows, seen from the camera.
// `top`: y of the compass box's top edge (it sits at the right of the screen).
void drawWindCompass(HudRenderer& hud, const App& app, float pixel, float top) {
    const Camera& cam = app.activeCamera();
    glm::vec3 forward = cam.target - cam.getPosition();
    forward.y = 0.0f;
    if (glm::length(forward) < 1e-4f) return;
    forward = glm::normalize(forward);
    glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0, 1, 0)));
    glm::vec3 wind = WindSystem::direction();
    vec2 dir(glm::dot(wind, right), -glm::dot(wind, forward));   // screen: x right, y down
    dir = glm::normalize(dir);
    vec2 side(-dir.y, dir.x);

    float radius = 24.0f * pixel;
    vec2 c((float)hud.width() - radius - 4.0f * pixel - 12.0f, top + radius + 4.0f * pixel);
    hud.rect(c.x - radius - 4 * pixel, c.y - radius - 4 * pixel, 2 * radius + 8 * pixel, 2 * radius + 16 * pixel, PANEL);
    // Ring: 32 short segments.
    for (int i = 0; i < 32; ++i) {
        float a0 = 6.2832f * i / 32.0f, a1 = 6.2832f * (i + 1) / 32.0f;
        vec2 p0 = c + radius * vec2(std::cos(a0), std::sin(a0)), p1 = c + radius * vec2(std::cos(a1), std::sin(a1));
        vec2 q0 = c + (radius - pixel) * vec2(std::cos(a0), std::sin(a0)), q1 = c + (radius - pixel) * vec2(std::cos(a1), std::sin(a1));
        hud.triangle(p0, p1, q1, vec4(1, 1, 1, 0.35f));
        hud.triangle(p0, q1, q0, vec4(1, 1, 1, 0.35f));
    }
    // Arrow: a shaft plus a head, length scaled by the wind strength.
    float strength = std::clamp(app.state.plant.windMetersPerSecond / 15.0f, 0.25f, 1.0f);
    vec2 tail = c - dir * radius * 0.7f * strength, tip = c + dir * radius * 0.8f * strength;
    vec2 neck = tip - dir * 6.0f * pixel;
    hud.triangle(tail + side * pixel, neck + side * pixel, neck - side * pixel, WIND_COLOR);
    hud.triangle(tail + side * pixel, neck - side * pixel, tail - side * pixel, WIND_COLOR);
    hud.triangle(tip, neck + side * 4.0f * pixel, neck - side * 4.0f * pixel, WIND_COLOR);
    std::string label = format("WIND %.0f", app.state.plant.windMetersPerSecond);
    hud.text(label, c.x - BitmapFont::textWidth(label) * pixel * 0.5f, c.y + radius + 3 * pixel, pixel, LABEL);
}

void drawHelp(HudRenderer& hud, float pixel) {
    const char* rows[] = {
        "CAMERA     LEFT-DRAG ORBIT   WHEEL ZOOM   R RESET VIEW",
        "VIEWS      F1 OVERVIEW  F2 TURBINE  F3 DAM+WHEEL  F4 SOLAR  F5 BOAT",
        "           P PERSPECTIVE / ORTHOGRAPHIC",
        "SELECT     RIGHT-CLICK AN OBJECT   TAB NEXT OBJECT",
        "TIME       SPACE PAUSE   [ / ] SLOWER / FASTER   ENTER RESET SCENE",
        "WIND       Q / E  DOWN / UP (HOLD)",
        "GATE       G OPEN/SHUT   F / H LOWER/RAISE (HOLD)   J AUTO DEMO",
        "BOAT       V PATROL/MANUAL   W/S THROTTLE   A/D RUDDER",
        "SOLAR      Z / C  TILT DOWN / UP (HOLD)",
        "SHADING    1 FLAT  2 GOURAUD  3 PHONG   B BLINN",
        "LIGHTING   K TERMS   L LIGHTS   ARROWS + PGUP/PGDN MOVE LAMP",
        "DEBUG      X WIREFRAME   N NORMALS   F6 GALLERY (+/- O M)",
        "SCREEN     BACKSPACE HIDE PANEL   F11 THIS HELP   F12 SCREENSHOT",
        "           ESC QUIT",
    };
    const int count = (int)(sizeof(rows) / sizeof(rows[0]));
    float lineH = BitmapFont::LINE_HEIGHT * pixel;
    float w = 0.0f;
    for (const char* r : rows) w = std::max(w, (float)BitmapFont::textWidth(r) * pixel);
    w += 16 * pixel;
    float h = lineH * (count + 2) + 10 * pixel;
    float x = (hud.width() - w) * 0.5f, y = (hud.height() - h) * 0.5f;
    hud.rect(x, y, w, h, vec4(0.02f, 0.04f, 0.07f, 0.86f));
    hud.text("CONTROLS", x + 8 * pixel, y + 6 * pixel, pixel, TITLE);
    for (int i = 0; i < count; ++i) hud.text(rows[i], x + 8 * pixel, y + 6 * pixel + lineH * (i + 1.5f), pixel, TEXT);
}

}

void HudOverlay::draw(HudRenderer& hud, const App& app, const std::string& galleryDetail, float fps) {
    const ViewSettings& v = app.state.view;
    const float pixel = std::max(2.0f, std::round(hud.height() / 450.0f));

    hud.vignette(v.galleryActive ? 0.15f : 0.38f);   // cinematic edge darkening
    if (!v.hudVisible) return;                       // Backspace: clean picture

    const float hintHeight = BitmapFont::LINE_HEIGHT * pixel + 8;
    if (v.galleryActive) {
        drawPanel(hud, 12, 12, pixel, "GRAPHICS GALLERY  (F6 RETURNS)", galleryLines(app, galleryDetail));
    } else {
        // Top-RIGHT: over the sky, so the turbines on the left stay visible.
        vec2 size = drawPanel(hud, 12, 12, pixel, "CONFLUENCE  -  RENEWABLE POWER PLANT", plantLines(app), true);
        drawWindCompass(hud, app, pixel, 12 + size.y + 6);

        if (v.selected != Selectable::None) {
            std::vector<Line> sel = selectionLines(app);
            float h = BitmapFont::LINE_HEIGHT * pixel * (sel.size() + 1) + 10 * pixel;
            drawPanel(hud, 12, hud.height() - h - hintHeight - 6, pixel, std::string("SELECTED: ") + Picking::name(v.selected), sel);
        }
    }

    std::string hint = format("F11 HELP   BACKSPACE HIDE   RIGHT-CLICK SELECT   %.0f FPS", fps);
    float hw = BitmapFont::textWidth(hint) * pixel;
    hud.text(hint, hud.width() - hw - 12, hud.height() - hintHeight + 2, pixel, vec4(1, 1, 1, 0.75f));

    if (v.helpVisible) drawHelp(hud, pixel);
}
