#pragma once

// `confluence.exe --selftest`
//
// Checks the "Done when" conditions of Steps 1-8 with numbers instead of eyes.
// No window, no OpenGL: it drives Clock, Animation, the systems, picking and
// the camera directly with fake frame times. Prints one [PASS]/[FAIL] line
// per check.
namespace SelfTest {
    // Returns the number of failed checks (0 = everything passed).
    int runAll();
}
