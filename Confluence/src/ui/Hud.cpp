#include "Hud.h"
#include "../App.h"
#include "../systems/Environment.h"
#include <cstdio>
#include <iostream>

const char* Hud::shadingName(ShadingMode mode) {
    switch (mode) {
        case ShadingMode::Flat:    return "Flat";
        case ShadingMode::Gouraud: return "Gouraud";
        case ShadingMode::Phong:   return "Phong";
    }
    return "?";
}

const char* Hud::termsName(LightingTerms terms) {
    switch (terms) {
        case LightingTerms::All:          return "all";
        case LightingTerms::AmbientOnly:  return "ambient only";
        case LightingTerms::DiffuseOnly:  return "diffuse only";
        case LightingTerms::SpecularOnly: return "specular only";
    }
    return "?";
}

void Hud::printHelp() {
    std::cout <<
        "\n=================== CONFLUENCE - CONTROLS (Steps 1-8) ===================\n"
        "  Camera     left-drag orbit | wheel zoom | R reset to overview\n"
        "  Views      F1 overview | F2 turbine | F3 dam+wheel | F4 solar | F5 boat (follows)\n"
        "             P perspective <-> orthographic\n"
        "  Select     right-click an object | Tab next object\n"
        "  Time       Space pause/resume | [ slower | ] faster | Enter reset scene\n"
        "  Wind       Q / E  wind down / up (hold)\n"
        "  Gate       G open/shut | F / H lower / raise target (hold) | J auto demo\n"
        "  Boat       V patrol <-> manual | W/S throttle | A/D rudder\n"
        "  Solar      Z / C  tilt down / up (hold)\n"
        "  Shading    1 Flat | 2 Gouraud | 3 Phong | B Blinn-Phong specular\n"
        "  Lighting   K terms | L lights | arrows + PageUp/PageDown move the lamp\n"
        "  Debug      X wireframe | N normals | F6 gallery (+ / - detail, O order, M normals)\n"
        "  Screen     Backspace hide panel | F11 help on screen | F12 screenshot | Esc quit\n"
        "=========================================================================\n"
        << std::endl;
}

std::string Hud::statusLine(const App& app, const std::string& galleryDetail, float fps) {
    const ViewSettings& v = app.state.view;
    char buffer[512];
    std::snprintf(buffer, sizeof(buffer),
        "Confluence | %s | %s%s | terms: %s | lights: %s | t=%.1fs x%g%s%s%s | %.0f FPS | F11 help",
        v.galleryActive ? "GALLERY" : "SCENE",
        shadingName(v.shading),
        v.useBlinn ? " (Blinn)" : "",
        termsName(v.terms),
        Environment::lightSetName(v.lights),
        app.state.plant.simulationTimeSeconds,
        app.clock.getTimeScale(),
        app.clock.isPaused() ? " PAUSED" : "",
        v.wireframe ? " | WIREFRAME" : "",
        v.showNormals ? " | NORMALS" : "",
        fps);

    std::string line = buffer;
    if (v.galleryActive) {
        line += " | " + galleryDetail;
        if (v.galleryWrongMatrixOrder) line += " | WRONG ORDER S*R*T";
        if (v.galleryWrongNormalMatrix) line += " | WRONG NORMAL MATRIX";
    }
    return line;
}

bool Hud::TitleTimer::update(float realDeltaSeconds) {
    elapsed += realDeltaSeconds;
    ++frames;
    if (elapsed < 0.25f) return false;
    framesPerSecond = frames / elapsed;
    elapsed = 0.0f;
    frames = 0;
    return true;
}
