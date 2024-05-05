#include"inputManager.h"

float GlobalScrollCallbackVal;
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
	GlobalScrollCallbackVal += (float)yoffset * 0.1f;
}

InputManager::InputManager() {
	p_window = nullptr;
	setCursorLastClickPos();
}

void InputManager::init(GLFWwindow* w) {
	p_window = w;

	keys[ZOOM_IN_KEY]		.init(w, keyBinds.zoomIn,		INPUT_TYPE_KEY);
	keys[ZOOM_OUT_KEY]		.init(w, keyBinds.zoomOut,		INPUT_TYPE_KEY);
	keys[MOVE_LEFT_KEY]		.init(w, keyBinds.moveLeft,		INPUT_TYPE_KEY);
	keys[MOVE_RIGHT_KEY]	.init(w, keyBinds.moveRight,	INPUT_TYPE_KEY);
	keys[MOVE_FORTH_KEY]	.init(w, keyBinds.moveForth,	INPUT_TYPE_KEY);
	keys[MOVE_BACK_KEY]		.init(w, keyBinds.moveBack,		INPUT_TYPE_KEY);
	keys[MOVE_UP_KEY]		.init(w, keyBinds.moveUp,		INPUT_TYPE_KEY);
	keys[MOVE_DOWN_KEY]		.init(w, keyBinds.moveDown,		INPUT_TYPE_KEY);
	keys[PITCH_DOWN_KEY]	.init(w, keyBinds.pitchDown,	INPUT_TYPE_KEY);
	keys[PITCH_UP_KEY]		.init(w, keyBinds.pitchUp,		INPUT_TYPE_KEY);
	keys[YAW_RIGHT_KEY]		.init(w, keyBinds.yawRight,		INPUT_TYPE_KEY);
	keys[YAW_LEFT_KEY]		.init(w, keyBinds.yawLeft,		INPUT_TYPE_KEY);
	keys[ROLL_LEFT_KEY]		.init(w, keyBinds.rollLeft,		INPUT_TYPE_KEY);
	keys[ROLL_RIGHT_KEY]	.init(w, keyBinds.rollRight,	INPUT_TYPE_KEY);
	mouseButtons[LEFT_CLICK_MOUSE_BUTTON].init(w, keyBinds.leftClick, INPUT_TYPE_MOUSE);
	mouseButtons[RIGHT_CLICK_MOUSE_BUTTON].init(w, keyBinds.rightClick, INPUT_TYPE_MOUSE);
}

void InputManager::update() {
	updateViaKeyboard();
	updateViaMouse();
}

void InputManager::updateViaKeyboard() {
	if (glfwGetKey(p_window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(p_window, true);
	}

	for (InputLatch& key : keys) {
		key.update();
	}
}

void InputManager::updateViaMouse() {
	for (InputLatch& mouseButton : mouseButtons) {
		mouseButton.update();
	}
	if (mouseButtons[LEFT_CLICK_MOUSE_BUTTON].click || mouseButtons[RIGHT_CLICK_MOUSE_BUTTON].click) {
		setCursorLastClickPos();
	}
	distFromLastClickCursorPixelPos = CursorPixelPos - lastClickCursorPixelPos;
}