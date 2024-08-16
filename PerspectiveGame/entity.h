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
	EntityType type;
	glm::vec4 color;

private:
	// If the entity is on an edge, it 'resides' in both tiles. Hence [2].
	// tileIndices[1] is always the 'leaving tile' or the only tile the entity is in.
	// tileIndices[0] is always the 'going tile' or -1 (not indexing anywhere).
	int tileIndices[2];
	LocalPosition  localPositions[2];
	LocalDirection localDirections[2];
	LocalOrientation localOrientations[2];

public: // MEMBER FUNCTIONS

	Entity(int entityIndex, int tileIndex, EntityType type, LocalDirection direction,
		   LocalOrientation orientation, glm::vec4 color) : type(type), color(color), index(entityIndex)
	{

		tileIndices[0] = tileIndex;
		localPositions[0] = LOCAL_POSITION_CENTER; // This keeps collision ties impossible.
		localDirections[0] = direction;
		localOrientations[0] = orientation;

		// Because entities MUST be created in the center of a tile, they cannot be in 
		// two tiles at once and thus cannot have valid info in the following variables:
		tileIndices[1] = -1;
		localPositions[1] = LOCAL_POSITION_INVALID;
		localDirections[1] = LOCAL_DIRECTION_INVALID;
		localOrientations[1] = LOCAL_DIRECTION_INVALID;
	}

	int getTileIndex() { return tileIndices[0]; }
	LocalPosition getPosition() { return localPositions[0]; }
	LocalOrientation getOrientation() { return localOrientations[0]; }
	LocalDirection getDirection() { return localDirections[0]; }

	void setTileIndex(int index) { tileIndices[0] = index; }
	void setPosition(LocalPosition position) { localPositions[0] = position; }
	void setDirection(LocalDirection direction) { localDirections[0] = direction; }
	void setOrientation(LocalOrientation orientation) { localOrientations[0] = orientation; }

	// - 'Leaving' designates that this tile is the tile the entity is moving away from.
	// - Entity MUST be in an edge position/have 2 tiles it has information on.
	int getLeavingTileIndex() { return tileIndices[1]; }
	// - 'Leaving' designates return is the local position relative to the tile the entity is moving away from.
	// - Entity MUST be in an edge position/have 2 tiles it has information on.
	LocalPosition getLeavingPosition() { return localPositions[1]; }
	// - 'Leaving' designates return is the local direction relative to the tile the entity is moving away from.
	// - Entity MUST be in an edge position/have 2 tiles it has information on.
	LocalDirection getLeavingDirection() { return localDirections[1]; }
	// - 'Leaving' designates return is the local orientation relative to the tile the entity is moving away from.
	// - Entity MUST be in an edge position/have 2 tiles it has information on.
	LocalOrientation getLeavingOrientation() { return localOrientations[1]; }

	// - 'Leaving' designates that this tile is the tile the entity is moving away from.
	// - Entity MUST be in an edge position/have 2 tiles it has information on.
	void setLeavingTileIndex(int index) { tileIndices[1] = index; }
	// - 'Leaving' designates return is the local position relative to the tile the entity is moving away from.
	// - Entity MUST be in an edge position/have 2 tiles it has information on.
	void setLeavingPosition(LocalPosition position) { localPositions[1] = position; }
	// - 'Leaving' designates return is the local direction relative to the tile the entity is moving away from.
	// - Entity MUST be in an edge position/have 2 tiles it has information on.
	void setLeavingDirection(LocalDirection direction) { localDirections[1] = direction; }
	// - 'Leaving' designates return is the local orientation relative to the tile the entity is moving away from.
	// - Entity MUST be in an edge position/have 2 tiles it has information on.
	void setLeavingOrientation(LocalOrientation orientation) { localOrientations[1] = orientation; }

	void swapArrivingAndLeavingInfos()
	{
		std::swap(tileIndices[0], tileIndices[1]);
		std::swap(localPositions[0], localPositions[1]);
		std::swap(localDirections[0], localDirections[1]);
		std::swap(localOrientations[0], localOrientations[1]);
	}

	bool inMiddlePosition() { return ((localPositions[0] > 3) && (localPositions[0] < 8)); }
	bool inEdgePosition() { return localPositions[0] < 4; }
	bool inCenterPosition() { return localPositions[0] == 8; }

	bool connectedToTile(bool index) { return tileIndices[index] != -1; }

	bool hasDirection() { return localDirections[0] < 4; }

	bool movingToEdge() { return inMiddlePosition() && (localDirections[0] == (localPositions[0] - 4)); }
	bool movingToCenter() { return inMiddlePosition() && (localDirections[0] == ((localPositions[0] - 2) % 4)); }
};

struct alignas(16) EntityGpuInfo {
	alignas(16) glm::vec4 color;

	alignas(8)  int localOrientation[2];
	alignas(8)  int localDirection[2];
			   
	alignas(8)  int position[2];
	alignas(4)  int type;
	alignas(4)  int padding;

	EntityGpuInfo(Entity* entity)
	{
		type = entity->type;
		color = entity->color;
		
		position[0] = entity->getPosition();
		localDirection[0] = entity->getDirection();
		localOrientation[0] = entity->getOrientation();
		
		if (entity->inEdgePosition()) {
			position[1] = entity->getLeavingPosition();
			localDirection[1] = entity->getLeavingDirection();
			localOrientation[1] = entity->getLeavingOrientation();
		}
		else {
			position[1] = LOCAL_POSITION_INVALID;
			localDirection[1] = LOCAL_DIRECTION_INVALID;
			localOrientation[1] = LOCAL_ORIENTATION_INVALID;
		}

		padding = 0;
	}
};