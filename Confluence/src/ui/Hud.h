#pragma once
#include <string>

struct App;
enum class ShadingMode;
enum class LightingTerms;

// The "heads-up display" for Steps 1-4, using no extra library:
//   * the WINDOW TITLE shows the live status (mode, time, FPS, ...)
//   * the CONSOLE shows the controls and one line per key press.
namespace Hud {
    const char* shadingName(ShadingMode mode);
    const char* termsName(LightingTerms terms);

    void printHelp();

    // One line, e.g. "Confluence | SCENE | Phong | terms: all | ...".
    // galleryDetail is the gallery's triangle-count summary ("" in the scene).
    std::string statusLine(const App& app, const std::string& galleryDetail, float fps);

    // Measures frames per second over ~0.25 s windows and says when the
    // title should be refreshed (setting the title every frame is wasteful).
    class TitleTimer {
    public:
        // Returns true about four times per second.
        bool update(float realDeltaSeconds);
        float fps() const { return framesPerSecond; }
    private:
        float elapsed = 0.0f;
        int frames = 0;
        float framesPerSecond = 0.0f;
    };
}
