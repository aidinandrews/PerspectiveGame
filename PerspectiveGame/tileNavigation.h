#pragma once
#include <iostream>

#define NO_ENTITY_INDEX -1
#define NO_TILE_INDEX -1
#define NO_SUB_TILE_INDEX -1

// Each tile has a type corrosponding to what principal/coordinate plane (XY, XA, and YZ) the tile is 
	// parallel to.  This type means that a tile can be fully described with a type and a point on the 
	// corrosponding coordinaate plane that corrosponds to one of it's vertices.
enum TileType {
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

// Along with the Tile::Type (XY, XZ, YZ), each tile has a direction (FRONT/BACK). This lets us know how we 
// should be looking at the tile, as each tile has a 'sibling' that faces the opposite direction.
enum TileSubType {
	TILE_TYPE_XYF,
	TILE_TYPE_XYB,
	TILE_TYPE_XZF,
	TILE_TYPE_XZB,
	TILE_TYPE_YZF,
	TILE_TYPE_YZB,
};

enum BasisType {
	BASIS_TYPE_NONE,
	BASIS_TYPE_PRODUCER,
	BASIS_TYPE_CONSUMER,
	BASIS_TYPE_FORCE_SINK,
	BASIS_TYPE_FORCE_GENERATOR,
};

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
	LOCAL_ALIGNMENT_0, 
	LOCAL_ALIGNMENT_1, 
	LOCAL_ALIGNMENT_2, 
	LOCAL_ALIGNMENT_3,
	LOCAL_ALIGNMENT_01,
	LOCAL_ALIGNMENT_12,
	LOCAL_ALIGNMENT_23,
	LOCAL_ALIGNMENT_30,
	LOCAL_ALIGNMENT_NONE, 
	LOCAL_ALIGNMENT_ERROR
};

typedef LocalAlignment LocalPosition;
#define LOCAL_POSITION_0 LOCAL_ALIGNMENT_0
#define LOCAL_POSITION_1 LOCAL_ALIGNMENT_1
#define LOCAL_POSITION_2 LOCAL_ALIGNMENT_2
#define LOCAL_POSITION_3 LOCAL_ALIGNMENT_3
#define LOCAL_POSITION_01 LOCAL_ALIGNMENT_01
#define LOCAL_POSITION_12 LOCAL_ALIGNMENT_12
#define LOCAL_POSITION_23 LOCAL_ALIGNMENT_23
#define LOCAL_POSITION_30 LOCAL_ALIGNMENT_30
#define LOCAL_POSITION_CENTER LOCAL_ALIGNMENT_NONE
#define LOCAL_POSITION_ERROR LOCAL_ALIGNMENT_ERROR

typedef LocalAlignment LocalDirection;
#define LOCAL_DIRECTION_0 LOCAL_ALIGNMENT_0
#define LOCAL_DIRECTION_1 LOCAL_ALIGNMENT_1
#define LOCAL_DIRECTION_2 LOCAL_ALIGNMENT_2
#define LOCAL_DIRECTION_3 LOCAL_ALIGNMENT_3
#define LOCAL_DIRECTION_01 LOCAL_ALIGNMENT_01
#define LOCAL_DIRECTION_12 LOCAL_ALIGNMENT_12
#define LOCAL_DIRECTION_23 LOCAL_ALIGNMENT_23
#define LOCAL_DIRECTION_30 LOCAL_ALIGNMENT_30
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
	
	// Given a local direction and a tile type, will return the global euclidean direction equivelant.
	GlobalAlignment localToGlobalDir(TileSubType type, LocalDirection direction);
	
	LocalAlignment alignmentToAlignmentMap(TileSubType currentTileType, TileSubType neighborTileType, LocalDirection exitingSide, LocalAlignment alignment);
	#define positionToPositionMap alignmentToAlignmentMap
	#define directionToDirectionMap alignmentToAlignmentMap
	#define orientationToOrientationMap alignmentToAlignmentMap
	
	LocalDirection oppositeAlignment(LocalDirection direction);
	#define oppositeDirection oppositeAlignment
	#define oppositePosition oppositeAlignment
	#define oppositeOrientation oppositeAlignment
	
	const LocalAlignment* localAlignmentComponents(LocalAlignment alignment);
	#define directionComponents localAlignmentComponents
	#define positionComponents localAlignmentComponents
	#define orientationComponents localAlignmentComponents
	
	const bool localAlignmentHasComponent(LocalDirection direction, LocalDirection component);
	#define directionHasComponent localAlignmentHasComponent
	#define positionHasComponent localAlignmentHasComponent
	#define orientationHasComponent localAlignmentHasComponent
	
	const LocalDirection combineLocalAlignments(LocalAlignment a, LocalAlignment b);
	#define combineDirections combineLocalAlignments
	#define combinePositions combineLocalAlignments
	#define combineOrientations combineLocalAlignments
	
	LocalPosition nextPosition(LocalPosition position, LocalDirection direction);

	inline void println(LocalDirection d)
	{
		switch (d) {
		case LOCAL_DIRECTION_0: std::cout << "LOCAL_DIRECTION_0"; break;
		case LOCAL_DIRECTION_1: std::cout << "LOCAL_DIRECTION_1"; break;
		case LOCAL_DIRECTION_2: std::cout << "LOCAL_DIRECTION_2"; break;
		case LOCAL_DIRECTION_3: std::cout << "LOCAL_DIRECTION_3"; break;
		case LOCAL_DIRECTION_01: std::cout << "LOCAL_DIRECTION_01"; break;
		case LOCAL_DIRECTION_12: std::cout << "LOCAL_DIRECTION_12"; break;
		case LOCAL_DIRECTION_23: std::cout << "LOCAL_DIRECTION_23"; break;
		case LOCAL_DIRECTION_30: std::cout << "LOCAL_DIRECTION_30"; break;
		case LOCAL_DIRECTION_STATIC: std::cout << "LOCAL_DIRECTION_STATIC"; break;
		case LOCAL_DIRECTION_ERROR: std::cout << "LOCAL_DIRECTION_INVALID"; break;
		default: std::cout << "OUT OF SCOPE";
		} std::cout << std::endl;
	}
	
	bool isOrthogonal(LocalDirection direction);
	bool isDiagonal(LocalDirection direction);

	const uint8_t getDirectionFlag(LocalDirection direction);
	const LocalDirection getDirection(uint8_t directionFlag);

}
