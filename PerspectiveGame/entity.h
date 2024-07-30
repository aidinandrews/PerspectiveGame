#pragma once
#include <iostream>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

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

	glm::vec4 color;
	float opacity;

public: // MEMBER FUNCTIONS

	Entity() : type(NONE), isStatic(true), localDirection(LocalDirection::LOCAL_DIRECTION_0), localOrientation(LocalDirection::LOCAL_DIRECTION_0),
		opacity(0), followingEntityIndex(-1) {
		color = glm::vec4(1, 1, 1, 1);
	}

	Entity(Entity::Type t, bool is, LocalPosition lp, LocalDirection ld, LocalOrientation lo, float o, int lei, int fei, glm::vec4 c) {
		index = 0;
		tileIndex[0] = -1;
		tileIndex[1] = -1;
		type = t;
		isStatic = is;
		localPosition = lp;
		localDirection = ld;
		lastLocalDirection = LOCAL_DIRECTION_INVALID;
		localOrientation = lo;
		opacity = o;
		leadingEntityIndex = lei;
		followingEntityIndex = fei;
		color = c;
	}

	bool hasLeader() { return leadingEntityIndex != -1; }
	bool hasNoLeader() { return !hasLeader(); }

	bool hasFollower() { return followingEntityIndex != -1; }
	bool hasNoFollower() { return !hasFollower(); }

	bool inMiddlePosition() { return ((localPosition > 3) && (localPosition < 8)); }
	bool inEdgePosition() { return localPosition < 4; }
	bool inCenterPosition() { return localPosition == 8; }

	bool connectedToTile(bool index) { return tileIndex[index] != -1; }

	Entity& operator=(const Entity& other) {
		index = other.index;
		tileIndex[0] = other.tileIndex[0];
		tileIndex[1] = other.tileIndex[1];
		type = other.type;
		isStatic = other.isStatic;
		localPosition = other.localPosition;
		localDirection = other.localDirection;
		lastLocalDirection = other.lastLocalDirection;
		localOrientation = other.localOrientation;
		opacity = other.opacity;
		leadingEntityIndex = other.leadingEntityIndex;
		followingEntityIndex = other.followingEntityIndex;
		color = other.color;

		return *this;
	}
};

struct EntityGpuInfo {
	alignas(4) int type;
	alignas(4) int localOrientation;
	alignas(4) int localDirection;
	alignas(4) int lastLocalDirection;
	alignas(4) int isStatic;
	alignas(16) glm::vec4 color;

	EntityGpuInfo(Entity* entity) {
		type = entity->type;
		localOrientation = entity->localOrientation;
		localDirection = entity->localDirection;
		lastLocalDirection = entity->lastLocalDirection;
		isStatic = entity->isStatic;
		color = entity->color;
	}
};