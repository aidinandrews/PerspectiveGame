#pragma once
#include<iostream>
#include<GLFW/glfw3.h>

#include"globalVariables.h"

extern float GlobalScrollCallbackVal;
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

struct KeyBindings {
	const int zoomIn = GLFW_KEY_EQUAL;
	const int zoomOut = GLFW_KEY_MINUS;

	const int moveLeft = GLFW_KEY_A;
	const int moveRight = GLFW_KEY_D;
	const int moveForth = GLFW_KEY_W;
	const int moveBack = GLFW_KEY_S;
	const int moveUp = GLFW_KEY_SPACE;
	const int moveDown = GLFW_KEY_LEFT_SHIFT;

	const int pitchUp = GLFW_KEY_UP;
	const int pitchDown = GLFW_KEY_DOWN;
	const int yawRight = GLFW_KEY_Q;
	const int yawLeft = GLFW_KEY_E;
	const int rollLeft = GLFW_KEY_LEFT;
	const int rollRight = GLFW_KEY_RIGHT;

	const int leftClick = GLFW_MOUSE_BUTTON_LEFT;
	const int rightClick = GLFW_MOUSE_BUTTON_RIGHT;

	const int rotation = GLFW_MOUSE_BUTTON_RIGHT;
	const int translation = GLFW_MOUSE_BUTTON_LEFT;

	const int rotate = GLFW_KEY_R;
};

#define INPUT_TYPE_KEY		0
#define INPUT_TYPE_MOUSE	1

#define ZOOM_IN_KEY		0
#define ZOOM_OUT_KEY	1

#define MOVE_LEFT_KEY	2
#define MOVE_RIGHT_KEY	3
#define MOVE_FORTH_KEY	4
#define MOVE_BACK_KEY	5
#define MOVE_UP_KEY		6
#define MOVE_DOWN_KEY	7
#define PITCH_DOWN_KEY	8
#define PITCH_UP_KEY	9
#define YAW_RIGHT_KEY	10
#define YAW_LEFT_KEY	11
#define ROLL_LEFT_KEY	12
#define ROLL_RIGHT_KEY	13
#define ROTATE_KEY		14

#define LEFT_CLICK_MOUSE_BUTTON		0
#define RIGHT_CLICK_MOUSE_BUTTON	1

struct InputLatch {
	int inputType;
	int inputID;
	bool click;
	bool pressed;
	bool released;
	bool justReleased;

	GLFWwindow* window;
	InputLatch() : window(nullptr), inputID(0), inputType(0), click(false), pressed(false) {}

	void init(GLFWwindow* w, int GLFW_INPUT_ID, int INPUT_TYPE) {
		window = w;
		inputID = GLFW_INPUT_ID;
		inputType = INPUT_TYPE;
	}
	void update() {
		click = false;
		justReleased = false;

		switch (inputType) {
		case INPUT_TYPE_KEY:
			if (glfwGetKey(window, inputID) && !pressed) {
				click = true;
				pressed = true;
			}
			if (glfwGetKey(window, inputID) == GLFW_RELEASE) {
				justReleased = pressed;
				pressed = false;
			}
			released = !pressed;
			return;
		case INPUT_TYPE_MOUSE:
			if (glfwGetMouseButton(window, inputID) && !pressed) {
				click = true;
				pressed = true;
			}
			if (glfwGetMouseButton(window, inputID) == GLFW_RELEASE) {
				justReleased = pressed;
				pressed = false;
			}
			released = !pressed;
			return;
		}
	}
};

struct InputManager {
	GLFWwindow* p_window;
	KeyBindings keyBinds;

	InputLatch keys[15];
	InputLatch mouseButtons[2];

	glm::dvec2 lastClickCursorPixelPos;
	glm::dvec2 distFromLastClickCursorPixelPos;

	InputManager();
	void init(GLFWwindow* w);
	
	void setCursorLastClickPos() { lastClickCursorPixelPos = CursorPixelPos; }
	glm::vec2 getDistFromLastCursorClick() { return glm::vec2(CursorPixelPos - lastClickCursorPixelPos); }
	void updateViaKeyboard();
	void updateViaMouse();
	void update();

	bool leftMouseButtonPressed() { return mouseButtons[LEFT_CLICK_MOUSE_BUTTON].pressed; }
	bool leftMouseButtonReleased() { return mouseButtons[LEFT_CLICK_MOUSE_BUTTON].released; }
	bool rightMouseButtonPressed() { return mouseButtons[RIGHT_CLICK_MOUSE_BUTTON].pressed; }
	bool rightMouseButtonClicked() { return mouseButtons[RIGHT_CLICK_MOUSE_BUTTON].click; }
	bool rightMouseButtonReleased() { return mouseButtons[RIGHT_CLICK_MOUSE_BUTTON].released; }
};
