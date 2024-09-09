#pragma once
#include <iostream>

#include "tileNavigation.h"

// Info about the inside of the tile is stored here.  Things like sub-tile obstruciton and movement of entities.
struct TileInternals {
private:
	/*
	* BREIF: Tiles are sub-divided into 16 squares called 'sub-tiles.'  Each sub tile is given 4 bits out of 64 to
	*        express some piece of information. Infomation includes direction of the entity that resides in it
	*        (entities take up 4 sub-tiles) or special properties of the sub tile (see key for further examples).
	*
	* VISUAL KEY:
	* {3}                       {0}
	*   ,-----;-----;-----;-----,
	*   | [0] | [1] | [2] | [3] |   [x] : Sub-Tile Info Index
	*   |-----|-----|-----|-----|	{x} : Corner Index
	*   | [4] | [5] | [6] | [7] |
	*   |-----|-----|-----|-----| == uint64_t(FEDCBA9876543210)
	*   | [8] | [9] | [A] | [B] |
	*   |-----|-----|-----|-----|
	*   | [C] | [D] | [E] | [F] |
	*   '-----'-----'-----'-----'
	* {2}                       {1}
	*
	* KEY: LOCAL_DIRECTION_0 == 0001
	*      LOCAL_DIRECTION_1 == 0010
	*      LOCAL_DIRECTION_2 == 0100
	*      LOCAL_DIRECTION_3 == 1000
	*      LOCAL_DIRECTION_0_1 == 0011
	*      LOCAL_DIRECTION_1_2 == 0110
	*      LOCAL_DIRECTION_2_3 == 1100
	*      LOCAL_DIRECTION_3_0 == 1001
	*      LOCAL_DIRECTION_STATIC == 1111
	*      LOCAL_DIRECTION_NONE == 0000
	*/ uint64_t movementInfos;

	// There needs to be a distinction between sub tile areas that have static entities vs no entities for movement
	// forces to propogate between invementInfos.
	uint16_t obstructionFlags;

public:
	TileInternals() { movementInfos = 0; }

	uint64_t get4BitDirectionFlag(LocalDirection direction);

	uint16_t getObstructionFlags() { return obstructionFlags; }

	uint64_t getAllLocalEntityInfos() { return movementInfos; }
	uint8_t getLocalEntityInfo(int subTileIndex);

	void addMovementInfo(int subTileIndex, LocalDirection direction);
	void addMovementInfo(LocalPosition position, LocalDirection direction);

	void removeMovementInfo(int subTileIndex, LocalDirection direction);
	void removeMovementInfo(LocalPosition position, LocalDirection direction);

	void clearMovementInfo(int subTileIndex);
	void clearMovementInfo(LocalPosition position);

	void setMovementInfo(int subTileIndex, LocalDirection direction);
	void setMovementInfo(LocalPosition position, LocalDirection direction);

	bool isObstructed(int subTileIndex);
};