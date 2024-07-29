#pragma once
#include <iostream>

// Along with the Tile::Type (XY, XZ, YZ), each tile has a direction (FRONT/BACK). This lets us know how we 
// should be looking at the tile, as each tile has a 'sibling' that faces the opposite direction.
enum TileSubType {
	TILE_TYPE_XY_FRONT = 0,
	TILE_TYPE_XY_BACK = 1,
	TILE_TYPE_XZ_FRONT = 2,
	TILE_TYPE_XZ_BACK = 3,
	TILE_TYPE_YZ_FRONT = 4,
	TILE_TYPE_YZ_BACK = 5,
};

// Each entity has one of 9 positions on the tile (see ascii diagram below), but some positions are
	// not valid depending on tile type.  Positions ZERO and ONE are invalid for front-facing tiles and
	// positions TWO and THREE are invalid for back-facing tiles.  If an entity moves to an invalid space,
	// it must be transfered to the tile's neighbor, hence the NIEGHBOR_ prefix.  If an entity is at a valid
	// edge already and moves further away from the tile, it must be transfered to a neighboring tile but
	// inward from that neighbor's edge, so NEIGHBOR_ZERO_ZERO means to transfer the entity onto the neighbor
	// in the zero direction, then move it again in that direction (whatever the new local direction is) once
	// on the neighboring tile.
	// 
	//  __________3__________
	//  |    |    |    |    |
	//	|____|____7____|____|
	//  |    |    |    |    |
	//	2____6____8____4____0
	//  |    |    |    |    |
	//	|____|____5____|____|
	//  |    |    |    |    |
	//	|____|____1____|____|
	// 
	// Notice that the positions on the outer edges of the tile match the enums that describe local tile directions.
	// This allows for more simple transitions from one tile to another.
enum LocalPosition {
	LOCAL_POSITION_EDGE_0, LOCAL_POSITION_EDGE_1, LOCAL_POSITION_EDGE_2, LOCAL_POSITION_EDGE_3,
	LOCAL_POSITION_MIDDLE_0, LOCAL_POSITION_MIDDLE_1, LOCAL_POSITION_MIDDLE_2, LOCAL_POSITION_MIDDLE_3,
	LOCAL_POSITION_CENTER,
	LOCAL_POSITION_INVALID,
};

// Directions are pointing in the corrisponding 
enum LocalDirection {
	LOCAL_DIRECTION_0, LOCAL_DIRECTION_1, LOCAL_DIRECTION_2, LOCAL_DIRECTION_3,
	LOCAL_DIRECTION_STATIC, LOCAL_DIRECTION_INVALID
};
#define LocalOrientation LocalDirection

// Tiles exist in 3D space, and thus can be described with cartesian coordinates x, y, and z. 
	// Thus, there are 6 primary directions: left, right, forward, back, up, and down. 
	// Less relative are: x+, x-, y+, y-, z+, z-.  
	// The latter is what has been used for a naming convention in this enum.
enum GlobalDirection {
	X_POSITIVE, X_NEGATIVE,
	Y_POSITIVE, Y_NEGATIVE,
	Z_POSITIVE, Z_NEGATIVE
};

const uint16_t ENTITY_LOCAL_POSITION_OBSTRUCTION_MAP_MASKS[9] = {
	0b0000000100010000, // Edge   0
	0b0000000000000110, // Edge   1
	0b0000100010000000, // Edge   2
	0b0110000000000000, // Edge   3
	0b0000001100110000, // Middle 0
	0b0000000001100110, // Middle 1
	0b0000110011000000, // Middle 2
	0b0110011000000000, // Middle 3
	0b0000011001100000, // Center
};

struct TileNavigator {
private:
	// Returns the adjusted local orientation of an entity/basis when going from one tile to another.
	static const LocalDirection ORIENTATION_TO_ORIENTATION_MAP[6][6][4][4];

	// Given a local direction and a tile type, will return the global euclidean direction equivelant.
	static const GlobalDirection LOCAL_DIR_TO_GLOBAL_DIR[6][4];

	// Gives the next position given the tile's facing direction, a current position, and a direction.
	// format: NEXT_POSITION[isFrontFacing][currentPosition][direction].
	static const LocalPosition NEXT_LOCAL_POSITION[9][4];

public:
	// Returns the adjusted local direction of an entity/basis when going from one tile to another.
	inline static LocalDirection dirToDirMap(TileSubType currentTileType, TileSubType neighborTileType, LocalDirection exitingSide) {
		return TileNavigator::ORIENTATION_TO_ORIENTATION_MAP[currentTileType][neighborTileType][exitingSide][exitingSide];
	}

	// Returns the adjusted relative orientation of an entity/basis when going from one tile to another.
	inline static LocalDirection orientationToOrientationMap(TileSubType currentTileType, TileSubType neighborTileType, LocalDirection exitingSide, LocalDirection currentOrientation) {
		return TileNavigator::ORIENTATION_TO_ORIENTATION_MAP[currentTileType][neighborTileType][exitingSide][currentOrientation];
	}

	// Given a local direction and a tile type, will return the global euclidean direction equivelant.
	inline static GlobalDirection localToGlobalDir(TileSubType type, LocalDirection direction) {
		return LOCAL_DIR_TO_GLOBAL_DIR[type][direction];
	}

	// Given a local position and a direction, will give the immediate next position in that direction.  
	// If the next position is impossible (i.e. trying to go in direction 1/3 at position Edge_0), INVALID_POSIITON will be returned.
	// FIX THIS: If the next position is inside of a neighboring tile, INVALID_POSITION will be returned.
	inline static LocalPosition nextLocalPosition(LocalPosition localPos, LocalDirection direction) {
		return NEXT_LOCAL_POSITION[localPos][direction];
	}

	inline static LocalDirection oppositeDirection(LocalDirection currentDirection) {
		return LocalDirection((int(currentDirection) + 2) % 4);
	}
};
