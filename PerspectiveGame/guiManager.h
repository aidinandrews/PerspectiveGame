// Dear ImGui: standalone example application for GLFW + OpenGL 3, using programmable pipeline
// (GLFW is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan/Metal graphics context creation, etc.)

// Learn about Dear ImGui:
// - FAQ                  https://dearimgui.com/faq
// - Getting Started      https://dearimgui.com/getting-started
// - Documentation        https://dearimgui.com/docs (same as your local docs/ folder).
// - Introduction, links and more at the top of imgui.cpp

#define GUI_MANAGER_DEFINED
#define USE_GUI_WINDOW

#define NOMINMAX
#include<Windows.h>

#include"globalVariables.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <stdio.h>
#define GL_SILENCE_DEPRECATION
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <GLES2/gl2.h>
#endif

#include"dependancyHeaders.h"

#include "buttonManager.h"
#include "shaderManager.h"
#include "inputManager.h"
#include "vectorHelperFunctions.h"
#include "cameraManager.h"
#include "frameBuffer.h"
#include "currentSelection.h"
#include "tileNodeNetwork.h"
#include "entityManager.h"
#include "pov.h"

// [Win32] Our example includes a copy of glfw3.lib pre-compiled with VS2010 to maximize ease of testing and compatibility with old VS compilers.
// To link with VS2010-era libraries, VS2015+ requires linking with legacy_stdio_definitions.lib, which we do using this pragma.
// Your own project should not be affected, as you are likely to link with a newer binary of GLFW that is adequate for your version of Visual Studio.
#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

// This example can also compile and run with Emscripten! See 'Makefile.emscripten' for details.
#ifdef __EMSCRIPTEN__
#include "../libs/emscripten/emscripten_mainloop_stub.h"
#endif

enum RenderType2d3rdPerson {
	cpuCropping,
	gpu2dRayCasting,
	gpuViaNodeNetwork,
};

struct GuiManager {
public:
	int GUI_WINDOW_BORDER_EDGE_SIZE = 4;
	int GUI_WINDOW_BORDER_CORNER_SIZE = 8;
	glm::vec3 GUI_EDGE_COLOR;

	ShaderManager* p_shaderManager;
	GLFWwindow* p_window;
	GLFWwindow* p_imGuiWindow;
	InputManager* p_inputManager;
	Camera* p_camera;
	Framebuffer* p_framebuffer;
	ButtonManager* p_buttonManager;
	CurrentSelection* p_currentSelection;
	EntityManager* p_entityManager;
	//TileManager* p_tileManager;

	TileNodeNetwork* p_nodeNetwork;
	POV* p_pov;

	bool show_demo_window;
	bool show_another_window;
	ImVec4 clear_color;
	float f;
	ImGuiIO io;

	float frameTime;

	const RenderType2d3rdPerson renderType2d3rdPerson = gpuViaNodeNetwork;

public:
	void imGuiSetup();
	GuiManager(GLFWwindow* w,
			   GLFWwindow* imgw,
			   ShaderManager* sm,
			   InputManager* im,
			   Camera* c,
			   Framebuffer* fb,
			   ButtonManager* bm,
			   CurrentSelection* cs,
			   EntityManager* em,
			   TileNodeNetwork* nn,
			   POV* pov)
		: p_window(w)
		, p_imGuiWindow(imgw)
		, p_shaderManager(sm)
		, p_inputManager(im)
		, p_camera(c)
		, p_framebuffer(fb)
		, p_buttonManager(bm)
		, p_currentSelection(cs)
		, p_entityManager(em)
		, p_nodeNetwork(nn)
		, p_pov(pov)
	{

		imGuiSetup();

		GUI_EDGE_COLOR = glm::vec3(1, 0, 1);
	}
	
	~GuiManager();

	void setupFramebufferForButtonRender(int buttonIndex, GLuint textureID);

	void renderImGuiDebugWindows();

	void renderTargetButtonMovementElements();
	void draw2d3rdPersonViaNodeNetwork();
	void draw2d3rdPerson();
	void draw3d3rdPerson();
	void render();

	void bindSSBOs2d3rdPersonViaNodeNetwork();
	void bindUniforms2d3rdPersonViaNodeNetwork(Button* sceneView);

	void drawColoredRectFromPixelSpace(glm::ivec2 pos, glm::ivec2 size, glm::vec3 color);
	// Assumes screen space:
	void drawColoredRect(glm::vec2 corners[4], glm::vec3 color);

	void drawColoredTriFromPixelSpace(glm::ivec2 A, glm::ivec2 B, glm::ivec2 C, glm::vec3 color);
	// Assumes screen space:
	void drawColoredTri(glm::vec2 corners[3], glm::vec3 color);

	// True if B is 'inside' or 'between' A and C.
	bool vecInsideVecs(glm::vec2 A, glm::vec2 B, glm::vec2 C) { return (A.y * B.x - A.x * B.y) * (A.y * C.x - A.x * C.y) < 0; }
	
	void drawTilesSetup();

	void draw3Dview();

	void draw3DTile(Tile* info);

	void drawTilesCleanup()
	{
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}

	std::vector<glm::vec2> createNewDrawTileVerts(std::vector<glm::vec2>& parent,
												  glm::vec2 adj)
	{
		std::vector<glm::vec2> newDrawTile;
		for (int i = 0; i < parent.size(); i++) {
			newDrawTile.push_back(parent[i] + adj);
		}
		return newDrawTile;
	}
};