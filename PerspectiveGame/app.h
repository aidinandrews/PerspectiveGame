#pragma once
#include<iostream>
#include <string>
#include<iomanip>
#include <stdlib.h>
#include <time.h>
#define NOMINMAX
#define USING_WINDOWS
#include "Windows.h"
#define GLAD_INCLUDED
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "globalVariables.h"
#define GUI_MANAGER_DEFINED
#include "cameraManager.h"
#include "guiManager.h"
#include "windowManager.h"
#include "inputManager.h"
#include "shaderManager.h"
#include "vertexManager.h"
#include "textureManager.h"
#include "makeShapes.h"
#include "portal.h"
#include "scene.h"
#include "tileManager.h"
#include "frameBuffer.h"

struct App {
	Window window;
#ifdef USE_GUI_WINDOW
	Window imGuiWindow;
#endif
	InputManager inputManager;
	ShaderManager shaderManager;
	GuiManager *p_guiManager;
	VertexManager vertManager;
	Camera camera;
	ButtonManager* p_buttonManager;
	
	Framebuffer framebuffer;
	aaTexture *p_wave;

	TileManager *p_tileManager;

	App() {}

	~App() {
		delete p_guiManager;
		delete p_wave;
		delete p_tileManager;

		glfwTerminate();
	}

	bool init() {
		if (!glfwInit()) {
			std::cout << "GLFW initialisation failed!\n";
			glfwTerminate();
			return false;
		}
#ifdef USE_GUI_WINDOW
		imGuiWindow.init("Debug");
#endif
		window.init("Tiles In 3D");

		glfwMakeContextCurrent(window.window);

		ImGui::SetCurrentContext(ImGui::GetCurrentContext());

		framebuffer.init();

		initGlobalVariables(window.window);
		glfwSetScrollCallback(window.window, scroll_callback);

		inputManager.init(window.window);
		shaderManager.init();
		vertManager.init(&shaderManager);
		camera.init(window.window, inputManager, framebuffer);
		camera.allowPitchChange = false; // <- 2D enviornment needs no pitch change.

		p_wave = new aaTexture("textures/TheGreatWave.jpg", GL_TEXTURE_2D, GL_TEXTURE0, GL_RGBA, GL_UNSIGNED_BYTE);
		p_wave->texUnit(shaderManager.simpleShader, "tex0", 0);
		shaderManager.simpleShader.setUniformIndex("greatWave", 0);
		shaderManager.texIDs.push_back(p_wave->ID);

		p_buttonManager = new ButtonManager(&framebuffer, &shaderManager, window.window, &inputManager);

		p_tileManager = new TileManager(&camera, &shaderManager, window.window, &framebuffer, p_buttonManager, &inputManager);
		p_tileManager->texID = p_wave->ID;

#ifdef USE_GUI_WINDOW
		p_guiManager = new GuiManager(window.window, imGuiWindow.window, &shaderManager, &inputManager, &camera, p_tileManager, &framebuffer, p_buttonManager);
#else
		p_guiManager = new GuiManager(window.window, nullptr, &shaderManager, &inputManager, &camera, p_tileManager, &framebuffer, p_buttonManager);
#endif
		glfwMakeContextCurrent(window.window);

		int bufferWidth, bufferHeight;
		glfwGetFramebufferSize(window.window, &bufferWidth, &bufferHeight);

		glViewport(0, 0, bufferWidth, bufferHeight);
		p_guiManager->io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

		srand((unsigned int)time(NULL));

		return true;
	}

	void updateGraphicsAPI() {
		glfwPollEvents();
		glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}
	
	void run() {
		
		while (!glfwWindowShouldClose(window.window)) {
			auto start = std::chrono::high_resolution_clock::now();

			updateGlobalVariables(window.window);
			inputManager.update();
			p_buttonManager->updateButtons();
			camera.update();
			p_tileManager->update();

			glfwMakeContextCurrent(window.window);
			updateGraphicsAPI();
			p_guiManager->render();
			glfwSwapBuffers(window.window);
			
#ifdef USE_GUI_WINDOW
			glfwMakeContextCurrent(imGuiWindow.window);
			updateGraphicsAPI();
			p_guiManager->renderImGuiDebugWindows();
			glfwSwapBuffers(imGuiWindow.window);
#endif
			
			auto end = std::chrono::high_resolution_clock::now();
			FrameTime = std::chrono::duration<float, std::chrono::milliseconds::period>(end - start).count();
			//std::cout << FrameTime << std::endl;
			Sleep((DWORD)std::max(16.0f - FrameTime, 0.0f));
			FPS = 1000.0f / FrameTime;
		}
	}
};