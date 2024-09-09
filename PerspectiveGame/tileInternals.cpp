#pragma once
#include "tileInternals.h"

uint64_t TileInternals::get4BitDirectionFlag(LocalDirection direction)
{
	switch (direction) {
		// Orthogonal:
	case LOCAL_DIRECTION_0: return (uint64_t)0b0001;
	case LOCAL_DIRECTION_1: return (uint64_t)0b0010;
	case LOCAL_DIRECTION_2: return (uint64_t)0b0100;
	case LOCAL_DIRECTION_3: return (uint64_t)0b1000;
		// Diagonal:
	case LOCAL_DIRECTION_0_1: return (uint64_t)0b0011;
	case LOCAL_DIRECTION_1_2: return (uint64_t)0b0110;
	case LOCAL_DIRECTION_2_3: return (uint64_t)0b1100;
	case LOCAL_DIRECTION_3_0: return (uint64_t)0b1001;
		// Special:
	case LOCAL_DIRECTION_STATIC: return (uint64_t)0b0000;
	//case LOCAL_DIRECTION_STATIC: return (uint64_t)0b1111;
		// Whoops:
	default: std::cout << "TRIED TO CONVERT INVALID LOCAL DIRECTION TO BIT FLAG!" << std::endl;
		return 0xFFFFFFFFFFFFFFFF;
	}
}

uint8_t TileInternals::getLocalEntityInfo(int subTileIndex)
{
	uint64_t mask = (uint64_t(0b1111) << (subTileIndex * 4));
	return uint8_t((movementInfos & mask) >> (subTileIndex * 4));
}

void TileInternals::addMovementInfo(int subTileIndex, LocalDirection direction)
{
	movementInfos |= (get4BitDirectionFlag(direction) << (subTileIndex * 4));
}
void TileInternals::addMovementInfo(LocalPosition position, LocalDirection direction)
{
	/*const int* subTileIndices = tnav::getSurroundingSubTileIndices(position);
	for (int i = 0; i < 4; i++) {
		addMovementInfo(subTileIndices[i], direction);
	}*/
}

void TileInternals::removeMovementInfo(int subTileIndex, LocalDirection direction)
{
	movementInfos &= ~(get4BitDirectionFlag(direction) << (subTileIndex * 4));
}
void TileInternals::removeMovementInfo(LocalPosition position, LocalDirection direction)
{
	/*const int* subTileIndices = tnav::getSurroundingSubTileIndices(position);
	for (int i = 0; i < 4; i++) {
		removeMovementInfo(subTileIndices[i], direction);
	}*/
}

void TileInternals::clearMovementInfo(int subTileIndex)
{
	movementInfos &= ~(uint64_t(0b1111) << (subTileIndex * 4));
}
void TileInternals::clearMovementInfo(LocalPosition position)
{
	/*const int* subTileIndices = tnav::getSurroundingSubTileIndices(position);
	for (int i = 0; i < 4; i++) {
		clearMovementInfo(subTileIndices[i]);
	}*/
}

void TileInternals::setMovementInfo(int subTileIndex, LocalDirection direction)
{
	clearMovementInfo(subTileIndex);
	addMovementInfo(subTileIndex, direction);
}
void TileInternals::setMovementInfo(LocalPosition position, LocalDirection direction)
{
	clearMovementInfo(position);
	addMovementInfo(position, direction);
}

bool TileInternals::isObstructed(int subTileIndex)
{
	return (movementInfos & (uint64_t(0b1111) << (subTileIndex * 4))) != 0;
}