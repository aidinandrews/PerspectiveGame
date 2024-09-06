#ifndef TILE_DEFINED
#define TILE_DEFINED

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
#include <stb_image.h>
#endif

#include "globalVariables.h"
#include "cameraManager.h"
#include "shaderManager.h"
#include "vertexManager.h"
#include "frameBuffer.h"
#include "tileNavigation.h"
#include "tileInternals.h"

// Represents a square tile in 3D space.  Can be oriented such that it is parallel with one of the x, y, or z axes.  Tiles can connect to each other and shuffle items between them.
struct Tile {
	const static bool CORNER_UNSAFE = false;
	const static bool CORNER_SAFE = true;

public: // STRUCTS

	// A basis is an immoveable building-like object that can effect entities 'on top' of it and other bases.
	struct Basis {

	public: // MEMBER VARIABLES:
		BasisType type;
		LocalOrientation localOrientation;

	public: // MEMBER FUNCTIONS:
		Basis() : type(BASIS_TYPE_NONE), localOrientation(LOCAL_DIRECTION_0) {}
	} basis;

public:
	int index;
	TileSubType type;
	glm::ivec3 position; // is the vertex with the MAX position.
	Tile* sibling;
	glm::fvec3 color;
	
	int entityIndices[9];
	int entityInfoIndices[9];

	glm::vec2 texCoords[4];
	bool cornerSafety[4];

	Tile* neighborTilePtrs[4];
	int neighborConnectedSideIndices[4];
	bool isNeighborConnectionsMirrored[4];

public: // MEMBER FUNCTIONS:

	// Tile initializer.
	// - TileSubType: One of three Tile::Types (XY, XZ, YZ) and two directions (FRONT/BACK) -> 6 options.
	// - maxPoint   : Vertex with the largest cartesian coordinates.  Other vertices will be defined through it.
	//			        This vertex will always be in pos[0].
	Tile(TileSubType tileSubType, glm::ivec3 position);

	// Returns the normal of the tile.
	glm::ivec3 getNormal();

	// Returns the vertex with the maximum cartesian coordinate values.  
	// There will never be a tie since the tiles make up a square alligned with the principle planes.
	glm::ivec3 getMaxVert() { return position; }

	glm::ivec3 getVertPos(int index);

	static LocalDirection oppositeDirection(LocalDirection currentDirection) { return LocalDirection((int)currentDirection + 2 % 4); }

	bool getEntityIndex(LocalPosition position) { return entityIndices[position]; }

	bool hasEntity(LocalPosition position) { return entityIndices[position] != -1; }

	bool hasBasis() { return basis.type != BASIS_TYPE_NONE; }

	Tile* getNeighbor(LocalDirection side) { return neighborTilePtrs[side]; }

	// Given four vertices that will make up a tile, returns that potential tile's type.
	const static TileType getTileType(glm::ivec3 tileVert1, glm::ivec3 tileVert2, glm::ivec3 tileVert3);

	// Returns the maximum of all four vertex's coordinates.  
	// Meant for use finding the maxVert of four vertices that will make up a tile.
	static glm::ivec3 getMaxVert(glm::ivec3 A, glm::ivec3 B, glm::ivec3 C, glm::ivec3 D);
	
	// Converts a TileSubType to a Tile::Type.
	static TileType superTileType(TileSubType subType);

	// Converts a Tile::Type to a TileSubType.
	static TileSubType tileSubType(TileType tileType, bool isFront);

	// Returns the 'opposite' TileSubType.  Front gets changed to back.
	// Ex: TILE_SUB_TYPE_XY_FRONT -> TILE_SUB_TYPE_XY_BACK
	static TileSubType inverseTileType(TileSubType type);

	// True if tile faces 'frontwards,' false if tile faces 'backwards.'
	bool isFrontFacing() { return type % 2 == 0; }

	static const int oppositeLocalDirection(int direction) { return (direction + 2) % 4; }
};

// This struct acts as a wrapper for a tile, giving it more information that is useful when
// rendering scenes and animations effects.  The thought is to wrap whatever tile you are 'on'
// in one of these, so that it is easier to keep track of stuff.
struct TileTarget {
	Tile *tile;
	int sideInfosOffset;
	int initialSideIndex;
	int initialVertIndex;

	TileTarget() :
		tile(nullptr), sideInfosOffset(1), initialSideIndex(0), initialVertIndex(0) {}

	TileTarget(Tile *tile, int sideInfosOffset, int initialSideIndex, int initialVertIndex) :
		tile(tile), sideInfosOffset(sideInfosOffset), initialSideIndex(initialSideIndex),
		initialVertIndex(initialVertIndex) {}

	// Returns the index to the vertex info of the tile target.  This index will key into 
	// information relating to the vertex but NOT the side.
	// index respects DrawTile ordering:
	// 0 (top left) -> 1 (top right) -> 2 (bottom right) -> 3 (bottom left)...
	int vertIndex(int i) {
		return (initialVertIndex + sideInfosOffset * i) % 4;
	}
	// Returns the index to the side info of the tile target.  This index will key into 
	// information relating to the side/connection of the side but NOT the vertex.
	// index respects DrawTile ordering:
	// 0 (top left) -> 1 (top right) -> 2 (bottom right) -> 3 (bottom left)...
	int sideIndex(int i) {
		return (initialSideIndex + sideInfosOffset * i) % 4;
	}
	// Returns the ith vertex coordinates respecting DrawTile ordering: 
	// 0 (top left) -> 1 (top right) -> 2 (bottom right) -> 3 (bottom left)...
	glm::vec3 drawTilePos(int i) {
		return (glm::vec3)tile->getVertPos(vertIndex(i));
	}
	// Returns the side info of the ith side index respecting Draw Tile ordering:
	// 0 (top left) -> 1 (top right) -> 2 (bottom right) -> 3 (bottom left)...
	//Tile::TileSideInfo *sideInfo(int i) { return &tile->sideInfos[sideIndex(i)]; }

	// Returns true if the 'next' vertex (from the perspective of a drawTile) 
	// is clockwise from the previous vertex.
	bool woundClockwise() { return sideInfosOffset == 1; }

	// Returns true if the 'next' vertex (from the perspective of a drawTile) 
	// is counterclockwise from the previous vertex.
	bool woundCounterClockwise() { return !woundClockwise(); }

	TileTarget &operator=(const TileTarget &other) {
		this->tile = other.tile;
		this->sideInfosOffset = other.sideInfosOffset;
		this->initialSideIndex = other.initialSideIndex;
		this->initialVertIndex = other.initialVertIndex;
		return *this;
	}
};

// Becuase the data structure used for storing tiles on the CPU is not easily translateable to the GPU, this
// struct will act as a translator for the information so that a scene can be drawn in a shader on the GPU.
struct alignas(32) GPU_TileInfo {
	alignas(32) glm::vec2 texCoords[4];
	
	alignas(16) int neighborIndices[4];
	alignas(16) int neighborMirrored[4];
	
	alignas(16) int neighborSideIndex[4];
	alignas(16) glm::vec4 color;

	alignas(16)  int entityIndices[4];
	alignas(16)  int entityInfoIndices[4];

	alignas(16) int cornerSafety[4];
	alignas(4)  int basisType;
	alignas(4)  int basisOrientation;
	alignas(4)  int tileSubType;
	alignas(4)  int obstructionFlags;

//	alignas(4) int padding[];

	GPU_TileInfo(Tile *tile);
};

#endif