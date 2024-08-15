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
public: // MEMBER VARIABLES:

	int index; // Index into the entity vector in entityManager.
	// If the entity is on an edge, it 'resides' in both tiles. Hence [2].
	// tileIndices[1] is always the 'leaving tile' or the only tile the entity is in.
	// tileIndices[0] is always the 'going tile' or -1 (not indexing anywhere).
	int tileIndices[2];
	int leaderListIndex;
	int leadingEntityIndex;
	int followingEntityIndex;

	// indexes into the static entity list in entityManager.  used for trying to promote static entities.
	// -1 if entity is not static.
	int staticListIndex; 

	EntityType type;

	LocalPosition  localPositions[2];
	LocalDirection localDirections[2];
	LocalDirection lastLocalDirections[2];
	LocalDirection localOrientations[2];

	glm::vec4 color;
	float opacity;

public: // MEMBER FUNCTIONS

	Entity(EntityType type, LocalPosition position, LocalDirection direction, LocalOrientation orientation, 
		float opacity, int leaderIndex, int followerIndex, glm::vec4 color) {

		this->type = type;

		this->index = 0;
		this->tileIndices[0] = -1;
		this->tileIndices[1] = -1;
		this->leaderListIndex = -1;
		this->staticListIndex = -1;

		this->localPositions[0] = position;
		this->localPositions[1] = LOCAL_POSITION_INVALID;
		this->localDirections[0] = direction;
		this->localDirections[1] = LOCAL_DIRECTION_INVALID;
		this->lastLocalDirections[0] = LOCAL_DIRECTION_INVALID;
		this->lastLocalDirections[1] = LOCAL_DIRECTION_INVALID;
		this->localOrientations[0] = orientation;
		this->localOrientations[1] = LOCAL_DIRECTION_INVALID;

		this->opacity = opacity;
		this->leadingEntityIndex = leaderIndex;
		this->followingEntityIndex = followerIndex;
		this->color = color;
	}

	bool isStatic() { return staticListIndex != -1; }

	bool isLeader() { return leaderListIndex >= 0; }
	bool hasLeader() { return leadingEntityIndex != -1; }
	bool hasNoLeader() { return !hasLeader(); }

	bool hasFollower() { return followingEntityIndex != -1; }
	bool hasNoFollower() { return !hasFollower(); }

	bool inMiddlePosition() { return ((localPositions[0] > 3) && (localPositions[0] < 8)); }
	bool inEdgePosition() { return localPositions[0] < 4; }
	bool inCenterPosition() { return localPositions[0] == 8; }

	bool connectedToTile(bool index) { return tileIndices[index] != -1; }

	bool hasDirection() { return localDirections[0] < 4; }

	bool movingToEdge() { return inMiddlePosition() && (localDirections[0] == (localPositions[0] - 4)); }
	bool movingToCenter() { return inMiddlePosition() && (localDirections[0] == ((localPositions[0] - 2) % 4)); }

	/*Entity& operator=(const Entity& other) {
		index = other.index;
		tileIndex[0] = other.tileIndex[0];
		tileIndex[1] = other.tileIndex[1];
		type = other.type;
		localPosition = other.localPosition;
		localDirection = other.localDirection;
		lastLocalDirection = other.lastLocalDirection;
		localOrientation = other.localOrientation;
		opacity = other.opacity;
		leadingEntityIndex = other.leadingEntityIndex;
		followingEntityIndex = other.followingEntityIndex;
		color = other.color;

		return *this;
	}*/
};

struct alignas(16) EntityGpuInfo {
	alignas(16) glm::vec4 color;

	alignas(8) int localOrientation[2];
	alignas(8) int localDirection[2];
	
	alignas(8) int lastLocalDirection[2];
	alignas(4) int type;
	alignas(4) int padding;
	

	EntityGpuInfo(Entity* entity) {
		type = entity->type;
		for (int i = 0; i < 2; i++) {
			localOrientation[i] = entity->localOrientations[i];
			localDirection[i] = entity->localDirections[i];
			lastLocalDirection[i] = entity->lastLocalDirections[i];
		}

		if (entity->isLeader())
			color = glm::vec4(0, 1, 0, 1);
		else
			color = glm::vec4(0, 0, 0, 1);

		//color = entity->color;
	}
};