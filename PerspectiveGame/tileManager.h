#pragma once
#include <iostream>
#include <vector>
#include <algorithm>

#define NOMINMAX
#include<Windows.h>

#ifndef GLAD_INCLUDED
#include <glad/glad.h>
#endif
#include<GLFW/glfw3.h>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#ifndef STB_IMAGE_IMPLEMENTATION
#include<stb_image.h>
#endif

#include"cameraManager.h"
#include"shaderManager.h"
#include"vertexManager.h"
#include"frameBuffer.h"
#include"tile.h"

struct TileManager {

public: // MEMBER STRUCTS:

public: // ENUMS:

public: // CONST MEMBER VARIABLES:

	static const int MAX_DRAW_TILES = 5000;
	static const glm::vec2 DRAW_TILE_OFFSETS[4];
	static const glm::vec2 INITIAL_FRUSTUM[3];
	static const float DRAW_TILE_OPACITY_DECRIMENT_STEP;
	static const int VERT_INFO_OFFSETS[4];
	static std::vector<glm::vec2> INITIAL_DRAW_TILE_VERTS;

public: // DYNAMIC MEMBER VARIABLES:

	// Other Managers:
	Camera *p_camera;
	ShaderManager *p_shaderManager;
	GLFWwindow *p_window;
	Framebuffer *p_framebuffer;
	ButtonManager *p_buttonManager;
	InputManager *p_inputManager;

	// OpenGl stuff:
	GLuint texID;
	std::vector<GLfloat> verts;
	std::vector<GLuint> indices;

	std::vector<Tile *> tiles;
	/*std::vector<Producer> producers;
	std::vector<Consumer> consumers;
	std::vector<ForceSink> forceSinks;*/

	float TOTAL_TIME = 0;

	// Drawing stuff:
	glm::mat4 tempMat;
	std::vector<glm::vec2> windowFrustum;
	float windowFrustumDiagonalLength = 0;
	int drawnTiles = 0;

	// Center of flatlander POV, used for making frustums:
	glm::vec3 povPos;
	glm::vec2 povRelativePos;
	glm::vec2 relativePos[5];
	int relativePosTileIndices[5];
	// Tile that the flatlander POV 'eye' is stood on:
	TileTarget povTile;

	// Used for lerping between views in the 3D viewer.  If a player goes from one tile to 
	// another, the view should look directly down on the tile, but there should be a 
	// transition between the two views if the angle is different.
	glm::mat4 tileRotationAdjFor3DView;
	glm::mat4 lastpovTileTransf;
	glm::mat4 currentpovTileTransf;
	float lastpovTileTransfWeight = 0;
	glm::vec3 lastCamPosOffset;
	glm::vec3 lerpCamPosOffset;

	GLuint tileInfosBufferID;
	std::vector<TileGpuInfo> tileGpuInfos;

	std::vector<int> movedTiles;
	std::vector<Tile*> entityLeaders;

public: // INITIALIZERS:

	TileManager(Camera* camera, ShaderManager* shaderManager, GLFWwindow* window, Framebuffer* framebuffer, ButtonManager* bm, InputManager* im) {

		currentpovTileTransf = glm::mat4(1);
		lastpovTileTransf = glm::mat4(1);
		lerpCamPosOffset = glm::vec3(0);


		p_camera = camera;
		p_shaderManager = shaderManager;
		p_window = window;
		p_framebuffer = framebuffer;
		p_buttonManager = bm;
		p_inputManager = im;

		// make sure the camera is in the middle of the starting draw tile:
		p_camera->viewPlanePos = glm::vec3(0.5f, 0.5f, 0.0f);

		// by this point there should be two tiles in the scene:
		createTilePair(Tile::Type::TILE_TYPE_XY, glm::ivec3(1, 1, 0), glm::vec3(0, 0, 1), glm::vec3(0, 0, 0.8));

		povTile.tile = tiles[0];
		povPos = glm::vec3(0.5f, 0.5f, 0.0f);
		povTile.initialSideIndex = 0;
		povTile.initialVertIndex = 0;
		povTile.sideInfosOffset = 1;

		lastCamPosOffset = glm::vec3(0, 0, 0);

		glGenBuffers(1, &tileInfosBufferID);
	}

	~TileManager();

public: // MEMBER FUNCTIONS:

	void update();
	void updateVisuals();

	void updateTileGpuInfoIndices();
	void getRelativePovPosGpuInfos();
	glm::vec2 getRelativePovPosCentral(TileTarget & target);
	glm::vec2 getRelativePovPosTop(TileTarget & target);
	glm::vec2 getRelativePovPosBottom(TileTarget & target);
	glm::vec2 getRelativePovPosRight(TileTarget & target);
	glm::vec2 getRelativePovPosLeft(TileTarget & target);
	glm::vec2 getRelativePovPosTopRight(TileTarget & target);
	glm::vec2 getRelativePovPosTopLeft(TileTarget & target);
	glm::vec2 getRelativePovPosBottomRight(TileTarget & target);
	glm::vec2 getRelativePovPosBottomLeft(TileTarget & target);

	void clearEntityIndices()
	{
		for (Tile* tile : tiles) {
			for (int i = 0; i < 9; i++) {
				tile->entityIndices[i] = -1;
			}
		}
	}

	// The tiles are drawn one by one, and need to know when they are off screen.  This is 
	// done by translating the edges of the window to scene coordinates, in this function.
	void updateWindowFrustum();

	void deleteTile(Tile *tile);
	void deleteBuilding(Tile *tile);

	// Because of space wrapping, there are times when going over a corner of a tile would actually rip or stretch
	// an object in a way that is unacceptable.  These corners are 'unsafe' corners, and should not be traversable.
	// This function will identify them and move the player away if they cross over them.
	void collisionDetectUnsafeCorners();
	
	// The player is actually restricted to the domain { (x, y) | 0 <= x <= 1, 0 <= y <= 1 }, so when a tile edge is
	// 'crossed,' the player/camera is teleported to the opposite side of this 'meta tile' and the target info is 
	// updated.  This illusion makes it seem like we are moving from tile to tile when we are not.  povTile updates
	// its initial vert/side data as needed.
	void updatePovTileTarget ();

	// Given a tile target wrapped around a tile, this function will 'move' the target to a neighboring tile, making
	// sure to keep the initial vert/side info correct through the transition.
	static TileTarget adjustTileTarget(TileTarget  *currentTarget, int drawTileEdgeIndex);
	
	// It looks better if the camera tilts in such a way that when passing over the edge of a tile, the view
	// rotates smoothly from looking down on one tile to the next.  This adjustment needs to be calculated here.
	void update3dRotationAdj();

	// Because the camera is limited to a domain of { (x, y) | 0 <= x <= 1, 0 <= y <= 1 }, the actual position
	// of the player has to be interpolated in reference to the current povTile.  This returns the player's
	// 3D position/where the camera is looking.
	glm::vec3 getPovTilePos();

	// Each tile necessitates another tile to act as its backing.  A 2D tile has 2 faces, 
	// yes?  Thus when making a tile, two must be made, glued together, then added to the 
	// scene. The 'maxPoint' is the maximum vertex of the new tile pair.  I.e. a tile with 
	// vertices (0,0,0), (1,0,0), (1,0,1), and (0,0,1) would have a maxPoint of (1,0,1).  This 
	// makes sure that all tiles of the same type have their vertices in the correct order!
	bool createTilePair(Tile::Type tileType, glm::ivec3 maxPoint, glm::vec3 frontTileColor, glm::vec3 backTileColor);
	
	// In the creation of a tile, it needs to be connected to it's neighbors so that things can move from
	// one tile to another.
	void connectUpNewTile(Tile *newTile);
	
	// Trys to connect the subject tile to the other tile.
	// Returns false if the tiles cannot connect.
	bool tryConnect(Tile *subject, Tile *other);

	// Returns true if the new tile overlaps/shares all vertices with an existing tile.  
	// If true, *do not add this tile to the scene!*
	bool tileIsUnique(Tile &newTile);

	std::vector<glm::vec2> createNewDrawTileVerts(std::vector<glm::vec2> &parent, glm::vec2 adj);

	// True if B is 'inside' or 'between' A and C.
	bool vecInsideVecs(glm::vec2 A, glm::vec2 B, glm::vec2 C);
	// Checks if the draw tile is within the bounds of the projected window space.
	bool tileOnScreen(std::vector<glm::vec2> &tileVerts);

	void drawSetup();
	void drawAddTilePreview();
	// Draws the 2D scene from a 3rd person perspective.
	void draw2D3rdPerson();

	void drawPlayerPos();
	void draw3Dview();
	void draw3DTile(Tile *tile);
	// tileVerts is assumed to be a vector of 4 vec2s in 
	// top left, top right, bottom right, bottom left order.
	void drawTiles(Tile *tile, std::vector<glm::vec2> &tileVerts,
				   int initialSideIndex, int initialTexIndex, int tileVertInfoOffset,
				   glm::vec2 frustum[3], bool previousSides[4], float tileOpacity);
	// Draw an individual tile to the screen.  It is assumed that all the 
	// cropping and placement has been done before this function is called:
	void drawTile(std::vector<glm::vec2> tileVerts, std::vector<glm::vec2> tileTexCoords,
				  glm::vec4 tileColor);
	glm::mat4 packedPlayerPosInfo();
	void drawCleanup();
};