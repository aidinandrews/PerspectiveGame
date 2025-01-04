#pragma once

//#define RUNNING_TEST_SCENARIOS
#define RUNNING_DEBUG

#include<iostream>
#include <string>
#include<iomanip>
#include <stdlib.h>
#include <time.h>

#include"dependancyHeaders.h"

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
//#include "tileManager.h"
#include "forceManager.h"
#include "currentSelection.h"
#include "frameBuffer.h"
#include "scenarioSetup.h"
#include "pov.h"

struct App {
	Window window;
	#ifdef USE_GUI_WINDOW
	Window imGuiWindow;
	#endif
	InputManager inputManager;
	ShaderManager shaderManager;
	GuiManager* p_guiManager;
	VertexManager vertManager;
	Camera camera;
	ButtonManager* p_buttonManager;

	Framebuffer framebuffer;
	aaTexture* p_wave;

	//TileManager* p_tileManager;
	//ForceManager* p_forceManager;
	EntityManager* p_entityManager;
	CurrentSelection* p_currentSelection;
	BasisManager* p_basisManager;
	
	TileNodeNetwork* p_nodeNetwork;
	POV* p_pov;

	App() {}

	~App()
	{
		delete p_guiManager;
		delete p_wave;
		//delete p_tileManager;
		//delete p_entityManager;
		//delete p_basisManager;
		delete p_currentSelection;
		delete p_nodeNetwork;
		delete p_pov;

		glfwTerminate();
	}

	bool init()
	{
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

		//p_tileManager = new TileManager(&camera, &shaderManager, window.window, &framebuffer, p_buttonManager, &inputManager, nullptr);
		//p_tileManager->texID = p_wave->ID;
		
		p_nodeNetwork = new TileNodeNetwork(&camera);
		p_nodeNetwork->texID = p_wave->ID;

		p_pov = new POV(p_nodeNetwork, &camera, &p_buttonManager->buttons[ButtonManager::pov3d3rdPersonViewButtonIndex]);

		//p_forceManager = new ForceManager(p_tileManager);

		//p_entityManager = new EntityManager(p_tileManager);

		//p_basisManager = new BasisManager(p_tileManager, p_forceManager, p_entityManager);

		p_currentSelection = new CurrentSelection(&inputManager, p_entityManager, p_buttonManager, 
												  &camera, p_basisManager, p_nodeNetwork, p_pov);

		#ifdef USE_GUI_WINDOW
		p_guiManager = new GuiManager(window.window, imGuiWindow.window, &shaderManager, &inputManager, &camera,
									  &framebuffer, p_buttonManager, p_currentSelection, p_entityManager, 
									  p_nodeNetwork, p_pov);
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

	void setupWorld()
	{
		//setupScenarioDirectCollisionFromEdge(p_tileManager, p_entityManager, p_currentSelection);
	}

	void updateGraphicsAPI()
	{
		glfwPollEvents();
		glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void updateWorld()
	{
		//p_tileManager->update();
		p_pov->update();
		p_nodeNetwork->update();
		p_currentSelection->tryEditWorld();

		if ((TimeSinceProgramStart - LastUpdateTime) > UpdateTime) {
			LastUpdateTime = TimeSinceProgramStart;

			if (CurrentTick % 4 == 0) {
				//p_currentSelection->tryEditWorld();
				//p_currentSelection->addQueuedEntities();
				//p_forceManager->update();
				//p_basisManager->update();
			}

			//p_entityManager->update();
			//p_tileManager->updateTileGpuInfos();
			//p_entityManager->updateGpuInfos();

			CurrentTick++;

			#ifdef RUNNING_TEST_SCENARIOS
			TICKS_IN_SCENARIO++;
			#endif
		}
	}

	void updateGui()
	{
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
	}

	void run()
	{
		int counter = 0;
		int lastFPS = 0;
		float runningFPS = 0;
		float lastUpdateTime = 0;
		CurrentFrame = 0;
		CurrentTick = 0;

		while (!glfwWindowShouldClose(window.window)) {
			auto start = std::chrono::high_resolution_clock::now();

			#ifdef RUNNING_TEST_SCENARIOS
			if (CURRENT_SCENARIO_ID == 0 || TICKS_IN_SCENARIO > ticksPerScenario(CURRENT_SCENARIO_ID)) {
				setupTestScenario(CURRENT_SCENARIO_ID % NUM_OF_SCENARIOS, p_tileManager, p_entityManager, p_currentSelection);
				CURRENT_SCENARIO_ID++;
				TICKS_IN_SCENARIO = 0;
			}
			#endif

			updateGlobalVariables(window.window);
			inputManager.update();
			p_buttonManager->updateButtons();
			camera.update();
			
			p_currentSelection->update();
			updateWorld();
			//p_tileManager->updateVisualInfos();

			updateGui();

			auto end = std::chrono::high_resolution_clock::now();
			float thisFrameTime = std::chrono::duration<float, std::chrono::milliseconds::period>(end - start).count();
			//std::cout << FrameTime << std::endl;
			Sleep((DWORD)std::max(16.0f - FrameTime, 0.0f));
			CurrentFrame++;

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