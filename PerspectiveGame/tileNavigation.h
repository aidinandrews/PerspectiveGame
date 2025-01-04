#pragma once
#include <iostream>

#include"dependancyHeaders.h"

#define NO_ENTITY_INDEX -1
#define NO_TILE_INDEX -1
#define NO_SUB_TILE_INDEX -1

enum TileRelation {
	TILE_RELATION_FLAT,
	TILE_RELATION_UP,
	TILE_RELATION_DOWN,
	TILE_RELATION_FLIPPED,
};

// designates how to translate one direction from one basis to another.
// can be thought of as the basis or a LocalDirection.
enum BasisType {
	BASIS_TYPE_0 = 0, 
	BASIS_TYPE_1 = 1,  
	BASIS_TYPE_2 = 2,  
	BASIS_TYPE_3 = 3,  
	BASIS_TYPE_4 = 4,  
	BASIS_TYPE_5 = 5,  
	BASIS_TYPE_ERROR,
};

// Each tile has a type corrosponding to what principal/coordinate plane (XY, XA, and YZ) the tile is 
// parallel to.  This type means that a tile can be fully described with a type and a point on the 
// corrosponding coordinaate plane that corrosponds to one of it's vertices.
enum SuperTileType {
	TILE_TYPE_XY = 0,
	TILE_TYPE_XZ = 1,
	TILE_TYPE_YZ = 2,
};

using TileType = BasisType;
#define TILE_TYPE_XYF BASIS_TYPE_0 // Lies in the XY plan and faces Z+.
#define TILE_TYPE_XYB BASIS_TYPE_1 // Lies in the XY plan and faces Z-.
#define TILE_TYPE_XZF BASIS_TYPE_2 // Lies in the XZ plan and faces Y+.
#define TILE_TYPE_XZB BASIS_TYPE_3 // Lies in the XZ plan and faces Y-.
#define TILE_TYPE_YZF BASIS_TYPE_4 // Lies in the YZ plan and faces X+.
#define TILE_TYPE_YZB BASIS_TYPE_5 // Lies in the YZ plan and faces X-.
#define TILE_TYPE_ERROR BASIS_TYPE_ERROR
#define TILE_TYPE_DEFAULT TILE_TYPE_XYF

enum MapType {
	MAP_TYPE_IDENTITY		= 0, // [0, 1, 2, 3] -> [0, 1, 2, 3] (identity)
	MAP_TYPE_CW_ROT			= 1, // [0, 1, 2, 3] -> [1, 2, 3, 0] (countercockwise rotation)
	MAP_TYPE_DBL_ROT		= 2, // [0, 1, 2, 3] -> [2, 3, 0, 1] (double rotation)
	MAP_TYPE_CCW_ROT		= 3, // [0, 1, 2, 3] -> [3, 0, 1, 2] (clockwise rotation)
	MAP_TYPE_HORI_FLIP		= 4, // [0, 1, 2, 3] -> [0, 3, 2, 1] (horizontal axis flip)
	MAP_TYPE_MIN_DIAG_FLIP	= 5, // [0, 1, 2, 3] -> [1, 0, 3, 2] (minor diagonal axis flip)
	MAP_TYPE_VERT_FLIP		= 6, // [0, 1, 2, 3] -> [2, 1, 0, 3] (vertical axis flip)
	MAP_TYPE_MAJ_DIAG_FLIP	= 7, // [0, 1, 2, 3] -> [3, 2, 1, 0] (major diagonal axis flip)
	MAP_TYPE_ERROR,
};

//enum BasisType {
//	BASIS_TYPE_NONE,
//	BASIS_TYPE_PRODUCER,
//	BASIS_TYPE_CONSUMER,
//	BASIS_TYPE_FORCE_SINK,
//	BASIS_TYPE_FORCE_GENERATOR,
//};

enum EntityType {
	ENTITY_TYPE_NONE,
	ENTITY_TYPE_BASIC,
};

// Given a basis, specifies a direction in that basis, be it in neighbors, direction, ect.
enum LocalAlignment {
	LOCAL_ALIGNMENT_0, 
	LOCAL_ALIGNMENT_1, 
	LOCAL_ALIGNMENT_2, 
	LOCAL_ALIGNMENT_3,
	LOCAL_ALIGNMENT_0_1,
	LOCAL_ALIGNMENT_1_2,
	LOCAL_ALIGNMENT_2_3,
	LOCAL_ALIGNMENT_3_0,
	LOCAL_ALIGNMENT_NONE, 
	LOCAL_ALIGNMENT_ERROR
};
// It might make more sense at some point to refer to the diagonal alignment values by a different order:
#define LOCAL_ALIGNMENT_1_0 LOCAL_ALIGNMENT_0_1
#define LOCAL_ALIGNMENT_2_1 LOCAL_ALIGNMENT_1_2
#define LOCAL_ALIGNMENT_3_2 LOCAL_ALIGNMENT_2_3
#define LOCAL_ALIGNMENT_0_3 LOCAL_ALIGNMENT_3_0

typedef LocalAlignment LocalPosition;
#define LOCAL_POSITION_0 LOCAL_ALIGNMENT_0
#define LOCAL_POSITION_1 LOCAL_ALIGNMENT_1
#define LOCAL_POSITION_2 LOCAL_ALIGNMENT_2
#define LOCAL_POSITION_3 LOCAL_ALIGNMENT_3
#define LOCAL_POSITION_0_1 LOCAL_ALIGNMENT_0_1
#define LOCAL_POSITION_1_2 LOCAL_ALIGNMENT_1_2
#define LOCAL_POSITION_2_3 LOCAL_ALIGNMENT_2_3
#define LOCAL_POSITION_3_0 LOCAL_ALIGNMENT_3_0
#define LOCAL_POSITION_1_0 LOCAL_ALIGNMENT_0_1
#define LOCAL_POSITION_2_1 LOCAL_ALIGNMENT_1_2
#define LOCAL_POSITION_3_2 LOCAL_ALIGNMENT_2_3
#define LOCAL_POSITION_0_3 LOCAL_ALIGNMENT_3_0
#define LOCAL_POSITION_CENTER LOCAL_ALIGNMENT_NONE
#define LOCAL_POSITION_ERROR LOCAL_ALIGNMENT_ERROR

typedef LocalAlignment LocalDirection;
#define LOCAL_DIRECTION_0 LOCAL_ALIGNMENT_0
#define LOCAL_DIRECTION_1 LOCAL_ALIGNMENT_1
#define LOCAL_DIRECTION_2 LOCAL_ALIGNMENT_2
#define LOCAL_DIRECTION_3 LOCAL_ALIGNMENT_3
#define LOCAL_DIRECTION_0_1 LOCAL_ALIGNMENT_0_1
#define LOCAL_DIRECTION_1_2 LOCAL_ALIGNMENT_1_2
#define LOCAL_DIRECTION_2_3 LOCAL_ALIGNMENT_2_3
#define LOCAL_DIRECTION_3_0 LOCAL_ALIGNMENT_3_0
#define LOCAL_DIRECTION_1_0 LOCAL_ALIGNMENT_0_1
#define LOCAL_DIRECTION_2_1 LOCAL_ALIGNMENT_1_2
#define LOCAL_DIRECTION_3_2 LOCAL_ALIGNMENT_2_3
#define LOCAL_DIRECTION_0_3 LOCAL_ALIGNMENT_3_0
#define LOCAL_DIRECTION_STATIC LOCAL_ALIGNMENT_NONE
#define LOCAL_DIRECTION_ERROR LOCAL_ALIGNMENT_ERROR

typedef LocalAlignment LocalOrientation;
#define LOCAL_ORIENTATION_0 LOCAL_ALIGNMENT_0
#define LOCAL_ORIENTATION_1 LOCAL_ALIGNMENT_1
#define LOCAL_ORIENTATION_2 LOCAL_ALIGNMENT_2
#define LOCAL_ORIENTATION_3 LOCAL_ALIGNMENT_3
#define LOCAL_ORIENTATION_NONE LOCAL_ALIGNMENT_NONE
#define LOCAL_ORIENTATION_ERROR LOCAL_ALIGNMENT_ERROR

// Tiles exist in 3D space, and thus can be described with cartesian coordinates x, y, and z. 
// Thus, there are 6 primary directions: left, right, forward, back, up, and down. 
// Less relative are: x+, x-, y+, y-, z+, z-.  
// The latter is what has been used for a naming convention in this enum.
enum GlobalAlignment {
	GLOBAL_ALIGNMENT_pX,
	GLOBAL_ALIGNMENT_nX,
	GLOBAL_ALIGNMENT_pY,
	GLOBAL_ALIGNMENT_nY,
	GLOBAL_ALIGNMENT_pZ,
	GLOBAL_ALIGNMENT_nZ,
	GLOBAL_ALIGNMENT_pXpY,
	GLOBAL_ALIGNMENT_pXnY,
	GLOBAL_ALIGNMENT_nXpY,
	GLOBAL_ALIGNMENT_nXnY,
	GLOBAL_ALIGNMENT_pXpZ,
	GLOBAL_ALIGNMENT_pXnZ,
	GLOBAL_ALIGNMENT_nXpZ,
	GLOBAL_ALIGNMENT_nXnZ,
	GLOBAL_ALIGNMENT_pYpZ,
	GLOBAL_ALIGNMENT_pYnZ,
	GLOBAL_ALIGNMENT_nYpZ,
	GLOBAL_ALIGNMENT_nYnZ,
	GLOBAL_ALIGNMENT_NONE,
	GLOBAL_ALIGNMENT_ERROR
};

namespace tnav { // tnav is short for 'tile navigation'

	const extern LocalAlignment LOCAL_ALIGNMENT_SET[9];
	const extern LocalDirection DIRECTION_SET[8];
	const extern glm::vec3 TO_NODE_OFFSETS[3][8];
	const extern LocalDirection ORTHOGONAL_DIRECTION_SET[4];
	const extern LocalDirection DIAGONAL_DIRECTION_SET[4];

	void checkOrthogonal(LocalAlignment alignment);
	
	// Given a local direction and a tile type, will return the global euclidean direction equivelant.
	GlobalAlignment localToGlobalDir(TileType type, LocalDirection direction);
	glm::vec3 globalDirToVec3(GlobalAlignment g);
	
	//LocalAlignment alignmentToAlignmentMap(TileType currentTileType, TileType neighborTileType, LocalDirection exitingSide, LocalAlignment alignment);
	//#define positionToPositionMap alignmentToAlignmentMap
	//#define directionToDirectionMap alignmentToAlignmentMap
	//#define orientationToOrientationMap alignmentToAlignmentMap
	
	LocalDirection inverse(LocalDirection direction);
	
	const LocalAlignment* getAlignmentComponents(LocalAlignment alignment);
	
	const bool alignmentHasComponent(LocalDirection direction, LocalDirection component);
	
	const LocalDirection combine(LocalAlignment a, LocalAlignment b);
	
	const LocalAlignment map(MapType mapType, LocalAlignment currentAlignment);

	const MapType getNeighborMap(LocalDirection connectedCurrentTileEdgeIndex, LocalDirection connectedNeighborEdgeIndex);

	const MapType combine
	(MapType map1, MapType map2);

	const MapType inverse(MapType mapType);

	LocalPosition nextPosition(LocalPosition pos, LocalDirection direction);

	// Returns the position after the next position in the given direction.
	const LocalPosition getNextNextPosition(LocalPosition pos, LocalDirection direction);

	inline void println(LocalDirection d)
	{
		switch (d) {
		case LOCAL_DIRECTION_0: std::cout << "LOCAL_DIRECTION_0"; break;
		case LOCAL_DIRECTION_1: std::cout << "LOCAL_DIRECTION_1"; break;
		case LOCAL_DIRECTION_2: std::cout << "LOCAL_DIRECTION_2"; break;
		case LOCAL_DIRECTION_3: std::cout << "LOCAL_DIRECTION_3"; break;
		case LOCAL_DIRECTION_0_1: std::cout << "LOCAL_DIRECTION_0_1"; break;
		case LOCAL_DIRECTION_1_2: std::cout << "LOCAL_DIRECTION_1_2"; break;
		case LOCAL_DIRECTION_2_3: std::cout << "LOCAL_DIRECTION_2_3"; break;
		case LOCAL_DIRECTION_3_0: std::cout << "LOCAL_DIRECTION_3_0"; break;
		case LOCAL_DIRECTION_STATIC: std::cout << "LOCAL_DIRECTION_STATIC"; break;
		case LOCAL_DIRECTION_ERROR: std::cout << "LOCAL_DIRECTION_INVALID"; break;
		default: std::cout << "OUT OF SCOPE";
		} std::cout << std::endl;
	}

	inline void println(TileType tt)
	{
		switch (tt) {
		case TILE_TYPE_XYF: std::cout << "TILE_TYPE_XYF" << std::endl; return;
		case TILE_TYPE_XYB: std::cout << "TILE_TYPE_XYB" << std::endl; return;
		case TILE_TYPE_XZF: std::cout << "TILE_TYPE_XZF" << std::endl; return;
		case TILE_TYPE_XZB: std::cout << "TILE_TYPE_XZB" << std::endl; return;
		case TILE_TYPE_YZF: std::cout << "TILE_TYPE_YZF" << std::endl; return;
		case TILE_TYPE_YZB: std::cout << "TILE_TYPE_YZB" << std::endl; return;
		} 
	}
	
	bool isOrthogonal(LocalDirection direction);
	bool isDiagonal(LocalDirection direction);

	const uint8_t getDirectionFlag(LocalDirection direction);
	const LocalDirection getDirection(uint8_t directionFlag);

	// Converts a Tile::Type to a TileSubType.
	const TileType getTileType(SuperTileType tileType, bool isFront);
	const TileType getFrontTileType(SuperTileType tileMapType);
	const TileType getBackTileType(SuperTileType tileMapType);

	// Given four vertices that will make up a tile, returns that potential tile's type.
	const SuperTileType getSuperTileType(glm::ivec3 tileVert1, glm::ivec3 tileVert2, glm::ivec3 tileVert3);

	const SuperTileType getSuperTileType(glm::ivec3 tileVert1, glm::ivec3 tileVert2, glm::ivec3 tileVert3);
	
	const SuperTileType getSuperTileType(TileType type);

	// Returns the 'opposite' TileSubType.  Front gets changed to back.
	// Ex: TILE_SUB_TYPE_XY_FRONT -> TILE_SUB_TYPE_XY_BACK
	const TileType inverseTileType(TileType type);

	// returns true if the tile is a 'front' tile and false if it is a 'back' tile.
	inline const bool isFront(TileType t) { return (int)t % 2 == 1; }

	// Given a tile type and a direciton, will return a pointer to a list of 4 tile types that
	// describe the possible connectable tile types of any connected tile.
	const TileType getConnectableTileType(TileType type, int orthogonalSide, int i);

	// Given a tile type and a direciton, will return a pointer to a list of 4 vec3 offsets that
	// describe the possible max points of any connected tile.
	const glm::ivec3 getConnectableTileOffset(TileType type, int orthogonalSide, int i);

	const int getTileVisibility(TileType subjetTileType, LocalDirection orthoSide, TileType otherTileType);

	// Given a diagonal alignment and one of its components, will return the other component.
	// LOCAL_ALIGNMENT_ERROR is returned if 'component' is not a component of diagonal or 'diagonal' is not a diagonal alignment.
	const LocalAlignment getOtherComponent(LocalAlignment diagonal, LocalAlignment component);

	const glm::vec3 getNormal(TileType type);
	const TileType getTileType(glm::vec3 normal);

	// returns a pointer to an array of size 4.
	// the array is a set of offsets from the center of a tile to each side node's 3D position.
	// ex: center_node_position + return[side_node_index] = side_node_position
	const glm::vec3* getNodePositionOffsets(SuperTileType type);

	inline const glm::vec3* getNodePositionOffsets(TileType type)
	{
		return getNodePositionOffsets(getSuperTileType(type));
	}

	const glm::vec3 getTileEdgeVec(TileType type, LocalDirection orthoDir);
	const glm::vec3 getCenterToEdgeVec(TileType type, LocalDirection orthoDir);

	// given two tiles, finds the priority of the potential connection.
	// note that there are some tile pairs that are not able to be  connected through
	// some edges.  This assumes a valid connection between the tiles!
	const int getConnectionPriority(TileType A, TileType B);
}
