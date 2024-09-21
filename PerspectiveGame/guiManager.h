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

#ifndef GLAD_INCLUDED
#include <glad/glad.h>
#endif

#include"globalVariables.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <stdio.h>
#define GL_SILENCE_DEPRECATION
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <GLES2/gl2.h>
#endif
#include <GLFW/glfw3.h> // Will drag system OpenGL headers

#include "buttonManager.h"
#include "shaderManager.h"
#include "inputManager.h"
#include "vectorHelperFunctions.h"
#include "cameraManager.h"
#include "tileManager.h"
#include "frameBuffer.h"
#include "currentSelection.h"
#include "tileNavigation.h"

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
	TileManager* p_tileManager;
	Framebuffer* p_framebuffer;
	ButtonManager* p_buttonManager;
	CurrentSelection* p_currentSelection;
	EntityManager* p_entityManager;

	bool show_demo_window;
	bool show_another_window;
	ImVec4 clear_color;
	float f;
	ImGuiIO io;

	float frameTime;

	const RenderType2d3rdPerson renderType2d3rdPerson = gpu2dRayCasting;

public:
	void imGuiSetup();
	GuiManager(GLFWwindow* w, GLFWwindow* imgw, ShaderManager* sm, InputManager* im, Camera* c, TileManager* tm, Framebuffer* fb,
			   ButtonManager* bm, CurrentSelection* cs, EntityManager* em);
	~GuiManager();

	void setupFramebufferForButtonRender(int buttonIndex, GLuint textureID);

	void renderImGuiDebugWindows();

	void renderTargetButtonMovementElements();
	void draw2d3rdPersonCpuCropping();
	void draw2d3rdPersonGpuRaycasting();
	void draw2d3rdPerson();
	void draw3d3rdPerson();
	void render();

	void bindSSBOs2d3rdPerson();
	void bindUniforms2d3rdPerson();

	void drawColoredRectFromPixelSpace(glm::ivec2 pos, glm::ivec2 size, glm::vec3 color);
	// Assumes screen space:
	void drawColoredRect(glm::vec2 corners[4], glm::vec3 color);

	void drawColoredTriFromPixelSpace(glm::ivec2 A, glm::ivec2 B, glm::ivec2 C, glm::vec3 color);
	// Assumes screen space:
	void drawColoredTri(glm::vec2 corners[3], glm::vec3 color);


	// MOVED TILE MANAGER FUNCTIONS:

	// True if B is 'inside' or 'between' A and C.
	bool vecInsideVecs(glm::vec2 A, glm::vec2 B, glm::vec2 C)
	{
		return (A.y * B.x - A.x * B.y) * (A.y * C.x - A.x * C.y) < 0;
	}

	bool tileOnScreen(std::vector<glm::vec2>& tileVerts)
	{
		using namespace vechelp;

		for (glm::vec2 v : tileVerts) {
			if (point_in_polygon(v, p_tileManager->windowFrustum)) {
				return true;
			}
		}
		for (int wfi = 0; wfi < 4; wfi++) {
			for (int ti = 0; ti < tileVerts.size(); ti++) {
				if (doIntersect(tileVerts[ti], tileVerts[(ti + 1) % 4],
								p_tileManager->windowFrustum[wfi], p_tileManager->windowFrustum[(wfi + 1) % 4])) {
					return true;
				}
			}
		}
		return false;
	}
	
	void drawTilesSetup()
	{
		glDisable(GL_CULL_FACE);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_STENCIL_TEST);
		glDisable(GL_BLEND);

		glBindVertexArray(p_framebuffer->VAO);
		glBindBuffer(GL_ARRAY_BUFFER, p_framebuffer->VBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, p_framebuffer->EBO);

		setVertAttribVec3PosVec3NormVec3ColorVec2TextCoord1Index();
		p_shaderManager->POV3D3rdPerson.use();
	}

	// Draws the 2D scene from a 3rd person perspective.
	void draw2D3rdPerson()
	{
		glViewport(0, 0, WindowSize.x, WindowSize.y);
		drawTilesSetup();

		const float INITIAL_OPACITY = 0.5f;
		bool previousSides[4] = { 0, 0, 0, 0 };

		// Draw the povTile itself to start:
		std::vector<glm::vec2> croppedDrawTileTexCoords = {
			p_tileManager->povTile.tile->texCoords[(p_tileManager->povTile.initialVertIndex + p_tileManager->povTile.sideInfosOffset * 0) % 4],
			p_tileManager->povTile.tile->texCoords[(p_tileManager->povTile.initialVertIndex + p_tileManager->povTile.sideInfosOffset * 1) % 4],
			p_tileManager->povTile.tile->texCoords[(p_tileManager->povTile.initialVertIndex + p_tileManager->povTile.sideInfosOffset * 2) % 4],
			p_tileManager->povTile.tile->texCoords[(p_tileManager->povTile.initialVertIndex + p_tileManager->povTile.sideInfosOffset * 3) % 4],
		};
		std::vector<glm::vec2> povTileDrawVerts = {
			glm::vec2(1, 1),glm::vec2(1, 0),glm::vec2(0, 0),glm::vec2(0, 1)
		};
		drawTile(TileManager::TileManager::INITIAL_DRAW_TILE_VERTS, croppedDrawTileTexCoords, glm::vec4(p_tileManager->povTile.tile->color, INITIAL_OPACITY));

		// Start the recursive call to draw each tile connected to the eye tile edges:
		for (int drawTileSideIndex = 0; drawTileSideIndex < 4; drawTileSideIndex++) {
			glm::vec2 newFrustum[3] = {
				glm::normalize(TileManager::TileManager::INITIAL_DRAW_TILE_VERTS[(drawTileSideIndex) % 4]
							   - glm::vec2(p_camera->viewPlanePos)),
				glm::vec2(0, 0),
				glm::normalize(TileManager::TileManager::INITIAL_DRAW_TILE_VERTS[(drawTileSideIndex + 1) % 4]
							   - glm::vec2(p_camera->viewPlanePos)),
			};

			// After a draw tile moves in a direction, it should never need to move back 
			// in that direction again, thus we can make sure it doesnt with this bool array:
			bool newPreviousSides[4]{};
			newPreviousSides[(drawTileSideIndex + 2) % 4] = true;

			int newSideOffset, newInitialSideIndex, newInitialTexIndex;
			int sideIndex = (p_tileManager->povTile.initialSideIndex + p_tileManager->povTile.sideInfosOffset * drawTileSideIndex) % 4;

			// The next draw tile can be either mirrored or unmirrored.  Mirrored tiles are 
			// wound the opposite way and thus have an opposite side index offset.  Unmirrored 
			// tiles have the same winding, so no adjustment is necessary.  As the side index 
			// is in the domain of [0,3], we can also just wrap around from 3 -> 0 with % 4.
			if (p_tileManager->povTile.tile->is1stDegreeNeighborMirrored(sideIndex)) {
				newSideOffset = (p_tileManager->povTile.sideInfosOffset + 2) % 4;
			}
			else { // Current connection is unmirrored:
				newSideOffset = p_tileManager->povTile.sideInfosOffset;
			}
			newInitialSideIndex = (p_tileManager->povTile.tile->get1stDegreeNeighborConnectedSideIndex(LocalDirection(sideIndex))
								   + TileManager::VERT_INFO_OFFSETS[drawTileSideIndex] * newSideOffset) % 4;
			newInitialTexIndex = newInitialSideIndex;
			if (newSideOffset == 3) {
				// then the next tile will be wound counterclockwise, and it's initial side 
				// index will key into the *top right* tex coord instead of the top left.  
				// Because the winding is counterclockwise, we can adjust the initial tex 
				// coord by incrementing it once, going from the top right to the top left!
				newInitialTexIndex = (newInitialTexIndex + 1) % 4;
			}
			std::vector<glm::vec2> newTileVerts
				= createNewDrawTileVerts(TileManager::INITIAL_DRAW_TILE_VERTS, TileManager::DRAW_TILE_OFFSETS[drawTileSideIndex]);

			// Tiles that change angle will change opacity or 'tint' so that it 
			// can be noticed with traversing 3D space from this perspective:
			float newTileOpacity;
			// We want a smooth transition from one tile opacity to another, so
			// it should fade as you get closer to the next draw tile's edge:
			float edgeDist = vechelp::distToLineSeg((glm::vec2)p_camera->viewPlanePos,
										   TileManager::INITIAL_DRAW_TILE_VERTS[drawTileSideIndex],
										   TileManager::INITIAL_DRAW_TILE_VERTS[(drawTileSideIndex + 1) % 4],
										   nullptr);
			newTileOpacity = INITIAL_OPACITY;
			if (p_tileManager->povTile.tile->neighbors[sideIndex]->type != p_tileManager->povTile.tile->type) {
				newTileOpacity -= TileManager::DRAW_TILE_OPACITY_DECRIMENT_STEP;
			}
			if (edgeDist < 0.5f && p_tileManager->povTile.tile->neighbors[sideIndex]->type != p_tileManager->povTile.tile->type) {
				newTileOpacity += (-((edgeDist * 2) - 1)) * 0.1f;
			}

			// Finally!  We can actually go onto drawing the next tile:
			drawTiles(p_tileManager->povTile.tile->neighbors[sideIndex], newTileVerts,
					  newInitialSideIndex, newInitialTexIndex, newSideOffset,
					  newFrustum, newPreviousSides, newTileOpacity);
			//break;
		}


		/*auto end = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration<float, std::chrono::milliseconds::period>(end - start).count();
		TOTAL_TIME += time;*/
		//std::cout << TOTAL_TIME << std::endl;

		drawTilesCleanup();
	}

	void drawPlayerPos()
	{
		drawTilesSetup();
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glPolygonMode(GL_FRONT, GL_FILL);

		// Temp player location:
		std::vector<glm::vec2> v = {
			glm::vec2(-0.1,-0.1),
			glm::vec2(+0.1,-0.1),
			glm::vec2(+0.1,+0.1),
			glm::vec2(-0.1,+0.1)
		};
		for (glm::vec2& vert : v) { vert += (glm::vec2)p_camera->viewPlanePos; }
		std::vector<glm::vec2> t = {
			glm::vec2(0,0),
			glm::vec2(1,0),
			glm::vec2(1,1),
			glm::vec2(0,1)
		};

		drawTile(v, t, glm::vec4(1, 1, 1, 1));
	}

	void draw3Dview()
	{
		drawTilesSetup();
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glPolygonMode(GL_FRONT, GL_FILL);
		glEnable(GL_CULL_FACE);

		Button* button = &p_buttonManager->buttons[ButtonManager::pov3d3rdPersonViewButtonIndex];
		glm::mat4 tempMat = p_camera->getPerspectiveProjectionMatrix((float)button->pixelWidth(),
														   (float)button->pixelHeight());

		glm::mat4 xMirror(1);
		xMirror[0][0] = -1;
		tempMat = xMirror * tempMat * p_tileManager->tileRotationAdjFor3DView;
		GLuint transfMatrixID = glGetUniformLocation(p_shaderManager->POV3D3rdPerson.ID, "inTransfMatrix");
		glUniformMatrix4fv(transfMatrixID, 1, GL_FALSE, glm::value_ptr(tempMat));

		GLuint playerPosInfoID = glGetUniformLocation(p_shaderManager->POV3D3rdPerson.ID, "inPlayerPosInfo");
		glUniformMatrix4fv(playerPosInfoID, 1, GL_FALSE, glm::value_ptr(packedPlayerPosInfo()));

		glm::vec3 playerPos = p_tileManager->getPovTilePos();
		playerPos = glm::vec3(tempMat * glm::vec4(playerPos, 1));
		GLuint playerPosID = glGetUniformLocation(p_shaderManager->POV3D3rdPerson.ID, "inPlayerPos");
		glUniform3f(playerPosID, playerPos.x, playerPos.y, playerPos.z);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, p_tileManager->texID);

		glDrawBuffer(GL_COLOR_ATTACHMENT0);

		for (Tile* t : p_tileManager->tiles) {
			draw3DTile(t);
		}

		drawTilesCleanup();
	}

	void draw3DTile(Tile* tile)
	{
		// prepare the tile:
		verts.clear();
		indices.clear();
		for (int i = 0; i < 4; i++) {
			// pos:
			verts.push_back((GLfloat)tile->getVertPos(i).x);
			verts.push_back((GLfloat)tile->getVertPos(i).y);
			verts.push_back((GLfloat)tile->getVertPos(i).z);
			// normal:
			verts.push_back(0.0f);
			verts.push_back(0.0f);
			verts.push_back(1.0f);
			// color:
			verts.push_back((GLfloat)tile->color.r);
			verts.push_back((GLfloat)tile->color.g);
			verts.push_back((GLfloat)tile->color.b);
			// texture coord:
			verts.push_back(tile->texCoords[i].x);
			verts.push_back(tile->texCoords[i].y);
			// tile index:
			verts.push_back((GLfloat)tile->index);
		}

		if (tile->type == TileType::TILE_TYPE_XYB ||
			tile->type == TileType::TILE_TYPE_XZB ||
			tile->type == TileType::TILE_TYPE_YZB) {
			indices.push_back(0);
			indices.push_back(1);
			indices.push_back(3);
			indices.push_back(1);
			indices.push_back(2);
			indices.push_back(3);
		}
		else {
			indices.push_back(3);
			indices.push_back(1);
			indices.push_back(0);
			indices.push_back(3);
			indices.push_back(2);
			indices.push_back(1);
		}

		GLuint alphaID = glGetUniformLocation(p_shaderManager->POV3D3rdPerson.ID, "inAlpha");
		glUniform1f(alphaID, 1.0f);

		GLuint colorAlphaID = glGetUniformLocation(p_shaderManager->POV3D3rdPerson.ID, "inColorAlpha");
		glUniform1f(colorAlphaID, 0.5f);


		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, p_tileManager->texID);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * verts.size(), verts.data(), GL_DYNAMIC_DRAW);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * indices.size(), indices.data(), GL_DYNAMIC_DRAW);
		glDrawElements(GL_TRIANGLES, (GLsizei)indices.size(), GL_UNSIGNED_INT, 0);
	}

	// tileVerts is assumed to be a vector of 4 vec2s in 
	// top left, top right, bottom right, bottom left order.
	void drawTiles(Tile* tile, std::vector<glm::vec2>& drawTileVerts,
								int initialSideIndex, int initialTexIndex, int tileVertInfoOffset,
								glm::vec2 frustum[3], bool previousSides[4], float tileOpacity)
	{

		if (!tileOnScreen(drawTileVerts) || tileOpacity <= 0 || p_tileManager->drawnTiles > TileManager::MAX_DRAW_TILES) {
			return;
		}

		std::vector<glm::vec2> croppedDrawTileVerts = createNewDrawTileVerts(drawTileVerts, glm::vec2(0, 0));
		std::vector<glm::vec2> croppedDrawTileTexCoords = {
			tile->texCoords[initialTexIndex],
			tile->texCoords[(initialTexIndex + tileVertInfoOffset) % 4],
			tile->texCoords[(initialTexIndex + tileVertInfoOffset * 2) % 4],
			tile->texCoords[(initialTexIndex + tileVertInfoOffset * 3) % 4],
		};

		frustum[0] += (glm::vec2)p_camera->viewPlanePos;
		frustum[1] += (glm::vec2)p_camera->viewPlanePos;
		frustum[2] += (glm::vec2)p_camera->viewPlanePos;
		vechelp::cropTileToFrustum(croppedDrawTileVerts, croppedDrawTileTexCoords, frustum);
		frustum[0] -= (glm::vec2)p_camera->viewPlanePos;
		frustum[1] -= (glm::vec2)p_camera->viewPlanePos;
		frustum[2] -= (glm::vec2)p_camera->viewPlanePos;

		if (croppedDrawTileVerts.size() < 3) {
			return;
		}
		drawTile(croppedDrawTileVerts, croppedDrawTileTexCoords, glm::vec4(tile->color, tileOpacity));
		p_tileManager->drawnTiles++;

		for (int drawTileSideIndex = 0; drawTileSideIndex < 4; drawTileSideIndex++) {
			if (previousSides[drawTileSideIndex]) {
				continue;
			}
			glm::vec2 newFrustum[3] = {
				glm::normalize(drawTileVerts[drawTileSideIndex] - glm::vec2(p_camera->viewPlanePos)),
				glm::vec2(0, 0),
				glm::normalize(drawTileVerts[(drawTileSideIndex + 1) % 4] - glm::vec2(p_camera->viewPlanePos)),
			};
			// initial case has frustum verts = (0, 0), so we make sure to init frustum from that:
			if (frustum[0] != frustum[2]) {
				if (!vecInsideVecs(newFrustum[0], frustum[0], frustum[2])) {
					newFrustum[0] = frustum[0];
				}
				if (!vecInsideVecs(newFrustum[2], frustum[0], frustum[2])) {
					newFrustum[2] = frustum[2];
				}
			}
			// After a draw tile moves in a direction, it should never need to move back 
			// in that direction again, thus we can make sure it doesnt with this bool array:
			bool newPreviousSides[] = {
				previousSides[0], previousSides[1], previousSides[2], previousSides[3]
			};
			newPreviousSides[(drawTileSideIndex + 2) % 4] = true;

			int newSideOffset, newInitialSideIndex, newInitialTexIndex;
			int sideIndex = (initialSideIndex + tileVertInfoOffset * drawTileSideIndex) % 4;
			// The next draw tile can be either mirrored or unmirrored.  Mirrored tiles are 
			// wound the opposite way and thus have an opposite side index offset.  Unmirrored 
			// tiles have the same winding, so no adjustment is necessary.  As the side index 
			// is in the domain of [0,3], we can also just wrap around from 3 -> 0 with % 4.
			if (tile->is1stDegreeNeighborMirrored(sideIndex)) {
				newSideOffset = (tileVertInfoOffset + 2) % 4;
			}
			else { // Current connection is unmirrored:
				newSideOffset = tileVertInfoOffset;
			}
			newInitialSideIndex = (tile->get1stDegreeNeighborConnectedSideIndex(LocalDirection(sideIndex))
								   + TileManager::VERT_INFO_OFFSETS[drawTileSideIndex] * newSideOffset) % 4;
			newInitialTexIndex = newInitialSideIndex;
			if (newSideOffset == 3) {
				// then the next tile will be wound counterclockwise, and it's initial side 
				// index will key into the *top right* tex coord instead of the top left.  
				// Because the winding is counterclockwise, we can adjust the initial tex 
				// coord by incrementing it once, going from the top right to the top left!
				newInitialTexIndex = (newInitialTexIndex + 1) % 4;
			}
			std::vector<glm::vec2> newTileVerts
				= createNewDrawTileVerts(drawTileVerts, TileManager::DRAW_TILE_OFFSETS[drawTileSideIndex]);
			// Tiles that change angle will change opacity or 'tint' so that it 
			// can be noticed with traversing 3D space from this perspective:
			float newTileOpacity = tileOpacity;
			// We want a smooth transition from one tile opacity to another, so
			// it should fade as you get closer to the next draw tile's edge:
			if (tile->neighbors[sideIndex]->type != tile->type) {
				float edgeDist = vechelp::distToLineSeg((glm::vec2)p_camera->viewPlanePos,
											   drawTileVerts[drawTileSideIndex],
											   drawTileVerts[(drawTileSideIndex + 1) % 4], nullptr);
				if (edgeDist > 0.5) {
					newTileOpacity -= TileManager::DRAW_TILE_OPACITY_DECRIMENT_STEP;
				}
				else {
					newTileOpacity = tileOpacity - (edgeDist * 2) * TileManager::DRAW_TILE_OPACITY_DECRIMENT_STEP;
				}
			}

			// Finally!  We can actually go onto drawing the next tile:
			drawTiles(tile->neighbors[sideIndex], newTileVerts,
					  newInitialSideIndex, newInitialTexIndex, newSideOffset,
					  newFrustum, newPreviousSides, newTileOpacity);
		}
	}
	// Draw an individual tile to the screen.  It is assumed that all the 
	// cropping and placement has been done before this function is called:
	void drawTile(std::vector<glm::vec2> tileVerts, std::vector<glm::vec2> tileTexCoords, glm::vec4 tileColor)
	{
		// prepare the tile:
		verts.clear();
		indices.clear();
		int indexOffset = (int)verts.size() / 11;
		for (int i = 0; i < tileVerts.size(); i++) {
			// pos:
			verts.push_back(tileVerts[i].x);
			verts.push_back(tileVerts[i].y);
			verts.push_back(0);
			// normal:
			verts.push_back(0.0f);
			verts.push_back(0.0f);
			verts.push_back(1.0f);
			// color:
			verts.push_back(tileColor.r);
			verts.push_back(tileColor.g);
			verts.push_back(tileColor.b);
			// texture coord:
			verts.push_back(tileTexCoords[i].x);
			verts.push_back(tileTexCoords[i].y);
		}
		for (int i = 0; i < tileVerts.size() - 2; i++) {
			indices.push_back(indexOffset);
			indices.push_back(indexOffset + i + 1);
			indices.push_back(indexOffset + i + 2);
		}

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		GLuint alphaID = glGetUniformLocation(p_shaderManager->simpleShader.ID, "inAlpha");
		glUniform1f(alphaID, tileColor.a);

		GLuint colorAlphaID = glGetUniformLocation(p_shaderManager->simpleShader.ID, "inColorAlpha");
		glUniform1f(colorAlphaID, 0.5f);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, p_tileManager->texID);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * verts.size(), verts.data(), GL_DYNAMIC_DRAW);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * indices.size(), indices.data(), GL_DYNAMIC_DRAW);
		glDrawElements(GL_TRIANGLES, (GLsizei)indices.size(), GL_UNSIGNED_INT, 0);
	}

	void getProjectedPlayerPosInfo(TileTarget& target, glm::vec2 P, int index, float rightOffset, float upOffset,
								   glm::vec3* projectedTilePositions, int* tileIndices)
	{
		glm::vec3 bottomRight = target.tile->getVertPos(target.vertIndex(2));
		glm::vec3 rightward = (glm::vec3)target.tile->getVertPos(target.vertIndex(1)) - bottomRight;
		glm::vec3 upward = (glm::vec3)target.tile->getVertPos(target.vertIndex(3)) - bottomRight;

		projectedTilePositions[index] = bottomRight
			+ rightward * rightOffset
			+ upward * upOffset
			+ rightward * P.x
			+ upward * P.y;
		tileIndices[index] = target.tile->index;
	}

	glm::mat4 packedPlayerPosInfo()
	{
		glm::vec3 projectedTilePositions[4];
		int tileIndices[4];
		TileTarget target;
		glm::vec3 P = p_camera->viewPlanePos;

		projectedTilePositions[0] = p_tileManager->getPovTilePos();
		tileIndices[0] = p_tileManager->povTile.tile->index;

		if (p_camera->viewPlanePos.x > 0.5f) {
			target = p_tileManager->adjustTileTarget(&p_tileManager->povTile, LocalDirection::LOCAL_DIRECTION_0);
			getProjectedPlayerPosInfo(target, P, 1, -1.0f, 0.0f, projectedTilePositions, tileIndices);

			if (p_camera->viewPlanePos.y > 0.5f) {
				target = p_tileManager->adjustTileTarget(&target, LocalDirection::LOCAL_DIRECTION_3);
				getProjectedPlayerPosInfo(target, P, 3, -1.0f, -1.0f, projectedTilePositions, tileIndices);
			}
			else {
				target = p_tileManager->adjustTileTarget(&target, LocalDirection::LOCAL_DIRECTION_1);
				getProjectedPlayerPosInfo(target, P, 3, -1.0f, 1.0f, projectedTilePositions, tileIndices);
			}
		}
		else {
			target = p_tileManager->adjustTileTarget(&p_tileManager->povTile, LocalDirection::LOCAL_DIRECTION_2);
			getProjectedPlayerPosInfo(target, P, 1, 1.0f, 0.0f, projectedTilePositions, tileIndices);

			if (p_camera->viewPlanePos.y > 0.5f) {
				target = p_tileManager->adjustTileTarget(&target, LocalDirection::LOCAL_DIRECTION_3);
				getProjectedPlayerPosInfo(target, P, 3, 1.0f, -1.0f, projectedTilePositions, tileIndices);
			}
			else {
				target = p_tileManager->adjustTileTarget(&target, LocalDirection::LOCAL_DIRECTION_1);
				getProjectedPlayerPosInfo(target, P, 3, 1.0f, 1.0f, projectedTilePositions, tileIndices);
			}
		}
		if (p_camera->viewPlanePos.y > 0.5f) {
			target = p_tileManager->adjustTileTarget(&p_tileManager->povTile, LocalDirection::LOCAL_DIRECTION_3);
			getProjectedPlayerPosInfo(target, P, 2, 0.0f, -1.0f, projectedTilePositions, tileIndices);
		}
		else {
			target = p_tileManager->adjustTileTarget(&p_tileManager->povTile, LocalDirection::LOCAL_DIRECTION_1);
			getProjectedPlayerPosInfo(target, P, 2, 0.0f, 1.0f, projectedTilePositions, tileIndices);
		}

		glm::mat4 playerPosInfo = {
			projectedTilePositions[0].x, projectedTilePositions[0].y, projectedTilePositions[0].z, tileIndices[0],
			projectedTilePositions[1].x, projectedTilePositions[1].y, projectedTilePositions[1].z, tileIndices[1],
			projectedTilePositions[2].x, projectedTilePositions[2].y, projectedTilePositions[2].z, tileIndices[2],
			projectedTilePositions[3].x, projectedTilePositions[3].y, projectedTilePositions[3].z, tileIndices[3],
		};

		return playerPosInfo;
	}

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