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

#include "cameraManager.h"
#include "shaderManager.h"
#include "vertexManager.h"
#include "frameBuffer.h"
//#include "entity.h"

// Represents a square tile in 3D space.  Can be oriented such that it is parallel with one of the x, y, or z axes.  Tiles can connect to each other and shuffle items between them.
struct Tile {
	/////////
	//ENUMS//
	/////////

	// Each tile has a type corrosponding to what principal/coordinate plane (XY, XA, and YZ) the tile is 
	// parallel to.  This type means that a tile can be fully described with a type and a point on the 
	// corrosponding coordinaate plane that corrosponds to one of it's vertices.
	enum Type {
		TILE_TYPE_XY = 0,
		TILE_TYPE_XZ = 1,
		TILE_TYPE_YZ = 2,
	};

	// Along with the Tile::Type (XY, XZ, YZ), each tile has a direction (FRONT/BACK). This lets us know how we 
	// should be looking at the tile, as each tile has a 'sibling' that faces the opposite direction.
	enum SubType {
		TILE_TYPE_XY_FRONT = 0,
		TILE_TYPE_XY_BACK = 1,
		TILE_TYPE_XZ_FRONT = 2,
		TILE_TYPE_XZ_BACK = 3,
		TILE_TYPE_YZ_FRONT = 4,
		TILE_TYPE_YZ_BACK = 5,
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

	// Tiles exist in 3D space, and thus can be described with cartesian coordinates x, y, and z. 
	// Thus, there are 6 primary directions: left, right, forward, back, up, and down. 
	// Less relative are: x+, x-, y+, y-, z+, z-.  
	// The latter is what has been used for a naming convention in this enum.
	enum TileDirection {
		TILE_DIR_X_POS,
		TILE_DIR_X_NEG,
		TILE_DIR_Y_POS,
		TILE_DIR_Y_NEG,
		TILE_DIR_Z_POS,
		TILE_DIR_Z_NEG,
	};

	enum Edge {
		RIGHT,
		DOWN,
		LEFT,
		UP
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

	enum EntityType {
		ENTITY_TYPE_NONE,
		ENTITY_TYPE_RED_CIRCLE,
	};

	///////////
	//STRUCTS//
	///////////

	struct Connection {
		Tile *tile = nullptr;
		int sideIndex = 0;
		bool mirrored = false;
	};

	struct TileSideInfo {
		glm::vec2 texCoord{};
		Connection connection;
	};

	////////////////////
	//MEMBER VARIABLES//
	////////////////////

	int index = 0;
	Tile::SubType type;
	glm::ivec3 maxVert;

	glm::fvec3 color;
	bool hasBeenDrawn;
	float opacity;
	Tile *sibling;
	TileSideInfo sideInfos[4];

	BuildingType buildingType;
	Tile::Edge buildingOrientation;

	EntityType entityType;
	float entityOffset;
	Tile::Edge entityOffsetSide;

	////////////////
	//INITIALIZERS//
	////////////////

	Tile();

	// Tile initializer.
	// - Tile::SubType: One of three Tile::Types (XY, XZ, YZ) and two directions (FRONT/BACK) -> 6 options.
	// - maxPoint: Vertex with the largest cartesian coordinates.  Other vertices will be defined through it.
	//			   This vertex will always be in pos[0].
	Tile(Tile::SubType tileSubType, glm::ivec3 maxVert);


	////////////////////////////
	//DYNAMIC MEMBER FUNCTIONS//
	////////////////////////////

	// Returns the normal of the tile.
	glm::ivec3 normal();

	// Returns the vertex with the maximum cartesian coordinate values.  
	// There will never be a tie since the tiles make up a square alligned with the principle planes.
	glm::ivec3 getMaxVert() { return maxVert; }

	glm::ivec3 getVertPos(int index);

	///////////////////////////
	//STATIC MEMBER FUNCTIONS//
	///////////////////////////

	// Given four vertices that will make up a tile, returns that potential tile's type.
	static Tile::Type getTileType(glm::ivec3 A, glm::ivec3 B, glm::ivec3 C, glm::ivec3 D);
	
	// Returns the maximum of all four vertex's coordinates.  
	// Meant for use finding the maxVert of four vertices that will make up a tile.
	static glm::ivec3 getMaxVert(glm::ivec3 A, glm::ivec3 B, glm::ivec3 C, glm::ivec3 D);
	
	// Converts a Tile::SubType to a Tile::Type.
	static Tile::Type superTileType(Tile::SubType subType);

	// Converts a Tile::Type to a Tile::SubType.
	static Tile::SubType tileSubType(Tile::Type tileType, bool isFront);

	// Returns the 'opposite' Tile::SubType.  Front gets changed to back.
	// Ex: TILE_SUB_TYPE_XY_FRONT -> TILE_SUB_TYPE_XY_BACK
	static Tile::SubType inverseTileType(Tile::SubType type);

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
	static Tile::TileRelation getRelation(Tile::SubType A, Tile::SubType B);
};

// This struct acts as a wrapper for a tile, giving it more information that is useful when
// rendering scenes and animations effects.  The thought is to wrap whatever tile you are 'on'
// in one of these, so that it is easier to keep track of stuff.
struct PovTileTarget {
	Tile *tile;
	int sideInfosOffset;
	int initialSideIndex;
	int initialVertIndex;

	PovTileTarget() :
		tile(nullptr), sideInfosOffset(1), initialSideIndex(0), initialVertIndex(0) {}

	PovTileTarget(Tile *tile, int sideInfosOffset, int initialSideIndex, int initialVertIndex) :
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
	Tile::TileSideInfo *sideInfo(int i) { return &tile->sideInfos[sideIndex(i)]; }

	// Returns true if the 'next' vertex (from the perspective of a drawTile) 
	// is clockwise from the previous vertex.
	bool woundClockwise() { return sideInfosOffset == 1; }

	// Returns true if the 'next' vertex (from the perspective of a drawTile) 
	// is counterclockwise from the previous vertex.
	bool woundCounterClockwise() { return !woundClockwise(); }

	PovTileTarget &operator=(const PovTileTarget &other) {
		this->tile = other.tile;
		this->sideInfosOffset = other.sideInfosOffset;
		this->initialSideIndex = other.initialSideIndex;
		this->initialVertIndex = other.initialVertIndex;
		return *this;
	}
};

// Becuase the data structure used for storing tiles on the CPU is not easily translateable to the GPU, this
// struct will act as a translator for the information so that a scene can be drawn in a shader on the GPU.
struct TileGpuInfo {
	alignas(16) glm::vec4 color;
	alignas(16) int neighborIndices[4];
	alignas(16) int neighborMirrored[4];
	alignas(16) int neighborSideIndex[4];
	alignas(32) glm::vec2 texCoords[4];
	alignas(4) int buildingType;
	alignas(4) int entityType;
	alignas(4) float entityOffset;
	alignas(4) int entityOffsetSide;

	TileGpuInfo(Tile *tile);
};

#endif



struct NewTile {
public: // ENUMS AND MEMBER STRUCTS:

	// Each tile has one of 6 orientations.  TileType specifies this orientation.  
	// The first two letters designate the coordinate plane the tile is parallel 
	// to (XY, XZ, or YZ) and the third letter specifies if it is facing forward 
	// (F), or backward (B).
	enum Type {
		XYF, XYB, XZF, XZB, YZF, YZB,
	};

	// Since the world is made up of tiles, there are only 4 possible directions 
	// to go when moving between them.  Unforfunately, since there are 6 different 
	// tileTypes/tile orientations, this direction must be relative to each tile.  
	// This means that when comparing directions between tiles, a conversion must 
	// take place if the two tiles do not have the same tileType.
	enum Direction {
		DIRECTION_0,
		DIRECTION_1,
		DIRECTION_2,
		DIRECTION_3,
	};

	// Each side of a tile has associated data that is specified here.  
	// This coveres connections to other tiles, texture information, etc.
	struct SideInfos {
		glm::vec2 texCoords[4];
		
		Tile *connectedTiles[4];
		Direction connectedSideIndices[4];
		bool connectionsMirrored[4];

		SideInfos() {
			for (int i = 0; i < 4; i++) {
				texCoords[i] = glm::vec2(0, 0);
				connectedTiles[i] = nullptr;
				connectedSideIndices[i] = DIRECTION_0;
				connectionsMirrored[i] = false;
			}
		}
	};

	struct Force {
		// This can be any number but will likely be limited for game design.  
		// Force applies to masses and makes them move.
		int magnitude;

		// Since there are only 4 directions force can be applied, 
		// only two bits are needed to store the direction.
		unsigned int direction : 2;
	};

	struct Entity {
	public: // ENUMS AND MEMBER STRUCTS:

		// All objects can be moved, so buildings and materials
		// both fall under the 'entity' struct.
		enum Type {
			NONE,

			MATERIAL_OMNI, 
			MATERIAL_A, 
			MATERIAL_B,

			BUILDING_PRODUCER, 
			BUILDING_CONSUMER, 
			BUILDING_COMPRESSOR
		};

	public: // MEMBER VARIABLES:

		Type type;

		// Velocity if quantized to 0, 1, 2, 4 tiles/tick for
		// optimization/simplicitys sake so it can be represented by two bits.
		unsigned int velocity : 2;

		// Because velocity is quantized, each sub-tick an object can only go 
		// incriments of 1/4th of a tile (making the representation only 2 bits 
		// long), with a maximum of 1 tile/sub tick @ velocity of 4 tiles/tick.
		unsigned int offset : 2;

		// Entities can move, so they need a movement direction.  There are only 4 
		// possible directions of movement, so all directions can be represented 
		// by 2 bits.  This direction is relative to the tileType, so comparing 
		// directions of entities onside tiles of different tileTypes will require 
		// a conversion function.
		unsigned int direction : 2;

		// Force blocks let tiles give force to entities, and this given force is 
		// kept track of this this variable.  Force is consumed on movement, so 
		// this variable will be incremented by force tiles and decremented by 
		// entity movement.  Max value is 3 * mass as velocity of 4 tiles/tick is 
		// max velocity.
		int storedForce;

		// Entities have mass.  This mass determines how much force is required 
		// for them to move.  Each tick, each entity will consume an amount of 
		// storedForce equal to n times its mass, allowing the entity to move k 
		// tiles between ticks.
		int mass;

		// Opacity is used on creation/destruction of objects (and maybe 2D 
		// lighting simulation if I get to that).  On creation, an entity will 
		// slowly increase its opacity until fully opaque.  On destruction, an 
		// entuty will slowly decrease its opacity until full transparent, then 
		// delete itself.  This transition is for visual purposes as popping in 
		// and out of existance looks bad.
		float opacity;
	};

public: // MEMBER VARIABLES:
	int index = 0;
	Tile *sibling;

	Type type;
	Force force;
	Entity entity;

	SideInfos sideInfos;

	// only one position value is needed since the other three corners 
	// can be caluclated using only the maxVert and tileType variables.
	glm::ivec3 maxVert;

	// Used for decoration/organization.  
	// Colors the tile a certain color/pattern.	
	glm::fvec3 color;

public: // FUNCTIONS:

	//glm::ivec3 getVertPos(int index);
};