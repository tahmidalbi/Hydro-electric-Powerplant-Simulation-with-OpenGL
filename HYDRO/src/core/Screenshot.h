// =============================================================================
//  Screenshot.h  -  saves what is on screen to a .bmp file (hand-written BMP
//  writer, no image library).  F12 in the program, or --shot on the command
//  line.
// =============================================================================
#pragma once
#include <string>

bool saveScreenshotBMP(const std::string& file, int width, int height);
