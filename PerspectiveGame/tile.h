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

#include "vectorHelperFunctions.h"
#include "globalVariables.h"
#include "cameraManager.h"
#include "shaderManager.h"
#include "vertexManager.h"
#include "frameBuffer.h"
#include "tileNavigation.h"
#include "tileInternals.h"
#include "metaPositionNodeManager.h"

struct TileManager;

// Represents a square tile in 3D space.  Can be oriented such that it is parallel with one of the x, y, or z axes.  Tiles can connect to each other and shuffle items between them.
struct Tile {
	const static bool CORNER_UNSAFE = false;
	const static bool CORNER_SAFE = true;

public: // STRUCTS

	friend struct TileManager;

	// A basis is an immoveable building-like object that can effect entities 'on top' of it and other bases.
	struct Basis {

	public: // MEMBER VARIABLES:
		BasisType type;
		LocalOrientation localOrientation;

	public: // MEMBER FUNCTIONS:
		Basis() : type(BASIS_TYPE_NONE), localOrientation(LOCAL_DIRECTION_0) {}
	} basis;

private:
	// The 'neighborhood' is the set of neighbors orthogonally and diagonally connected to this tile.
	// Index scheme:
	//    [n][0] == 1st degree neighbor in direction of n.
	//    [n][1] == 2nd degree neighbor clockwise from [n][0].
	//    [n][2] == 2nd degree neighbor counterclockwise from [n][0].
	struct Neighborhood {
		Tile* tiles[4][3];
		int maps[4][3];

		Neighborhood()
		{
			for (int i = 0; i < 4; i++) {
				for (int j = 0; j < 3; j++) {
					tiles[i][j] = nullptr;
					maps[i][j] = -1;
				}
			}
		}
	} neighborhood;

public:
	int index;
	TileType type;
	glm::ivec3 position; // is the vertex with the MAX position.
	Tile* sibling;
	glm::fvec3 color;
	
	int entityIndices[9];
	int entityInfoIndices[9];

	glm::vec2 texCoords[4];
	bool cornerSafety[4];

	//Tile* neighborTilePtrs[4];
	//int neighborAlignmentMapIndex[4];

public: // MEMBER FUNCTIONS:

	// Tile initializer.
	// - TileSubType: One of three Tile::Types (XY, XZ, YZ) and two directions (FRONT/BACK) -> 6 options.
	// - maxPoint   : Vertex with the largest cartesian coordinates.  Other vertices will be defined through it.
	//			        This vertex will always be in pos[0].
	Tile(TileType getTileType, glm::ivec3 position);

	// Returns the normal of the tile.
	glm::ivec3 getNormal();

	// Returns the vertex with the maximum cartesian coordinate values.  
	// There will never be a tie since the tiles make up a square alligned with the principle planes.
	glm::ivec3 getMaxVert() { return position; }

	glm::ivec3 getVertPos(int index);

	bool getEntityIndex(LocalPosition position) { return entityIndices[position]; }

	Tile* getNeighbor(LocalDirection side) { return neighborhood.tiles[side][0]; }
	Tile* getNeighborNeighbor(LocalDirection side, bool clockwise) { return neighborhood.tiles[side][1 + (!clockwise)]; }

	int getNeighborAlignmentMap(LocalDirection side) { return neighborhood.maps[side][0]; }
	int getNeighborNeighborAlignmentMap(LocalDirection side, bool clockwise) { 
		return neighborhood.maps[side][1 + (!clockwise)];
	}

	
	bool hasEntity(LocalPosition position) { return entityIndices[position] != -1; }
	bool hasBasis() { return basis.type != BASIS_TYPE_NONE; }

	// True if tile faces 'frontwards,' false if tile faces 'backwards.'
	bool isFrontFacing() { return type % 2 == 0; }

	LocalAlignment mapAlignmentTo1stDegreeNeighbor(LocalDirection leavingDirection, LocalAlignment currentAlignment)
	{
#ifdef RUNNING_DEBUG
		tnav::checkOrthogonal(leavingDirection);
#endif
		return tnav::getMappedAlignment(neighborhood.maps[leavingDirection][0], currentAlignment);
	}

	// TODO: Try to remove this function:
	bool is1stDegreeNeighborMirrored(int leavingDirection)
	{
#ifdef RUNNING_DEBUG
		tnav::checkOrthogonal((LocalDirection)leavingDirection);
#endif
		return (neighborhood.maps[leavingDirection][0] > 3);
	}

	// TODO: Try to remove this function:
	int get1stDegreeNeighborConnectedSideIndex(LocalDirection leavingDirection)
	{
#ifdef RUNNING_DEBUG
		tnav::checkOrthogonal(leavingDirection);
#endif
		return mapAlignmentTo1stDegreeNeighbor(leavingDirection, tnav::oppositeAlignment(leavingDirection));
	}
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
	alignas(4)  int getTileType;
	alignas(4)  int obstructionFlags;

//	alignas(4) int padding[];

	GPU_TileInfo(Tile *tile);
};

#endif