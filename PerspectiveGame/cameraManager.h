#pragma once
#include <iostream>
#include <vector>
#ifndef GLAD_INCLUDED
#include <glad/glad.h>
#endif
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define _USE_MATH_DEFINES
#include <math.h>
#include <chrono>

#include "buttonManager.h"
#include "inputManager.h"
#include "vectorHelperFunctions.h"

#define ORTHOGRAPHIC_PERSPECTIVE 0
#define ONE_POINT_PERSPECTIVE 1

struct Camera {
	glm::vec3 viewVec;
	glm::vec3 pos;
	glm::vec3 viewPlanePos, viewPlanePosStore, viewPlanePosAdj;
	glm::vec3 lastFrameViewPlanePos;

	glm::vec2 cursorLastClickPos;

	glm::vec2 cursorPlanePos;
	glm::mat4 translationMatrix;
	glm::mat4 transfMatrix, viewMatrix, projectionMatrix, modelMatrix;
	glm::mat4 inverseTransfMatrix;

	bool allowMouseInput;
	bool allowYawChange;
	bool allowPitchChange;
	bool allowRollChange;
	bool allowPosChange;
	bool allowZoomChange;
	float zoom, yaw, pitch, roll;
	float zoomLinear;

	Framebuffer *p_framebuffer;
private:
	InputManager *p_inputManager;
	GLFWwindow *p_window;

	glm::vec2 lastFrameCursorPos;
	int perspective = ORTHOGRAPHIC_PERSPECTIVE;

public:
	Camera() {
		p_inputManager = nullptr;
		p_window = nullptr;
		allowPosChange = true;
		allowZoomChange = true;
		allowYawChange = false;
		allowRollChange = true;
		allowPitchChange = true;
		viewPlanePos = glm::vec3(0.0f, 0.0f, 0.0f);
		lastFrameViewPlanePos = glm::vec3(0.0f, 0.0f, 0.0f);
		lastFrameCursorPos = glm::vec2(CursorPixelPos);
		viewVec = glm::vec3(0.0f, 0.0f, 1.0f);
		cursorPlanePos = glm::vec2(0.0f, 0.0f);
		viewPlanePos = glm::vec3(0.0f, 0.0f, 0.0f);
		viewPlanePosStore = glm::vec3(0.0f, 0.0f, 0.0f);
		viewPlanePosAdj = glm::vec3(0.0f, 0.0f, 0.0f);
		pitch = 0.0f;
		yaw = 0.0f;//3.0f * (float)M_PI / 2.0f;
		roll = 0.0f;
		zoom = 2.01f;
		transfMatrix = glm::mat4(1);
		inverseTransfMatrix = glm::mat4(1);
	}
	void init(GLFWwindow* window, InputManager& inputManager, Framebuffer& framebuffer) {
		p_window = window;
		p_inputManager = &inputManager;
		p_framebuffer = &framebuffer;
	}
	glm::vec3 getPos() { return pos; }
	float getZoom() { return zoom; }
	float getScaledZoom() { return (float)pow(2,zoom); }
	float getYaw() { return yaw; }
	float getPitch() { return pitch; }
	float getRoll() { return roll; }

	void updateZoom();
	void updateYaw();
	void updateYawViaChangeInCursorPixelPosX();
	void updateYawViaChangeInCursorPixelPosAngleFromWindowCenter();
	void updatePitch();
	void updateRoll();
	void updatePos();
	void update();
	
	void getProjectionMatrix();
	void adjProjMatrixToSubWindow(glm::ivec2 windowSizeInPixels);
	glm::mat4 getProjectionMatrix(float windowWidth, float windowHeight);
	glm::mat4 getPerspectiveProjectionMatrix(float windowWidth, float windowHeight);
	// Returns the XY world coordinates (Z assumed 0) of a screen coordinate.
	glm::vec2 screenPosToWorldPos(glm::vec2 screenPos);
};