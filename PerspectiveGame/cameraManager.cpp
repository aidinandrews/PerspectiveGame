#include"cameraManager.h"

void Camera::update() {
	lastFrameCursorPos = cursorPlanePos;
	cursorPlanePos = CursorScreenPos;

	//allowMouseInput = !p_guiManager->io.WantCaptureMouse;
	if (allowZoomChange) { updateZoom(); }
	if (allowPitchChange) { updatePitch(); }
	if (allowYawChange) { updateYaw(); }
	if (allowRollChange) { updateRoll(); }
	if (allowPosChange) { updatePos(); }

	getProjectionMatrix();
}

void Camera::updateZoom() {
	float zoomSpeed = 3.0f;
	if (p_inputManager->keys[ZOOM_IN_KEY].pressed) { zoom -= zoomSpeed * DeltaTime; }
	if (p_inputManager->keys[ZOOM_OUT_KEY].pressed) { zoom += zoomSpeed * DeltaTime; }

	if (allowMouseInput) {
		zoom -= GlobalScrollCallbackVal;
	}
	GlobalScrollCallbackVal = 0.0f;
}

void Camera::updateYawViaChangeInCursorPixelPosX() {
	if (p_inputManager->mouseButtons[LEFT_CLICK_MOUSE_BUTTON].pressed) {
		float dist = ((float)CursorScreenPos.x - lastFrameCursorPos.x) * 0.005f;
		if (lastFrameCursorPos.y > WindowSize.y / 2.0f) {
			dist *= -1;
		}
		yaw += dist;
	}
}

void Camera::updateYawViaChangeInCursorPixelPosAngleFromWindowCenter() {
	if (p_inputManager->mouseButtons[LEFT_CLICK_MOUSE_BUTTON].pressed) {
		float radianA = (float)atan2(CursorPixelPos.y - (WindowSize.y / 2), CursorPixelPos.x - (WindowSize.x / 2));
		float radianB = (float)atan2(p_inputManager->lastClickCursorPixelPos.y - (WindowSize.y / 2), p_inputManager->lastClickCursorPixelPos.x - (WindowSize.x / 2));
		yaw -= radianB - radianA;
		p_inputManager->lastClickCursorPixelPos = CursorPixelPos;
	}
}

void Camera::updateYaw() {
	float yawSpeed = 3.0f;
	if (p_inputManager->keys[YAW_RIGHT_KEY].pressed) {
		yaw += yawSpeed * DeltaTime;
	}
	if (p_inputManager->keys[YAW_LEFT_KEY].pressed) {
		yaw -= yawSpeed * DeltaTime;
	}
	if (allowMouseInput) {
		if (allowPitchChange) {
			updateYawViaChangeInCursorPixelPosX();
		} else {
			updateYawViaChangeInCursorPixelPosAngleFromWindowCenter();
		}
	}
}

void Camera::updatePitch() {
	float pitchSpeed = 3.0f;
	if (p_inputManager->keys[PITCH_UP_KEY].pressed) {
		pitch += pitchSpeed * DeltaTime;
	}
	if (p_inputManager->keys[PITCH_DOWN_KEY].pressed) {
		pitch -= pitchSpeed * DeltaTime;
	}
	if (p_inputManager->mouseButtons[LEFT_CLICK_MOUSE_BUTTON].pressed && allowMouseInput) {
		pitch -= ((float)cursorPlanePos.y - lastFrameCursorPos.y) * 0.005f;
	}
}

void Camera::updateRoll() {
	float rollSpeed = 0.5f;
}

void Camera::updatePos() {
	if (p_inputManager->mouseButtons[RIGHT_CLICK_MOUSE_BUTTON].pressed && allowMouseInput) {
		viewPlanePos += glm::vec3(screenPosToWorldPos(cursorPlanePos) - screenPosToWorldPos(lastFrameCursorPos), 0);
	}

	float moveSpeed = 2.0f;
	glm::vec3 posAdj(0.0f, 0.0f, 0.0f);
	if (p_inputManager->keys[MOVE_LEFT_KEY].pressed) {
		posAdj -= glm::vec3(moveSpeed * DeltaTime, 0.0f, 0.0f);
	}
	if (p_inputManager->keys[MOVE_RIGHT_KEY].pressed) {
		posAdj += glm::vec3(moveSpeed * DeltaTime, 0.0f, 0.0f);
	}

	if (p_inputManager->keys[MOVE_BACK_KEY].pressed) {
		posAdj -= glm::vec3(0.0f, moveSpeed * DeltaTime, 0.0f);
	}
	if (p_inputManager->keys[MOVE_FORTH_KEY].pressed) {
		posAdj += glm::vec3(0.0f, moveSpeed * DeltaTime, 0.0f);
	}

	if (p_inputManager->keys[MOVE_UP_KEY].pressed) {
		posAdj -= glm::vec3(0.0f, 0.0f, moveSpeed * DeltaTime);
	}
	if (p_inputManager->keys[MOVE_DOWN_KEY].pressed) {
		posAdj += glm::vec3(0.0f, 0.0f, moveSpeed * DeltaTime);
	}
	posAdj = vechelp::rotate(posAdj, yaw);
	// Make the player move faster when zoomed out:
	//posAdj *= 1 / (float)pow(2, zoom);
	viewPlanePos += posAdj;
}

void Camera::getProjectionMatrix() {
	//println(viewPlanePos);

	translationMatrix = glm::translate(glm::mat4(1), -viewPlanePos);
	glm::mat4 rotateZ = glm::rotate(glm::mat4(1), yaw, glm::vec3(0.0f, 0.0f, 1.0f));
	glm::mat4 rotateX = glm::rotate(glm::mat4(1), -pitch, glm::vec3(1.0f, 0.0f, 0.0f));
	float z = (float)pow(2, zoom);
	glm::mat4 scale = glm::scale(glm::mat4(1), glm::vec3(z, z, z));
	modelMatrix = scale * rotateX * rotateZ * translationMatrix;

	glm::vec4 temp4 = glm::vec4(0, 0, 1, 1);
	temp4 = rotateZ * rotateX * temp4;
	viewVec = glm::vec3(temp4);
	viewVec = glm::normalize(viewVec);

	float camDist = 10;
	pos = viewVec;
	pos.x = viewVec.x * camDist;
	pos.y = -viewVec.y * camDist;
	pos.z = viewVec.z * camDist;
	pos += viewPlanePos;

	viewMatrix = glm::mat4(1);// glm::lookAt(glm::vec3(0.0f, 0.01f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));

	float windowAdj = float(WindowSize.x) / float(WindowSize.y);
	//windowAdj.y = p_inputManager->currentWindowRatio;
	projectionMatrix = glm::ortho(
		-1.0f * windowAdj, 1.0f * windowAdj,
		1.0f, -1.0f,
		-100.0f, 100.0f);

	transfMatrix = projectionMatrix * viewMatrix * modelMatrix;

	glm::mat4 inverseTranslationMatrix = glm::translate(glm::mat4(1), -viewPlanePos);
	rotateZ = glm::rotate(glm::mat4(1), yaw, glm::vec3(0.0f, 0.0f, 1.0f));
	rotateX = glm::rotate(glm::mat4(1), pitch, glm::vec3(1.0f, 0.0f, 0.0f));
	z = 1 / (float)pow(2, zoom);
	scale = glm::scale(glm::mat4(1), glm::vec3(windowAdj * z, z, z));
	inverseTransfMatrix = inverseTranslationMatrix * rotateZ * rotateX * scale;
}

void Camera::adjProjMatrixToSubWindow(glm::ivec2 subWindowSizeInPixels) {
	// Magic bullshit numbers that I had to graph to not even understand but make work:
	float superWindowRatio = float(WindowSize.x) / float(WindowSize.y);
	float left = -float(subWindowSizeInPixels.x) / float(WindowSize.x);
	float right = 2.0f + left;
	float top = float(subWindowSizeInPixels.y) / float(WindowSize.y);
	float bottom = top - 2.0f;
	projectionMatrix = glm::ortho(left * superWindowRatio, right * superWindowRatio,
								  -bottom, -top, 
								  -100.0f, 100.0f);
	//projectionMatrix = glm::ortho(leftEdit, rightEdit, bottomEdit, topEdit, -100.0f, 100.0f);
	//projectionMatrix = glm::ortho(-1.0f,1.0f,1.0f,-1.0f, -100.0f, 100.0f);
	transfMatrix = projectionMatrix * viewMatrix * modelMatrix;
}

glm::mat4 Camera::getProjectionMatrix(float windowWidth, float windowHeight) {
	glm::mat4 translationMatrix = glm::translate(glm::mat4(1), -viewPlanePos);
	glm::mat4 rotateZ = glm::rotate(glm::mat4(1), -yaw, glm::vec3(0.0f, 0.0f, 1.0f));
	glm::mat4 rotateX = glm::rotate(glm::mat4(1), -pitch, glm::vec3(1.0f, 0.0f, 0.0f));
	glm::mat4 modelMatrix = rotateX * rotateZ * translationMatrix;

	float z = (float)pow(2, zoom);
	// to keep from scaling at some transitions, the zoom needs an 'anchor' so that it can 
	// scale back to the same dimensions on view ratio changes. 300 is chosen here as the
	// original window is 600 x 600 and each input to glm::ortho wants the distance from the
	// center of the screen to the side.  300 == 600 / 2, thus 300 is the anchor.
	z *= (float)windowHeight / 600.0f;
	float windowAdj = windowWidth / windowHeight;
	float left = -z * windowAdj;
	float right = z * windowAdj;
	float bottom = z;
	float top = -z;
	glm::mat4 projectionMatrix = glm::ortho(left, right, bottom, top, -100.0f, 100.0f);

	return projectionMatrix * modelMatrix;
}

glm::mat4 Camera::getPerspectiveProjectionMatrix(float windowWidth, float windowHeight) {
	glm::vec3 t(viewPlanePos.x, viewPlanePos.y, viewPlanePos.z);
	glm::mat4 translationMatrix = glm::translate(glm::mat4(1), t);
	glm::mat4 rotateZ = glm::rotate(glm::mat4(1), -yaw, glm::vec3(0.0f, 0.0f, 1.0f));
	glm::mat4 rotateX = glm::rotate(glm::mat4(1), -pitch, glm::vec3(1.0f, 0.0f, 0.0f));
	glm::mat4 modelMatrix = rotateX * rotateZ * translationMatrix;

	float aspectEqualizer = windowHeight / 300.0f;
	float z = (float)pow(2, zoom);
	glm::mat4 viewMatrix = glm::lookAt(glm::vec3(0.0f, 0.01f, z * aspectEqualizer),
									   glm::vec3(0.0f, 0.0f, 0.0f), 
									   glm::vec3(0.0f, 0.0f, 1.0f));
	float aspect = windowWidth / windowHeight;
	glm::mat4 projectionMatrix = glm::perspective(45.0f, aspect, 0.1f, 1000.0f);

	return projectionMatrix * modelMatrix * viewMatrix;
}

glm::vec2 Camera::screenPosToWorldPos(glm::vec2 screenPos) {
	glm::mat4 translationMatrix = glm::translate(glm::mat4(1), -viewPlanePos);
	glm::mat4 rotateZ = glm::rotate(glm::mat4(1), yaw, glm::vec3(0.0f, 0.0f, 1.0f));
	glm::mat4 rotateX = glm::rotate(glm::mat4(1), pitch, glm::vec3(1.0f, 0.0f, 0.0f));
	float z = 1 / ((float)pow(2, zoom));
	glm::mat4 scale = glm::scale(glm::mat4(1), glm::vec3(z, z, z));
	glm::mat4 inverseMatrix = translationMatrix * rotateZ * rotateX * scale;

	return glm::vec2(inverseMatrix * glm::vec4(screenPos, 0, 1));
}