#include "globalVariables.h"

glm::ivec2 WindowSize = glm::ivec2(600, 600);
glm::ivec2 MonitorSize;
glm::ivec2 WindowPos;
glm::dvec2 CursorPixelPos;
glm::dvec2 CursorScreenPos;

std::chrono::steady_clock::time_point ProgramStart;
std::chrono::steady_clock::time_point FrameStart;
float DeltaTime;
float TimeSinceProgramStart;

int PixelsPerGuiGridUnit = 60;

float FPS = 0;
float FrameTime = 0;

float leftEdit = -1, rightEdit = 1, topEdit = 1, bottomEdit = -1;

bool CanEditTiles = true;