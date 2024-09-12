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
float UpdateTime = 1.0/4.0;
float LastUpdateTime = 0.0f;

int PixelsPerGuiGridUnit = 60;

float FPS = 0;
float FrameTime = 0;
int CurrentFrame = 0;
int CurrentTick = 0;

float leftEdit = -1, rightEdit = 1, topEdit = 1, bottomEdit = -1;

bool CanEditSubWindows = false;

int TICKS_IN_SCENARIO = 0;
int CURRENT_SCENARIO_ID = 0;
int NUM_OF_SCENARIOS = 5;