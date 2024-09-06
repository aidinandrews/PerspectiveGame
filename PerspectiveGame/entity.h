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
public:
	int index; // Index into the entity vector in entityManager.
	EntityType type;
	glm::vec4 color;

private:
	// An entity can be in 1, 2, or 4 tiles.  NEVER 3 tiles.
	// info index scheme:
	// [0] == arriving tile info (tile the entity is going to be in if it continues to move in its direction).
	// [1] == leaving tile info or null (tile the entity is leaving if it continues to move in its direction).
	// [2] == adjacent tile info (when the entity is in a corner position and in 4 tiles at once).
	// [3] == adjacent tile info twin (as there must be another next to tile if the entity is in a corner).
	// There will always be ONLY 1 arriving/leaving tile as orthogonal movement on an edge is impossible given entity spawn/movement restrictions!

	int tileIndices[4]; 
	LocalPosition positions[4];
	LocalDirection directions[4];
	uint8_t directionFlags[4];
	LocalOrientation orientations[4];
	bool tileInfoIsLeavings[4];

	bool addedToCollisionList;

public:
	Entity(int entityIndex, int tileIndex, EntityType type, LocalDirection direction, LocalOrientation orientation, glm::vec4 color) 
	{
		this->type = type;
		this->index = entityIndex;
		this->color = color;

		tileIndices[0] = tileIndex;
		positions[0] = LOCAL_POSITION_CENTER;
		directions[0] = direction;
		directionFlags[0] = tnav::getDirectionFlag(direction);
		orientations[0] = orientation;
		tileInfoIsLeavings[0] = false;

		// Because entities MUST be created in the center of a tile, they cannot be in 
		// more than one tile on creation, meaning that these variables must be set to invalid:
		for (int i = 1; i < 4; i++) {
			clearTileInfo(i);
		}

		addedToCollisionList = false;
	}

	int getTileIndex(int infoIndex) { return tileIndices[infoIndex]; }

	LocalPosition getPosition(int infoIndex) { return positions[infoIndex]; }
	LocalDirection getDirection(int infoIndex) { return directions[infoIndex]; }
	uint8_t getDirectionFlag(int infoIndex) { return directionFlags[infoIndex]; }
	LocalOrientation getOrientation(int infoIndex) { return orientations[infoIndex]; }

	void setTileIndex(int infoIndex, int tileIndex) { tileIndices[infoIndex] = tileIndex; }
	void setPosition(int infoIndex, LocalPosition position) { positions[infoIndex] = position; }
	void setDirectionFlag(int infoIndex, uint8_t flag) { directionFlags[infoIndex] = flag; }
	void setDirection(int infoIndex, LocalDirection direction) { 
		directions[infoIndex] = direction; 
		setDirectionFlag(infoIndex, tnav::getDirectionFlag(direction));
	}
	void setOrientation(int infoIndex, LocalOrientation orientation) { orientations[infoIndex] = orientation; }
	
	void removeDirectionFlag(int infoIndex, uint8_t flag) { directionFlags[infoIndex] &= (~flag); }
	void addDirectionFlag(int infoIndex, uint8_t flag) { directionFlags[infoIndex] |= flag; }
	
	void makeTileInfoLeavings(int infoIndex) { tileInfoIsLeavings[infoIndex] = true; }
	void makeTileInfoGoings(int infoIndex) { tileInfoIsLeavings[infoIndex] = false; }
	bool isTileInfoLeavings(int infoIndex) { return tileInfoIsLeavings[infoIndex] == true; }

	bool isAddedToCollisionList() { return addedToCollisionList; }
	void setIsAddedToCollisionList(bool value) { addedToCollisionList = value; }

	bool isInTileEdge() { return positions[0] < 4; }
	bool isInTileCenter() { return positions[0] == LOCAL_POSITION_CENTER; }
	bool isInTileCorner() { return 3 < positions[0] && positions[0] < 8; }

	void setTileInfo(int infoIndex, int tileIndex, LocalPosition position, LocalDirection direction, LocalOrientation orientation)
	{
		tileIndices[infoIndex] = tileIndex;
		positions[infoIndex] = position;
		directions[infoIndex] = direction;
		directionFlags[infoIndex] = tnav::getDirectionFlag(direction);
		orientations[infoIndex] = orientation;
	}
	void clearTileInfo(int infoIndex)
	{
		tileIndices[infoIndex] = NO_TILE_INDEX;
		positions[infoIndex] = LOCAL_POSITION_ERROR;
		directions[infoIndex] = LOCAL_DIRECTION_ERROR;
		directionFlags[infoIndex] = tnav::getDirectionFlag(LOCAL_DIRECTION_ERROR);
		orientations[infoIndex] = LOCAL_ORIENTATION_ERROR;
		tileInfoIsLeavings[infoIndex] = false;
	}
	void swapTileInfos(int infoIndex0, int infoIndex1)
	{
		std::swap(tileIndices[infoIndex0], tileIndices[infoIndex1]);
		std::swap(positions[infoIndex0], positions[infoIndex1]);
		std::swap(directions[infoIndex0], directions[infoIndex1]);
		std::swap(directionFlags[infoIndex0], directionFlags[infoIndex1]);
		std::swap(orientations[infoIndex0], orientations[infoIndex1]);
		std::swap(tileInfoIsLeavings[infoIndex0], tileInfoIsLeavings[infoIndex1]);
	}
};

struct alignas(4) GPU_EntityTileInfo {
	alignas(4) int orientation;
	alignas(4) int direction;
	alignas(4) int position;
	alignas(4) int isLeavings;

	GPU_EntityTileInfo(int position, int direction, int orientation, int isLeavings) :
		orientation(orientation), direction(direction), position(position), isLeavings(isLeavings)
	{}
};

struct alignas(16) GPU_EntityInfo {
	alignas(16) glm::vec4 color;
	alignas(16) int tileInfoIndex[4];

	alignas(4) int type;
	alignas(4) int padding[3];

	GPU_EntityInfo(Entity* entity)
	{
		type = entity->type;
		color = entity->color;

		for (int i = 0; i < 4; i++) {
			tileInfoIndex[i] = -1;
			/*positions[i] = (int)entity->getPosition(i);
			directions[i] = (int)entity->getDirection(i);
			orientations[i] = (int)entity->getOrientation(i);
			isLeavings[i] = (int)entity->isTileInfoLeavings(i);*/
		}
	}
};