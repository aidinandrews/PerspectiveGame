#pragma once
#include <iostream>

#include"dependancyHeaders.h"

#define NO_ENTITY_INDEX -1
#define NO_TILE_INDEX -1
#define NO_SUB_TILE_INDEX -1

// Each tile has a type corrosponding to what principal/coordinate plane (XY, XA, and YZ) the tile is 
	// parallel to.  This type means that a tile can be fully described with a type and a point on the 
	// corrosponding coordinaate plane that corrosponds to one of it's vertices.
enum SuperTileType {
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
//enum TileDihedral {
//	TILE_DIHEDRAL_90,
//	TILE_DIHEDRAL_180,
//	TILE_DIHEDRAL_270,
//};

enum TileRelation {
	TILE_RELATION_FLAT,
	TILE_RELATION_UP,
	TILE_RELATION_DOWN,
	TILE_RELATION_FLIPPED,
};

// designates the plane something lies in and what direction it 'faces'
enum OrientationType {
	ORIENTATION_TYPE_0 = 0, // Lies in the XY plan and faces Z+.
	ORIENTATION_TYPE_1 = 1, // Lies in the XY plan and faces Z-. 
	ORIENTATION_TYPE_2 = 2, // Lies in the XZ plan and faces Y+. 
	ORIENTATION_TYPE_3 = 3, // Lies in the XZ plan and faces Y-. 
	ORIENTATION_TYPE_4 = 4, // Lies in the YZ plan and faces X+. 
	ORIENTATION_TYPE_5 = 5, // Lies in the YZ plan and faces X-. 
	ORIENTATION_TYPE_ERROR,
};

using TileType = OrientationType;
#define TILE_TYPE_XYF ORIENTATION_TYPE_0
#define TILE_TYPE_XYB ORIENTATION_TYPE_1
#define TILE_TYPE_XZF ORIENTATION_TYPE_2
#define TILE_TYPE_XZB ORIENTATION_TYPE_3
#define TILE_TYPE_YZF ORIENTATION_TYPE_4
#define TILE_TYPE_YZB ORIENTATION_TYPE_5
#define TILE_TYPE_ERROR ORIENTATION_TYPE_ERROR
#define TILE_TYPE_DEFAULT TILE_TYPE_XYF

//using NodeType = OrientationType;
//#define NODE_TYPE_XYF ORIENTATION_TYPE_0
//#define NODE_TYPE_XYB ORIENTATION_TYPE_1
//#define NODE_TYPE_XZF ORIENTATION_TYPE_2
//#define NODE_TYPE_XZB ORIENTATION_TYPE_3
//#define NODE_TYPE_YZF ORIENTATION_TYPE_4
//#define NODE_TYPE_YZB ORIENTATION_TYPE_5
//#define NODE_TYPE_ERROR ORIENTATION_TYPE_ERROR
//#define NODE_TYPE_DEFAULT NODE_TYPE_XYF

enum MapType {
	MAP_TYPE_0 = 0,
	MAP_TYPE_1 = 1,
	MAP_TYPE_2 = 2,
	MAP_TYPE_3 = 3,
	MAP_TYPE_4 = 4,
	MAP_TYPE_5 = 5,
	MAP_TYPE_6 = 6,
	MAP_TYPE_7 = 7,
	MAP_TYPE_ERROR,
};
#define MAP_TYPE_IDENTITY MAP_TYPE_0


//enum BasisType {
//	BASIS_TYPE_NONE,
//	BASIS_TYPE_PRODUCER,
//	BASIS_TYPE_CONSUMER,
//	BASIS_TYPE_FORCE_SINK,
//	BASIS_TYPE_FORCE_GENERATOR,
//};

enum EntityType {
	ENTITY_TYPE_NONE,
	ENTITY_TYPE_OMNI,
};

enum CornerBuildingType {
	CORNER_BUILDING_NONE,
	CORNER_BUILDING_BELT_MIDDLE_FORWARD,
	CORNER_BUILDING_BELT_MIDDLE_BACK,
	CORNER_BUILDING_BELT_END_FORWARD,
	CORNER_BUILDING_BELT_END_BACK,
};

// Specifies information about movement for various entities.
enum LocalAlignment {
	LOCAL_ALIGNMENT_0, LOCAL_ALIGNMENT_1, LOCAL_ALIGNMENT_2, LOCAL_ALIGNMENT_3,
	LOCAL_ALIGNMENT_0_1,LOCAL_ALIGNMENT_1_2,LOCAL_ALIGNMENT_2_3,LOCAL_ALIGNMENT_3_0,
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

	// Combines and returns map1 -> map2.
	// * Return may be different than map2 -> map1!
	const MapType combine(MapType map1, MapType map2);

	const MapType inverse(MapType mapType);

	LocalPosition nextPosition(LocalPosition position, LocalDirection direction);

	// Returns the position after the next position in the given direction.
	const LocalPosition getNextNextPosition(LocalPosition position, LocalDirection direction);

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

	const glm::vec3 getCenterToNeighborVec(TileType type, LocalDirection orthoDir);
}
