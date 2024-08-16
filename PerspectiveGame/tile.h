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

enum ENTITYPOSTEST {

};

// Represents a square tile in 3D space.  Can be oriented such that it is parallel with one of the x, y, or z axes.  Tiles can connect to each other and shuffle items between them.
struct Tile {

public: // ENUMS

	// Each tile has a type corrosponding to what principal/coordinate plane (XY, XA, and YZ) the tile is 
	// parallel to.  This type means that a tile can be fully described with a type and a point on the 
	// corrosponding coordinaate plane that corrosponds to one of it's vertices.
	enum Type {
		TILE_TYPE_XY = 0,
		TILE_TYPE_XZ = 1,
		TILE_TYPE_YZ = 2,
	};

	// Inside each tile are four edges.  Each edge will be parallel to an axis of R3 (X, Y, or Z). 
	// Thus, each edge can be described partially by what axis it is parallel to.  
	// An edge type and a vertex corrosponding to its 'tail' are all that is needed to fully descibe an edge. 
	enum TileEdgeType {
		TILE_EDGE_TYPE_X,
		TILE_EDGE_TYPE_Y,
		TILE_EDGE_TYPE_Z,
	};

	// Tiles are all square and can differ from each other in location and angle.  
	// They are either parallel with the XY, XZ, or YZ planes, so they can either have a 'dihedral angle' 
	// (internal angle of 90, 180, or 270 degrees).  
	// Both 90 and 270 are interchangeable depending on the perspective between two tiles, 
	// but it can be useful to desribe them as different in certain circumstances.
	enum TileDihedral {
		TILE_DIHEDRAL_90,
		TILE_DIHEDRAL_180,
		TILE_DIHEDRAL_270,
	};

	enum TileRelation {
		TILE_RELATION_FLAT,
		TILE_RELATION_UP,
		TILE_RELATION_DOWN,
		TILE_RELATION_FLIPPED,
	};

	enum BuildingType {
		BUILDING_TYPE_NONE,
		BUILDING_TYPE_PRODUCER,
		BUILDING_TYPE_CONSUMER,
	};

public: // STRUCTS

	struct SideInfos {

	public: // MEMBER VARIABLES:

		glm::vec2 texCoords[4];
		Tile *connectedTiles[4];
		int connectedSideIndices[4];
		bool connectionsMirrored[4];

	public: // MEMBER FUNCTIONS:

		SideInfos() {
			for (int i = 0; i < 4; i++) {
				texCoords[i] = glm::vec2(0, 0);
				connectedTiles[i] = nullptr;
				connectedSideIndices[i] = 0;
				connectionsMirrored[i] = false;
			}
		}
	};

	// Each tile can have an underlying ans immoveable (basis) structure if necessary for its action.
	// Ex: A tile that produces entities needs to communicate this visually and also cannot be able to be moved.
	// Providing the shader with information about a tile's basis will allow it to rener under the tile's entity.
	// Not allowing basis structures to move is necessary because of how they interact with entities.  If a basis
	// could move, there would be no way to seperate it from the object it interacts with.  Overall, tiles are
	// the space and basis structures are the forces/actors that act on that space.
	struct Basis {

	public: // MEMBER ENUMS:

	public: // MEMBER VARIABLES:

		BasisType type;
		LocalDirection localOrientation;

	public: // MEMBER FUNCTIONS:

		Basis() : type(BASIS_TYPE_NONE), localOrientation(LOCAL_DIRECTION_0) {}
	};

public: // MEMBER VARIABLES:
	int index = 0;
	Tile *sibling;

	TileSubType type;
	
	LocalDirection forceLocalDirection;

	// Each bit of this mask corrisponds to a 16th of the tile taken up by some object inside it.
	// Ordered (corner numbers are tile edge info indices/local direction codes):
	// 3 ------------------> 0
	// ^ [00] [01] [02] [03] |
	// |					 |
	// | [04] [05] [06] [07] |
	// |                     |
	// | [08] [09] [10] [11] |
	// |                     |
	// | [12] [13] [14] [15] V
	// 2 <------------------ 1
	uint16_t obstructionMask;

	// Due to the 3D nature of tiles, some assortments of tile connections make 'crossing' a corner impossible
	// for a 2D entity.  If a corner is 'unsafe,' it will be designated with true in this array.  Each component
	// of the array corrisponds to it's corner index.  Corner 0 -> unsafeCorners[0].
	bool unsafeCorners[4];

	// Indices indexing into the entities located in this tile (entity list in EntityManager).
	int entityIndices[9];

	Basis basis;

	SideInfos sideInfos;

	// only one position value is needed since the other three corners 
	// can be caluclated using only the maxVert and tileType variables.
	glm::ivec3 maxVert;

	// Used for decoration/organization.  
	// Colors the tile a certain color/pattern.	
	glm::fvec3 color;

private:
	// This array maps the orientation/direction info of entities/forces/bases when moving from one tile to another.
	// The simple case (moving from an XYF -> XYF) is easy (side n -> side n), but mapping differing tile types is more 
	// difficult!  Hopefully this array will mean lightnig fast queries when moving entities/forces between tiles.
	// Entries 0 - 3 represent sides, -1 represents a movement that should not be possible.  i.e. moving from an XYF 
	// side 0 -> XZF tile.  Impossible movements are defined as 'X' so things look a little nicer.
	// Input Key:
	// [Current Tile Type][Destination Tile Type][Exiting side][Current Orientation]
	static const LocalDirection ORIENTATION_TO_ORIENTATION_MAP[6][6][4][4];

	// Given a local direction relative to a tile, will return it's global direction relative to 3D euclidean coordinates:
	static const GlobalDirection LOCAL_DIR_TO_GLOBAL_DIR[6][4];

public: // MEMBER FUNCTIONS:

	//Tile();

	// Tile initializer.
	// - TileSubType: One of three Tile::Types (XY, XZ, YZ) and two directions (FRONT/BACK) -> 6 options.
	// - maxPoint: Vertex with the largest cartesian coordinates.  Other vertices will be defined through it.
	//			   This vertex will always be in pos[0].
	Tile(TileSubType tileSubType, glm::ivec3 maxVert);

	~Tile() {
		//delete entity;
	}

	// Returns the normal of the tile.
	glm::ivec3 normal();

	// Returns the vertex with the maximum cartesian coordinate values.  
	// There will never be a tie since the tiles make up a square alligned with the principle planes.
	glm::ivec3 getMaxVert() { return maxVert; }

	glm::ivec3 getVertPos(int index);

	static LocalDirection oppositeDirection(LocalDirection currentDirection) { return LocalDirection((int)currentDirection + 2 % 4); }

	bool hasForce() { return forceLocalDirection < 4; }

	bool hasEntity()
	{
		for (int i = 0; i < 9; i++) {
			if (entityIndices[i] != -1) {
				return true;
			}
		}
		return false;
	}

	// Returns true if there is an entity index in the input index position.
	// Note: entityIndicesIndex IS NOT necissarily the entity position of the enitity it indexes to.
	//    The entity indexed by entityIndices[0] may be in LOCAL_POSITION_MIDDLE_3, for instance.
	bool hasEntity(int entityIndicesIndex)
	{
		if (entityIndices[entityIndicesIndex] != -1) {
			return true;
		}
		return false;
	}

	bool isObstructed(LocalPosition position) {
		return obstructionMask & TileNavigator::getObstructionMask(position);
	}

	bool hasBasis() { return basis.type != BASIS_TYPE_NONE; }

	Tile* getNeighbor(LocalDirection side) { return sideInfos.connectedTiles[side]; }

	// Given four vertices that will make up a tile, returns that potential tile's type.
	static Tile::Type getTileType(glm::ivec3 A, glm::ivec3 B, glm::ivec3 C, glm::ivec3 D);
	
	// Returns the maximum of all four vertex's coordinates.  
	// Meant for use finding the maxVert of four vertices that will make up a tile.
	static glm::ivec3 getMaxVert(glm::ivec3 A, glm::ivec3 B, glm::ivec3 C, glm::ivec3 D);
	
	// Converts a TileSubType to a Tile::Type.
	static Tile::Type superTileType(TileSubType subType);

	// Converts a Tile::Type to a TileSubType.
	static TileSubType tileSubType(Tile::Type tileType, bool isFront);

	// Returns the 'opposite' TileSubType.  Front gets changed to back.
	// Ex: TILE_SUB_TYPE_XY_FRONT -> TILE_SUB_TYPE_XY_BACK
	static TileSubType inverseTileType(TileSubType type);

	// True if tile faces 'frontwards,' false if tile faces 'backwards.'
	bool isFrontFacing() { return type % 2 == 0; }

	// Returns the connected tile in the direction of neighborTileIndex:
	Tile* neighbor(int neighborIndex) { return sideInfos.connectedTiles[neighborIndex]; }

	// Given two tiles, will return the 'relationship' between both.
	// 
	// Ex: if tile A is of type XY front and B is of XY front, the relationship is 'flat' since they lie on the 
	// same plane.  If tile A is of type XY front and B is of XZ front, the relationship would be 'up' as to get
	// from one tile to the next, you would have to go up.  I picture it as looking at a wall for up relationships
	// and at a cliff for down.  A pair of type XY front and  XZ back have a 'down' relationship, as to get from 
	// one to the other feels like going off the edge of a cliff.  
	// .
	// You can also picture it as dihedral angles:
	//     TILE_RELATION_FLAT    == 180 degrees 
	//     TILE_RELATION_UP      == 90 degrees 
	//     TILE_RELATION_DOWN    == 270 degrees 
	//     TILE_RELATION_FLIPPED == 360 degrees
	static Tile::TileRelation getRelation(TileSubType A, TileSubType B);

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
struct alignas(32) TileGpuInfo {
	alignas(32) glm::vec2 texCoords[4];
	
	alignas(16) int neighborIndices[4];
	alignas(16) int neighborMirrored[4];
	
	alignas(16) int neighborSideIndex[4];
	alignas(16) glm::vec4 color;

	alignas(4)  int entityIndices[9]; // 36
	
	alignas(4)  int basisType;
	alignas(4)  int basisOrientation;
	alignas(4)  int tileSubType;
	alignas(4)  int obstructionMask;
	alignas(4)  int padding[3];

	TileGpuInfo(Tile *tile);
};

#endif