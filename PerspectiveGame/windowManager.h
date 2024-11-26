#pragma once
#include<iostream>

#include"dependancyHeaders.h"

#include"globalVariables.h"

extern int WINDOW_WIDTH;
extern int WINDOW_HEIGHT;

#define FAIL_TO_CREATE_WINDOW -1

struct Window {
	GLFWwindow* window;
	GLuint FramebufferID;

	Window();
	~Window() {
		glfwDestroyWindow(window);
	}

	void init(const char* title);

	void bindAsRenderTarget() {
		glBindFramebuffer(GL_FRAMEBUFFER, FramebufferID);
		glViewport(0, 0, WindowSize.x, WindowSize.y);
	}
};

void initWindow(GLFWwindow **window, const char *windowName);