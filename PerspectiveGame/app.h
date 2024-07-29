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
#include "forceManager.h"
#include "currentSelection.h"
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
	ForceManager* p_forceManager;
	EntityManager* p_entityManager;
	CurrentSelection* p_currentSelection;
	BasisManager* p_basisManager;

	App() {}

	~App() {
		delete p_guiManager;
		delete p_wave;
		delete p_tileManager;
		delete p_entityManager;
		delete p_basisManager;
		delete p_currentSelection;

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

		p_forceManager = new ForceManager(p_tileManager);

		p_entityManager = new EntityManager(p_tileManager);

		p_basisManager = new BasisManager(p_tileManager, p_forceManager, p_entityManager);

		p_currentSelection = new CurrentSelection(&inputManager, p_tileManager, p_entityManager, p_buttonManager, &camera, p_basisManager);

#ifdef USE_GUI_WINDOW
		p_guiManager = new GuiManager(window.window, imGuiWindow.window, &shaderManager, &inputManager, &camera, 
			p_tileManager, &framebuffer, p_buttonManager, p_currentSelection);
#else
		p_guiManager = new GuiManager(window.window, nullptr, &shaderManager, &inputManager, &camera, p_tileManager, &framebuffer, p_buttonManager);
#endif
		glfwMakeContextCurrent(window.window);

		int bufferWidth, bufferHeight;
		glfwGetFramebufferSize(window.window, &bufferWidth, &bufferHeight);

		glViewport(0, 0, bufferWidth, bufferHeight);
		p_guiManager->io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

		srand((unsigned int)time(NULL));

		setupWorld();

		return true;
	}

	void setupWorld() {
		// This is the initial two tiles that must exist for the player to even move around at all:
		for (int w = 0; w < 4; w++) {
			for (int h = 0; h < 4; h++) {
				p_tileManager->createTilePair(Tile::TILE_TYPE_XY, glm::ivec3(w, h, 0), glm::vec3(0, 0, 1), glm::vec3(0, 0, 0.5));
			}
		}
		p_basisManager->createProducer(0, Entity::Type::MATERIAL_A, true);
	}

	void updateGraphicsAPI() {
		glfwPollEvents();
		glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}
	
	void run() {
		float runningFPS = 0;
		int counter = 0;
		int lastFPS = 0;

		while (!glfwWindowShouldClose(window.window)) {
			auto start = std::chrono::high_resolution_clock::now();

			updateGlobalVariables(window.window);

			inputManager.update();

			p_buttonManager->updateButtons();

			camera.update();

			p_currentSelection->update();

			p_tileManager->update();
			p_forceManager->update();
			p_basisManager->update();

#ifdef USE_GUI_WINDOW
			glfwMakeContextCurrent(window.window);
#endif
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
			float thisFrameTime = std::chrono::duration<float, std::chrono::milliseconds::period>(end - start).count();
			//std::cout << FrameTime << std::endl;
			Sleep((DWORD)std::max(16.0f - FrameTime, 0.0f));
			NumFrames++;

			counter++;
			runningFPS += (1000.0f / thisFrameTime);
			if (counter > 50) {
				FPS = (runningFPS / (float)counter);
				FrameTime = thisFrameTime;
				counter = 0;
				runningFPS = 0;
			}
		}
	}
};