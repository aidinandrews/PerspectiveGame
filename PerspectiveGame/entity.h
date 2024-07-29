#pragma once
#include <iostream>

#include "tileNavigation.h"

// Each tile has the ability to 'hold' one entity.  These entities can be shuffled between tiles.  Entities
// can be either simple materials that are processed by buildings/machines or the machines themselves.
// Entities are moved with the application of force (supplied by a force block which is another entity).
// Entities consume force on movement based on their mass.  Continuous movement does not obec Newton's II.
// Movement is quantized to 0, 1, 2, and 4 tiles/tick with 0, 1/4, 1/2, 3/4, and 1 tiles/sub tick moved.
// This allows for much more straight forward and mechanic-driven collisions.  Overall, tiles are the space
// and entities are the objects inside that space.
struct Entity {

public: // MEMBER ENUMS AND MEMBER STRUCTS:

	// All objects can be moved, so buildings and materials
	// both fall under the 'entity' struct.
	enum Type {
		NONE,

		MATERIAL_OMNI,
		MATERIAL_A,
		MATERIAL_B,

		BUILDING_COMPRESSOR,
		BUILDING_FORCE_BLOCK,
		BUILDING_FORCE_MIRROR,
	};

private:

public: // MEMBER VARIABLES:

	int index; // Index into the entity vector in entityManager.
	int tileIndex[2]; // If the entity is on an edge, it 'resides' in both tiles. Hence [2].
	int leadingEntityIndex;
	int followingEntityIndex;

	Type type;

	bool           isStatic;
	LocalPosition  localPosition;
	LocalDirection localDirection;
	LocalDirection lastLocalDirection;
	LocalDirection localOrientation;

	float opacity;
	
public: // MEMBER FUNCTIONS

	Entity() : type(NONE), isStatic(true), localDirection(LocalDirection::LOCAL_DIRECTION_0), localOrientation(LocalDirection::LOCAL_DIRECTION_0),
		opacity(0), followingEntityIndex(-1)
	{}

	Entity(Entity::Type t, bool is, LocalDirection ld, LocalOrientation lo, float o, int lei, int fei) {
		type = t;
		isStatic = is;
		localDirection = ld;
		lastLocalDirection = LOCAL_DIRECTION_INVALID;
		localOrientation = lo;
		opacity = o;
		leadingEntityIndex = lei;
		followingEntityIndex = fei;
	}

	bool hasLeader() { return leadingEntityIndex != -1; }
	bool hasNoLeader() { return !hasLeader(); }

	bool hasFollower() { return followingEntityIndex != -1; }
	bool hasNoFollower() { return !hasFollower(); }

	static uint16_t localPosToObstructionMask(LocalPosition pos) {
		return ENTITY_LOCAL_POSITION_OBSTRUCTION_MAP_MASKS[pos];
	}
};