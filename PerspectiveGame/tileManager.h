#pragma once
#include <iostream>
#include <vector>
#include <algorithm>

#define NOMINMAX
#include<Windows.h>

#include"dependancyHeaders.h"

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

public:
	// Connections:
	Camera *p_camera;
	ShaderManager *p_shaderManager;
	GLFWwindow *p_window;
	Framebuffer *p_framebuffer;
	ButtonManager *p_buttonManager;
	InputManager *p_inputManager;
	//SuperPositionNetwork* p_superPositionNetwork;

	std::vector<Tile*> tiles;
	std::vector<GPU_TileInfo> tileGpuInfos;

	// Tile that the flatlander POV 'eye' is stood on:
	TileTarget povTile;
	
	// OpenGl stuff:
	GLuint texID;
	GLuint tileInfosBufferID;

	// Drawing stuff:
	std::vector<glm::vec2> windowFrustum;
	int drawnTiles;

	// Used for lerping between views in the 3D viewer.  If a player goes from one tile to 
	// another, the view should look directly down on the tile, but there should be a 
	// transition between the two views if the angle is different.
	glm::mat4 tileRotationAdjFor3DView;
	glm::mat4 lastpovTileTransf;
	glm::mat4 currentpovTileTransf;
	glm::vec3 lastCamPosOffset;
	glm::vec3 lerpCamPosOffset;
	float lastpovTileTransfWeight = 0;

public: // INITIALIZERS:

	TileManager(Camera* camera, ShaderManager* shaderManager, GLFWwindow* window, Framebuffer* framebuffer, 
				ButtonManager* bm, InputManager* im, SuperPositionNetwork* sp) {

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
		createTilePair(TILE_TYPE_XY, glm::ivec3(1, 1, 0), glm::vec3(0, 0, 1), glm::vec3(0, 0, 0.8));

		povTile.node = tiles[0];
		povTile.initialSideIndex = 0;
		povTile.initialVertIndex = 0;
		povTile.sideInfosOffset = 1;

		lastCamPosOffset = glm::vec3(0, 0, 0);

		glGenBuffers(1, &tileInfosBufferID);
	}

	~TileManager();

public: // MEMBER FUNCTIONS:

	void update();
	void updateVisualInfos();

	// An individual tile can never be created by itself as every tile has a 'backer' tile (sibling) connected to it.
	bool createTilePair(SuperTileType tileMapType, glm::ivec3 maxPoint, glm::vec3 frontTileColor, glm::vec3 backTileColor);

	// An individual tile can never be deleted alone as every tile has a 'backer' tile (sibling) connected to it.
	void deleteTilePair(Tile* node, bool allowDeletePovTile);

	void updateTileGpuInfos();
	void updateCornerSafety(Tile* node);

	// The tiles are drawn one by one, and need to know when they are off screen.  This is 
	// done by translating the edges of the window to scene coordinates, in this function.
	void updateWindowFrustum();

	// Because of space wrapping, there are times when going over a corner of a tile would actually rip or stretch
	// an object in a way that is unacceptable.  These corners are 'unsafe' corners, and should not be traversable.
	// This function will identify them and move the player away if they cross over them.
	void solvePlayerUnsafeCornerCollisions();
	
	// The player is actually restricted to the domain { (x, y) | 0 <= x <= 1, 0 <= y <= 1 }, so when a tile edge is
	// 'crossed,' the player/camera is teleported to the opposite side of this 'meta tile' and the target info is 
	// updated.  This illusion makes it seem like we are moving from tile to tile when we are not.  povTile updates
	// its initial vert/side data as needed.
	void updatePovTileTarget();

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
	
	// In the creation of a tile, it needs to be connected to it's neighbors so that things can move from
	// one tile to another.
	void updateNeighborConnections(Tile *node);

	void createCornerSuperPosition(Tile* node, LocalPosition pos);
	void updateSuperPositionNeighbors(SuperPosition* pos);
	void removeConnectedSuperPositions(Tile* node, Tile* sibling, std::vector<SuperPosition*>& affectedSuperPositions);
	void createMetaNodeConnections(Tile* node, Tile* sibling);
	void updateMetaNodeConnections(Tile* node);
	
	// Tries to connect the subject tile to the other tile.
	// Returns false if the tiles cannot connect.
	bool tryConnect(Tile *subject, Tile *other);

	// Returns true if the new tile overlaps/shares all vertices with an existing tile.  
	// If true, *do not add this tile to the scene!*
	bool tileIsUnique(Tile &newTile);

	// TODO: this can probably be vastly simplified:
	void getRelativePovPosGpuInfos(glm::vec2* relativePos, int* relativePosTileIndices);
	glm::vec2 getRelativePovPosCentral(TileTarget& target);
	glm::vec2 getRelativePovPosTop(TileTarget& target);
	glm::vec2 getRelativePovPosBottom(TileTarget& target);
	glm::vec2 getRelativePovPosRight(TileTarget& target);
	glm::vec2 getRelativePovPosLeft(TileTarget& target);
	glm::vec2 getRelativePovPosTopRight(TileTarget& target);
	glm::vec2 getRelativePovPosTopLeft(TileTarget& target);
	glm::vec2 getRelativePovPosBottomRight(TileTarget& target);
	glm::vec2 getRelativePovPosBottomLeft(TileTarget& target);
};