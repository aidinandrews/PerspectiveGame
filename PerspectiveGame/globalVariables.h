#pragma once
#include <iostream>

#include <array>
#include <vector>
#define _USE_MATH_DEFINES
#include <math.h>
#include <chrono>

#include"dependancyHeaders.h"

#define RUN_DEBUG true

extern glm::ivec2 WindowSize;
extern glm::ivec2 MonitorSize;
extern glm::ivec2 WindowPos;
extern glm::dvec2 CursorPixelPos;
extern glm::dvec2 CursorScreenPos;

extern std::chrono::steady_clock::time_point ProgramStart;
extern std::chrono::steady_clock::time_point FrameStart;
extern float DeltaTime;
extern float TimeSinceProgramStart;
extern float UpdateTime;
extern float LastUpdateTime;

extern int PixelsPerGuiGridUnit;

extern float FPS;
extern float FrameTime;
extern int CurrentFrame;
extern int CurrentTick;

extern float guiEdit1;
extern float guiEdit2;
extern float guiEdit4;
extern float guiEdit3;

extern bool CanEditSubWindows;


extern int TICKS_IN_SCENARIO;
extern int CURRENT_SCENARIO_ID;
extern int NUM_OF_SCENARIOS;

inline void updateTimeSinceProgramStart()
{
	auto currentTime = std::chrono::high_resolution_clock::now();
	TimeSinceProgramStart = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - ProgramStart).count();
}

inline void updateGlobalVariables(GLFWwindow* window) {
	glfwGetWindowSize(window, &WindowSize.x, &WindowSize.y);
	glfwGetWindowPos(window, &WindowPos.x, &WindowPos.y);
	glfwGetCursorPos(window, &CursorPixelPos.x, &CursorPixelPos.y);
	CursorPixelPos.y = -(CursorPixelPos.y - WindowSize.y); // <- invert the y to make bottom left (0, 0).
	// Make sure to convert from glfw to OpenGL expectations:
	CursorScreenPos.x = -(CursorPixelPos.x / WindowSize.x * 2 - 1);
	CursorScreenPos.y = -(CursorPixelPos.y / WindowSize.y * 2 - 1);

	auto currentTime = std::chrono::high_resolution_clock::now();
	DeltaTime = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - FrameStart).count();
	updateTimeSinceProgramStart();
	if (DeltaTime < 16.0f / 1000.0f) {
		/*Sleep(16.0f - DeltaTime / 1000.0f);
		DeltaTime = 16.0f / 1000.0f;
		currentTime = std::chrono::high_resolution_clock::now();*/
	}
	FrameStart = currentTime;
}

inline void initGlobalVariables(GLFWwindow* window) {
	const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	MonitorSize.x = mode->width;
	MonitorSize.y = mode->height;
	WindowSize = glm::ivec2(600, 600);

	DeltaTime = 0.0f;
	ProgramStart = std::chrono::high_resolution_clock::now();
	FrameStart = ProgramStart;

	glfwSetWindowPos(window, MonitorSize.x / 2 - WindowSize.x / 2, MonitorSize.y / 2 - WindowSize.y / 2);
	updateGlobalVariables(window);
}

inline glm::vec2 guiGridUnitSpaceToWindowSpace(glm::ivec2 gridPos) {
	return glm::vec2(
		(float(gridPos.x * PixelsPerGuiGridUnit) / float(WindowSize.x) * 2) - 1,
		(float(gridPos.y * PixelsPerGuiGridUnit) / float(WindowSize.y) * 2) - 1);
}

inline glm::vec2 pixelSpaceToWindowSpace(glm::ivec2 pixelPos) {
	return glm::vec2(
		(float(pixelPos.x) / float(WindowSize.x) * 2.0f) - 1.0f,
		(float(pixelPos.y) / float(WindowSize.y) * 2.0f) - 1.0f);
}