#include "CommandLine.h"
#include "../App.h"
#include "../systems/HydroSystem.h"
#include "../systems/WindSystem.h"
#include <cstdlib>
#include <iostream>

namespace {

bool parseShading(const std::string& s, ShadingMode& out) {
    if (s == "flat")    { out = ShadingMode::Flat;    return true; }
    if (s == "gouraud") { out = ShadingMode::Gouraud; return true; }
    if (s == "phong")   { out = ShadingMode::Phong;   return true; }
    return false;
}

bool parseTerms(const std::string& s, LightingTerms& out) {
    if (s == "all")      { out = LightingTerms::All;          return true; }
    if (s == "ambient")  { out = LightingTerms::AmbientOnly;  return true; }
    if (s == "diffuse")  { out = LightingTerms::DiffuseOnly;  return true; }
    if (s == "specular") { out = LightingTerms::SpecularOnly; return true; }
    return false;
}

bool parseLights(const std::string& s, LightSet& out) {
    if (s == "all")  { out = LightSet::All;      return true; }
    if (s == "sun")  { out = LightSet::SunOnly;  return true; }
    if (s == "fill") { out = LightSet::FillOnly; return true; }
    if (s == "lamp") { out = LightSet::LampOnly; return true; }
    return false;
}

bool parseDetail(const std::string& s, GalleryDetail& out) {
    if (s == "low")    { out = GalleryDetail::Low;    return true; }
    if (s == "medium") { out = GalleryDetail::Medium; return true; }
    if (s == "high")   { out = GalleryDetail::High;   return true; }
    return false;
}

bool parseView(const std::string& s, CameraView& out) {
    if (s == "overview") { out = CameraView::Overview;    return true; }
    if (s == "turbine")  { out = CameraView::Turbine;     return true; }
    if (s == "dam")      { out = CameraView::DamAndWheel; return true; }
    if (s == "solar")    { out = CameraView::Solar;       return true; }
    if (s == "boat")     { out = CameraView::Boat;        return true; }
    return false;
}

bool parseSelect(const std::string& s, Selectable& out) {
    if (s == "turbine1") { out = Selectable::Turbine1; return true; }
    if (s == "turbine2") { out = Selectable::Turbine2; return true; }
    if (s == "gate")     { out = Selectable::Gate;     return true; }
    if (s == "wheel")    { out = Selectable::Wheel;    return true; }
    if (s == "solar")    { out = Selectable::Solar;    return true; }
    if (s == "boat")     { out = Selectable::Boat;     return true; }
    return false;
}

float number(const char* text) { return (float)std::atof(text); }

}

void CommandLine::printUsage() {
    std::cout <<
        "Usage: confluence.exe [--selftest] [--screenshot FILE.bmp] [options]\n"
        "  --gallery  --shading flat|gouraud|phong  --terms all|ambient|diffuse|specular\n"
        "  --lights all|sun|fill|lamp  --detail low|medium|high\n"
        "  --blinn --wireframe --normals --wrong-order --wrong-normals\n"
        "  --view overview|turbine|dam|solar|boat  --ortho  --select OBJECT\n"
        "  --no-hud --help-overlay --paused\n"
        "  --wind M/S  --gate %  --gate-target %  --auto-gate  --tilt DEG  --manual-boat THROTTLE\n"
        "  --advance SECONDS\n";
}

CommandLineOptions CommandLine::parse(int argc, char** argv, App& app) {
    CommandLineOptions options;
    ViewSettings& view = app.state.view;

    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        const bool hasValue = (i + 1 < argc);
        bool ok = true;

        if      (arg == "--selftest")      options.selfTest = true;
        else if (arg == "--gallery")       view.galleryActive = true;
        else if (arg == "--blinn")         view.useBlinn = true;
        else if (arg == "--wireframe")     view.wireframe = true;
        else if (arg == "--normals")       view.showNormals = true;
        else if (arg == "--wrong-order")   view.galleryWrongMatrixOrder = true;
        else if (arg == "--wrong-normals") view.galleryWrongNormalMatrix = true;
        else if (arg == "--ortho")         view.orthographic = true;
        else if (arg == "--no-hud")        view.hudVisible = false;
        else if (arg == "--help-overlay")  view.helpVisible = true;
        else if (arg == "--paused")        options.paused = true;
        else if (arg == "--auto-gate")     options.autoGate = true;
        else if (arg == "--screenshot" && hasValue)  options.screenshotPath = argv[++i];
        else if (arg == "--advance" && hasValue)     options.advanceSeconds = number(argv[++i]);
        else if (arg == "--wind" && hasValue)        options.wind = number(argv[++i]);
        else if (arg == "--gate" && hasValue)        options.gate = number(argv[++i]) / 100.0f;
        else if (arg == "--gate-target" && hasValue) options.gateTarget = number(argv[++i]) / 100.0f;
        else if (arg == "--tilt" && hasValue)        options.tilt = number(argv[++i]);
        else if (arg == "--manual-boat" && hasValue) { options.manualBoat = true; options.boatThrottle = number(argv[++i]); }
        else if (arg == "--shading" && hasValue)     ok = parseShading(argv[++i], view.shading);
        else if (arg == "--terms" && hasValue)       ok = parseTerms(argv[++i], view.terms);
        else if (arg == "--lights" && hasValue)      ok = parseLights(argv[++i], view.lights);
        else if (arg == "--detail" && hasValue)      ok = parseDetail(argv[++i], view.galleryDetail);
        else if (arg == "--view" && hasValue)        ok = parseView(argv[++i], options.view);
        else if (arg == "--select" && hasValue)      ok = parseSelect(argv[++i], view.selected);
        else ok = false;

        if (!ok) {
            std::cerr << "Unknown or incomplete option: " << arg << "\n";
            printUsage();
            options.valid = false;
            return options;
        }
    }
    return options;
}

void CommandLine::applyPlantOptions(const CommandLineOptions& o, App& app) {
    PlantState& plant = app.state.plant;
    if (o.wind >= 0.0f) {
        plant.windTargetMetersPerSecond = plant.windMetersPerSecond = o.wind;
        plant.rotorSpeedRadPerSecond[0] = WindSystem::targetRotorSpeed(o.wind);
        plant.rotorSpeedRadPerSecond[1] = plant.rotorSpeedRadPerSecond[0] * WindSystem::SECOND_TURBINE_FACTOR;
    }
    if (o.gate >= 0.0f) {
        plant.gateOpening = plant.gateTarget = o.gate;
        plant.wheelSpeedRadPerSecond = HydroSystem::targetWheelSpeed(plant);
    }
    if (o.gateTarget >= 0.0f) plant.gateTarget = o.gateTarget;
    plant.gateAutoDemo = o.autoGate;
    if (o.tilt >= 0.0f) plant.solarTiltDeg = o.tilt;
    if (o.manualBoat) {
        plant.boatMode = BoatMode::Manual;
        plant.boatThrottleInput = o.boatThrottle;
    }
    app.clock.setPaused(o.paused);
}
