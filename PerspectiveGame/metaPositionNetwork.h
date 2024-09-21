#pragma once

#include <iostream>
#include <vector>

#include "tileNavigation.h"

struct SuperPositionTileInfo {
	int tileIndex;
	LocalPosition position;
	int alignmentMapIndex;
};

enum SuperPositionType {
	SUPER_POSITION_TYPE_CENTER,
	SUPER_POSITION_TYPE_SIDE,
	SUPER_POSITION_TYPE_CORNER,
	SUPER_POSITION_TYPE_ERROR,
};

struct SuperPosition {
public:
	SuperPositionType type;
	int index;

public:
	SuperPosition() : type(SUPER_POSITION_TYPE_ERROR), index(-1) {}

	virtual SuperPosition* getNeighbor(LocalDirection dir) {}
	virtual void setNeighbor(LocalDirection dir, CenterSuperPosition* superPosition) {}
	virtual void setNeighbor(LocalDirection dir, SideSuperPosition* superPosition) {}
	virtual void setNeighbor(LocalDirection dir, CornerSuperPosition* superPosition) {}
	virtual void setNeighborToNull(LocalDirection dir) {}

	virtual int getNeighborAlignmentMapIndex(LocalDirection dir) {}
	virtual void setNeighborAlignmentMapIndex(LocalDirection dir, int mapIndex) {}

	virtual int getTileIndex(LocalDirection dir) {}
	virtual void setTileIndex(LocalDirection dir, int tileIndex) {}

	virtual LocalPosition getTilePosition(LocalDirection dir) {}
	virtual void setTilePosition(LocalDirection dir, LocalPosition pos) {}

	virtual int getTileAlignmentMap(LocalDirection dir) {}
	virtual void setTileAlignmentMap(LocalDirection dir, int mapIndex) {}

	virtual float getDirectionMagnitude(LocalDirection dir) {}
	virtual void setDirectionMagnitude(LocalDirection dir, float mag) {}

};

struct SideSuperPosition;
struct CornerSuperPosition;

// ON CREATION OF A TILE PAIR:
// make a list of all the meta super positions they touch (A),
// make a list of all the meta super position neighbors of A (B),
// delete all super positions in A,
// create a new set of super positions that touch the pair and add them to B.
// * if a super position would be unsafe, skip it's creation.
// update/reconnect all super positions in B.

// ON DELETION OF A TILE PAIR:
// make a list of all the meta super positions (A),
// make a list of all the meta super position neighbors of A (B),
// make a list of (tile index + local SIDE position) (C) that:
//    point to super positions in the same 3D position 
//    connect to tiles other than the pair
// delete all super positions of A,
// for each pair of positions in C, create a new super position and add it to B,
// update/reconnect all super positions in B.
//

// Tiles point to 1 of these
// Exactly 2 overlapping center super positions in the same 3D position due to sibling tiles.
struct CenterSuperPosition : public SuperPosition {
	int tileIndex;
	// Position in tile is always LOCAL_POSITION_CENTER.
	// super position->tile map is always identity.
	
private:
	float directionMagnitudes[4];
	SideSuperPosition* sideNeighbors[4];
	CornerSuperPosition* cornerNeighbors[4];
	int neighborAlignmentMaps[8];

public:
	CenterSuperPosition(SuperPositionType type, int tileIndex)
	{
		type = SUPER_POSITION_TYPE_CENTER;

		this->tileIndex = tileIndex;
		for (int i = 0; i < 4; i++) { 
			directionMagnitudes[i] = 0; 
			sideNeighbors[i] = nullptr;
			cornerNeighbors[i] = nullptr;
		}
		for (int i = 0; i < 8; i++) {
			neighborAlignmentMaps[i] = -1;
		}
	}

	SuperPosition* getNeighbor(LocalDirection dir) override 
	{
		switch (dir) {
		case LOCAL_DIRECTION_1: return sideNeighbors[0];
		case LOCAL_DIRECTION_2: return sideNeighbors[1];
		case LOCAL_DIRECTION_3: return sideNeighbors[2];
		case LOCAL_DIRECTION_0: return sideNeighbors[3];
		case LOCAL_DIRECTION_0_1: return cornerNeighbors[0];
		case LOCAL_DIRECTION_1_2: return cornerNeighbors[1];
		case LOCAL_DIRECTION_2_3: return cornerNeighbors[2];
		case LOCAL_DIRECTION_3_0: return cornerNeighbors[3];
		default: return nullptr;
		}
	}
	void setNeighbor(LocalDirection dir, SideSuperPosition* superPosition) override 
	{
		switch (dir) {
		case LOCAL_DIRECTION_0: sideNeighbors[0] = superPosition; return;
		case LOCAL_DIRECTION_1: sideNeighbors[1] = superPosition; return;
		case LOCAL_DIRECTION_2: sideNeighbors[2] = superPosition; return;
		case LOCAL_DIRECTION_3: sideNeighbors[3] = superPosition; return;
		}
	}
	void setNeighbor(LocalDirection dir, CornerSuperPosition* superPosition) override 
	{
		switch (dir) {
		case LOCAL_DIRECTION_0_1: cornerNeighbors[0] = superPosition; return;
		case LOCAL_DIRECTION_1_2: cornerNeighbors[1] = superPosition; return;
		case LOCAL_DIRECTION_2_3: cornerNeighbors[2] = superPosition; return;
		case LOCAL_DIRECTION_3_0: cornerNeighbors[3] = superPosition; return;
		}
	}
	void setNeighborToNull(LocalDirection dir) override
	{
		switch (dir) {
		case LOCAL_DIRECTION_0: sideNeighbors[0] = nullptr; return;
		case LOCAL_DIRECTION_1: sideNeighbors[1] = nullptr; return;
		case LOCAL_DIRECTION_2: sideNeighbors[2] = nullptr; return;
		case LOCAL_DIRECTION_3: sideNeighbors[3] = nullptr; return;
		case LOCAL_DIRECTION_0_1: cornerNeighbors[0] = nullptr; return;
		case LOCAL_DIRECTION_1_2: cornerNeighbors[1] = nullptr; return;
		case LOCAL_DIRECTION_2_3: cornerNeighbors[2] = nullptr; return;
		case LOCAL_DIRECTION_3_0: cornerNeighbors[3] = nullptr; return;
		}
	}

	int getNeighborAlignmentMapIndex(LocalDirection dir) override
	{
		switch (dir) {
		case LOCAL_DIRECTION_1: return neighborAlignmentMaps[0];
		case LOCAL_DIRECTION_2: return neighborAlignmentMaps[1];
		case LOCAL_DIRECTION_3: return neighborAlignmentMaps[2];
		case LOCAL_DIRECTION_0: return neighborAlignmentMaps[3];
		case LOCAL_DIRECTION_0_1: return neighborAlignmentMaps[4];
		case LOCAL_DIRECTION_1_2: return neighborAlignmentMaps[5];
		case LOCAL_DIRECTION_2_3: return neighborAlignmentMaps[6];
		case LOCAL_DIRECTION_3_0: return neighborAlignmentMaps[7];
		default: return -1;
		}
	}
	void setNeighborAlignmentMapIndex(LocalDirection dir, int mapIndex) override
	{
		switch (dir) {
		case LOCAL_DIRECTION_1: neighborAlignmentMaps[0] = mapIndex;  return;
		case LOCAL_DIRECTION_2: neighborAlignmentMaps[1] = mapIndex;  return;
		case LOCAL_DIRECTION_3: neighborAlignmentMaps[2] = mapIndex;  return;
		case LOCAL_DIRECTION_0: neighborAlignmentMaps[3] = mapIndex;  return;
		case LOCAL_DIRECTION_0_1: neighborAlignmentMaps[4] = mapIndex;  return;
		case LOCAL_DIRECTION_1_2: neighborAlignmentMaps[5] = mapIndex;  return;
		case LOCAL_DIRECTION_2_3: neighborAlignmentMaps[6] = mapIndex;  return;
		case LOCAL_DIRECTION_3_0: neighborAlignmentMaps[7] = mapIndex;  return;
		}
	}

	float getDirectionMagnitude(LocalDirection dir) override
	{
		switch (dir) {
		case LOCAL_DIRECTION_1: return directionMagnitudes[0];
		case LOCAL_DIRECTION_2: return directionMagnitudes[1];
		case LOCAL_DIRECTION_3: return directionMagnitudes[2];
		case LOCAL_DIRECTION_0: return directionMagnitudes[3];
		default: return -1;
		}
	}
	void setDirectionMagnitude(LocalDirection dir, float mag) override 
	{
		switch (dir) {
		case LOCAL_DIRECTION_1: directionMagnitudes[0] = mag;  return;
		case LOCAL_DIRECTION_2: directionMagnitudes[1] = mag;  return;
		case LOCAL_DIRECTION_3: directionMagnitudes[2] = mag;  return;
		case LOCAL_DIRECTION_0: directionMagnitudes[3] = mag;  return;
		}
	}

};

enum MetaSideSuperPositionOrientation { 
	SIDE_SUPER_POSITION_ORIENTATION_0_2, 
	SIDE_SUPER_POSITION_ORIENTATION_1_3, 
	SIDE_SUPER_POSITION_ORIENTATION_ERROR 
};

// Tiles point to 4 of these.
// Min of 1, max of 4 overlapping side super positions the same 3D position.
struct SideSuperPosition : public SuperPosition {
	MetaSideSuperPositionOrientation orientation;
	
private:
	float directionMagnitudes[4];
	CenterSuperPosition* centerNeighbors[2];
	SideSuperPosition* sideNeighbors[4];
	CornerSuperPosition* cornerNeighbors[2];
	int neighborAlignmentMaps[8];

	int tileIndices[2];
	LocalPosition tilePositions[2];
	int tileAlignmentMaps[2];

public:
	SideSuperPosition(SuperPositionTileInfo tileInfo0, SuperPositionTileInfo tileInfo1)
	{
		SuperPositionTileInfo* tileInfos[4] = { &tileInfo0, &tileInfo1 };

		index = -1;
		type = SUPER_POSITION_TYPE_SIDE;
		orientation = SIDE_SUPER_POSITION_ORIENTATION_ERROR;
		for (int i = 0; i < 4; i++) { directionMagnitudes[i] = 0; }
		for (int i = 0; i < 2; i++) { 
			centerNeighbors[i] = nullptr;
			neighborAlignmentMaps[i] = -1; 

			tileIndices[i] = tileInfos[i]->tileIndex;
			tilePositions[i] = tileInfos[i]->position;
			tileAlignmentMaps[i] = tileInfos[i]->alignmentMapIndex;
		}
	}

	SuperPosition* getNeighbor(LocalDirection dir) override
	{
		switch (orientation) {
		case SIDE_SUPER_POSITION_ORIENTATION_0_2:
			switch (dir) {
			case LOCAL_DIRECTION_0: return centerNeighbors[0];
			case LOCAL_DIRECTION_1: return cornerNeighbors[0];
			case LOCAL_DIRECTION_2: return centerNeighbors[1];
			case LOCAL_DIRECTION_3: return cornerNeighbors[1];
			case LOCAL_DIRECTION_0_1: return sideNeighbors[0];
			case LOCAL_DIRECTION_1_2: return sideNeighbors[1];
			case LOCAL_DIRECTION_2_3: return sideNeighbors[2];
			case LOCAL_DIRECTION_3_0: return sideNeighbors[3];
			default: return nullptr;
			}
		case SIDE_SUPER_POSITION_ORIENTATION_1_3:
			switch (dir) {
			case LOCAL_DIRECTION_0: return cornerNeighbors[0];
			case LOCAL_DIRECTION_1: return centerNeighbors[0];
			case LOCAL_DIRECTION_2: return cornerNeighbors[1];
			case LOCAL_DIRECTION_3: return centerNeighbors[1];
			case LOCAL_DIRECTION_0_1: return sideNeighbors[0];
			case LOCAL_DIRECTION_1_2: return sideNeighbors[1];
			case LOCAL_DIRECTION_2_3: return sideNeighbors[2];
			case LOCAL_DIRECTION_3_0: return sideNeighbors[3];
			default: return nullptr;
			}
		}
	}
	void setNeighbor(LocalDirection dir, CenterSuperPosition* superPosition) override
	{
		switch (orientation) {
		case SIDE_SUPER_POSITION_ORIENTATION_0_2:
			switch (dir) {
			case LOCAL_DIRECTION_0: centerNeighbors[0] = superPosition; return;
			case LOCAL_DIRECTION_2: centerNeighbors[1] = superPosition; return;
			}
		case SIDE_SUPER_POSITION_ORIENTATION_1_3:
			switch (dir) {
			case LOCAL_DIRECTION_1: centerNeighbors[0] = superPosition; return;
			case LOCAL_DIRECTION_3: centerNeighbors[1] = superPosition; return;
			}
		}
	}
	void setNeighbor(LocalDirection dir, SideSuperPosition* superPosition) override
	{
		switch (dir) {
		case LOCAL_DIRECTION_0_1: sideNeighbors[0] = superPosition; return;
		case LOCAL_DIRECTION_1_2: sideNeighbors[1] = superPosition; return;
		case LOCAL_DIRECTION_2_3: sideNeighbors[2] = superPosition; return;
		case LOCAL_DIRECTION_3_0: sideNeighbors[3] = superPosition; return;
		}
	}
	void setNeighbor(LocalDirection dir, CornerSuperPosition* superPosition) override
	{
		switch (orientation) {
		case SIDE_SUPER_POSITION_ORIENTATION_0_2:
			switch (dir) {
			case LOCAL_DIRECTION_1: cornerNeighbors[0] = superPosition; return;
			case LOCAL_DIRECTION_3: cornerNeighbors[1] = superPosition; return;
			}
		case SIDE_SUPER_POSITION_ORIENTATION_1_3:
			switch (dir) {
			case LOCAL_DIRECTION_0: cornerNeighbors[0] = superPosition; return;
			case LOCAL_DIRECTION_2: cornerNeighbors[1] = superPosition; return;
			}
		}
	}
	void setNeighborToNull(LocalDirection dir) override
	{
		switch (orientation) {
		case SIDE_SUPER_POSITION_ORIENTATION_0_2:
			switch (dir) {
			case LOCAL_DIRECTION_0: centerNeighbors[0] = nullptr; return;
			case LOCAL_DIRECTION_1: cornerNeighbors[0] = nullptr; return;
			case LOCAL_DIRECTION_2: centerNeighbors[1] = nullptr; return;
			case LOCAL_DIRECTION_3: cornerNeighbors[1] = nullptr; return;
			case LOCAL_DIRECTION_0_1: sideNeighbors[0] = nullptr; return;
			case LOCAL_DIRECTION_1_2: sideNeighbors[1] = nullptr; return;
			case LOCAL_DIRECTION_2_3: sideNeighbors[2] = nullptr; return;
			case LOCAL_DIRECTION_3_0: sideNeighbors[3] = nullptr; return;
			}
		case SIDE_SUPER_POSITION_ORIENTATION_1_3:
			switch (dir) {
			case LOCAL_DIRECTION_0: cornerNeighbors[0] = nullptr; return;
			case LOCAL_DIRECTION_1: centerNeighbors[0] = nullptr; return;
			case LOCAL_DIRECTION_2: cornerNeighbors[1] = nullptr; return;
			case LOCAL_DIRECTION_3: centerNeighbors[1] = nullptr; return;
			case LOCAL_DIRECTION_0_1: sideNeighbors[0] = nullptr; return;
			case LOCAL_DIRECTION_1_2: sideNeighbors[1] = nullptr; return;
			case LOCAL_DIRECTION_2_3: sideNeighbors[2] = nullptr; return;
			case LOCAL_DIRECTION_3_0: sideNeighbors[3] = nullptr; return;
			}
		}
	}
	
	int getNeighborAlignmentMapIndex(LocalDirection dir) override
	{
		switch (dir) {
		case LOCAL_DIRECTION_0: return neighborAlignmentMaps[0];
		case LOCAL_DIRECTION_1: return neighborAlignmentMaps[1];
		case LOCAL_DIRECTION_2: return neighborAlignmentMaps[2];
		case LOCAL_DIRECTION_3: return neighborAlignmentMaps[3];
		case LOCAL_DIRECTION_0_1: return neighborAlignmentMaps[4];
		case LOCAL_DIRECTION_1_2: return neighborAlignmentMaps[5];
		case LOCAL_DIRECTION_2_3: return neighborAlignmentMaps[6];
		case LOCAL_DIRECTION_3_0: return neighborAlignmentMaps[7];
		default: return -1;
		}
	}
	void setNeighborAlignmentMapIndex(LocalDirection dir, int mapIndex) override
	{
		switch (dir) {
		case LOCAL_DIRECTION_0: neighborAlignmentMaps[0] = mapIndex; return;
		case LOCAL_DIRECTION_1: neighborAlignmentMaps[1] = mapIndex; return;
		case LOCAL_DIRECTION_2: neighborAlignmentMaps[2] = mapIndex; return;
		case LOCAL_DIRECTION_3: neighborAlignmentMaps[3] = mapIndex; return;
		case LOCAL_DIRECTION_0_1: neighborAlignmentMaps[4] = mapIndex; return;
		case LOCAL_DIRECTION_1_2: neighborAlignmentMaps[5] = mapIndex; return;
		case LOCAL_DIRECTION_2_3: neighborAlignmentMaps[6] = mapIndex; return;
		case LOCAL_DIRECTION_3_0: neighborAlignmentMaps[7] = mapIndex; return;
		}
	}
	
	int getTileIndex(LocalDirection dir) override
	{
		switch (orientation) {
		case SIDE_SUPER_POSITION_ORIENTATION_0_2:
			switch (dir) {
			case LOCAL_DIRECTION_0: return tileIndices[0];
			case LOCAL_DIRECTION_2: return tileIndices[1];
			default: return -1;
			}
		case SIDE_SUPER_POSITION_ORIENTATION_1_3:
			switch (dir) {
			case LOCAL_DIRECTION_1: return tileIndices[0];
			case LOCAL_DIRECTION_3: return tileIndices[1];
			default: return -1;
			}
		default: return -1;
		}
	}
	void setTileIndex(LocalDirection dir, int tileIndex) override
	{
		switch (orientation) {
		case SIDE_SUPER_POSITION_ORIENTATION_0_2:
			switch (dir) {
			case LOCAL_DIRECTION_0: tileIndices[0]=tileIndex; return;
			case LOCAL_DIRECTION_2: tileIndices[1] = tileIndex; return;
			}
		case SIDE_SUPER_POSITION_ORIENTATION_1_3:
			switch (dir) {
			case LOCAL_DIRECTION_1: tileIndices[0]=tileIndex; return;
			case LOCAL_DIRECTION_3: tileIndices[1] = tileIndex; return;
			}
		}
	}

	LocalPosition getTilePosition(LocalDirection dir) override
	{
		switch (orientation) {
		case SIDE_SUPER_POSITION_ORIENTATION_0_2:
			switch (dir) {
			case LOCAL_DIRECTION_0: return tilePositions[0];
			case LOCAL_DIRECTION_2: return tilePositions[1];
			default: return LOCAL_POSITION_ERROR;
			}
		case SIDE_SUPER_POSITION_ORIENTATION_1_3:
			switch (dir) {
			case LOCAL_DIRECTION_1: return tilePositions[0];
			case LOCAL_DIRECTION_3: return tilePositions[1];
			default: return LOCAL_POSITION_ERROR;
			}
		default: return LOCAL_POSITION_ERROR;
		}
	}
	void setTilePosition(LocalDirection dir, LocalPosition pos) override
	{
		switch (orientation) {
		case SIDE_SUPER_POSITION_ORIENTATION_0_2:
			switch (dir) {
			case LOCAL_DIRECTION_0: tilePositions[0]=pos; return;
			case LOCAL_DIRECTION_2: tilePositions[1]=pos; return;
			}
		case SIDE_SUPER_POSITION_ORIENTATION_1_3:
			switch (dir) {
			case LOCAL_DIRECTION_1: tilePositions[0]=pos; return;
			case LOCAL_DIRECTION_3: tilePositions[1]=pos; return;
			}
		}
	}

	int getTileAlignmentMap(LocalDirection dir) override
	{
		switch (orientation) {
		case SIDE_SUPER_POSITION_ORIENTATION_0_2:
			switch (dir) {
			case LOCAL_DIRECTION_0: return tileAlignmentMaps[0];
			case LOCAL_DIRECTION_2: return tileAlignmentMaps[1];
			default: return -1;
			}
		case SIDE_SUPER_POSITION_ORIENTATION_1_3:
			switch (dir) {
			case LOCAL_DIRECTION_1: return tileAlignmentMaps[0];
			case LOCAL_DIRECTION_3: return tileAlignmentMaps[1];
			default: return -1;
			}
		default: return -1;
		}
	}
	void setTileAlignmentMap(LocalDirection dir, int mapIndex) override
	{
		switch (orientation) {
		case SIDE_SUPER_POSITION_ORIENTATION_0_2:
			switch (dir) {
			case LOCAL_DIRECTION_0: tileAlignmentMaps[0]=mapIndex; return;
			case LOCAL_DIRECTION_2: tileAlignmentMaps[1]=mapIndex; return;
			}
		case SIDE_SUPER_POSITION_ORIENTATION_1_3:
			switch (dir) {
			case LOCAL_DIRECTION_1: tileAlignmentMaps[0]=mapIndex; return;
			case LOCAL_DIRECTION_3: tileAlignmentMaps[1]=mapIndex; return;
			}
		}
	}

	float getDirectionMagnitude(LocalDirection dir) override
	{
		switch (dir) {
		case LOCAL_DIRECTION_1: return directionMagnitudes[0];
		case LOCAL_DIRECTION_2: return directionMagnitudes[1];
		case LOCAL_DIRECTION_3: return directionMagnitudes[2];
		case LOCAL_DIRECTION_0: return directionMagnitudes[3];
		default: return -1;
		}
	}
	void setDirectionMagnitude(LocalDirection dir, float mag) override
	{
		switch (dir) {
		case LOCAL_DIRECTION_1: directionMagnitudes[0] = mag;  return;
		case LOCAL_DIRECTION_2: directionMagnitudes[1] = mag;  return;
		case LOCAL_DIRECTION_3: directionMagnitudes[2] = mag;  return;
		case LOCAL_DIRECTION_0: directionMagnitudes[3] = mag;  return;
		}
	}
};

// Tiles point to 4 of these.
// Min of 1, max of 2 overlapping corner super positions in same 3D position.  
// More than 2 means unsafe corner and we dont worry about those.
struct CornerSuperPosition : public SuperPosition {
private:
	float directionMagnitudes[4];

	CenterSuperPosition* centerNeighbors[4];
	SideSuperPosition* sideNeighbors[4];
	int neighborAlignmentMaps[8];

	int tileIndices[4];
	LocalPosition tilePositions[4];
	int tileAlignmentMaps[4];

public:
	CornerSuperPosition(SuperPositionTileInfo tileInfo0, SuperPositionTileInfo tileInfo1,
				   SuperPositionTileInfo tileInfo2, SuperPositionTileInfo tileInfo3)
	{
		SuperPositionTileInfo* tileInfos[4] = { &tileInfo0, &tileInfo1, &tileInfo2, &tileInfo3 };

		type = SUPER_POSITION_TYPE_CORNER;
		for (int i = 0; i < 8; i++) { neighborAlignmentMaps[i] = -1; }
		for (int i = 0; i < 4; i++) { 
			directionMagnitudes[i] = 0; 
			centerNeighbors[i] = nullptr;

			tileIndices[i] = tileInfos[i]->tileIndex;
			tilePositions[i] = tileInfos[i]->position;
			tileAlignmentMaps[i] = tileInfos[i]->alignmentMapIndex;
		}
	}

	SuperPosition* getNeighbor(LocalDirection dir) override
	{
		switch (dir) {
		case LOCAL_DIRECTION_0: return sideNeighbors[0];
		case LOCAL_DIRECTION_1: return sideNeighbors[0];
		case LOCAL_DIRECTION_2: return sideNeighbors[1];
		case LOCAL_DIRECTION_3: return sideNeighbors[1];
		case LOCAL_DIRECTION_0_1: return centerNeighbors[0];
		case LOCAL_DIRECTION_1_2: return centerNeighbors[1];
		case LOCAL_DIRECTION_2_3: return centerNeighbors[2];
		case LOCAL_DIRECTION_3_0: return centerNeighbors[3];
		default: return nullptr;
		}
	}
	void setNeighbor(LocalDirection dir, CenterSuperPosition* superPosition) override
	{
		switch (dir) {
		case LOCAL_DIRECTION_0_1: centerNeighbors[0] = superPosition;
		case LOCAL_DIRECTION_1_2: centerNeighbors[1] = superPosition;
		case LOCAL_DIRECTION_2_3: centerNeighbors[2] = superPosition;
		case LOCAL_DIRECTION_3_0: centerNeighbors[3] = superPosition;
		}
	}
	void setNeighbor(LocalDirection dir, SideSuperPosition* superPosition) override
	{
		switch (dir) {
		case LOCAL_DIRECTION_0: sideNeighbors[0] = superPosition;
		case LOCAL_DIRECTION_1: sideNeighbors[0] = superPosition;
		case LOCAL_DIRECTION_2: sideNeighbors[1] = superPosition;
		case LOCAL_DIRECTION_3: sideNeighbors[1] = superPosition;
		}
	}
	void setNeighborToNull(LocalDirection dir) override
	{
		switch (dir) {
		case LOCAL_DIRECTION_0: sideNeighbors[0] = nullptr;
		case LOCAL_DIRECTION_1: sideNeighbors[0] = nullptr;
		case LOCAL_DIRECTION_2: sideNeighbors[1] = nullptr;
		case LOCAL_DIRECTION_3: sideNeighbors[1] = nullptr;
		case LOCAL_DIRECTION_0_1: centerNeighbors[0] = nullptr;
		case LOCAL_DIRECTION_1_2: centerNeighbors[1] = nullptr;
		case LOCAL_DIRECTION_2_3: centerNeighbors[2] = nullptr;
		case LOCAL_DIRECTION_3_0: centerNeighbors[3] = nullptr;
		}
	}


	int getNeighborAlignmentMapIndex(LocalDirection dir) override
	{
		switch (dir) {
		case LOCAL_DIRECTION_0: return neighborAlignmentMaps[0];
		case LOCAL_DIRECTION_1: return neighborAlignmentMaps[1];
		case LOCAL_DIRECTION_2: return neighborAlignmentMaps[2];
		case LOCAL_DIRECTION_3: return neighborAlignmentMaps[3];
		case LOCAL_DIRECTION_0_1: return neighborAlignmentMaps[4];
		case LOCAL_DIRECTION_1_2: return neighborAlignmentMaps[5];
		case LOCAL_DIRECTION_2_3: return neighborAlignmentMaps[6];
		case LOCAL_DIRECTION_3_0: return neighborAlignmentMaps[7];
		default: return -1;
		}
	}
	void setNeighborAlignmentMapIndex(LocalDirection dir, int mapIndex) override
	{
		switch (dir) {
		case LOCAL_DIRECTION_0: neighborAlignmentMaps[0] = mapIndex; return;
		case LOCAL_DIRECTION_1: neighborAlignmentMaps[1] = mapIndex; return;
		case LOCAL_DIRECTION_2: neighborAlignmentMaps[2] = mapIndex; return;
		case LOCAL_DIRECTION_3: neighborAlignmentMaps[3] = mapIndex; return;
		case LOCAL_DIRECTION_0_1: neighborAlignmentMaps[4] = mapIndex; return;
		case LOCAL_DIRECTION_1_2: neighborAlignmentMaps[5] = mapIndex; return;
		case LOCAL_DIRECTION_2_3: neighborAlignmentMaps[6] = mapIndex; return;
		case LOCAL_DIRECTION_3_0: neighborAlignmentMaps[7] = mapIndex; return;
		}
	}

	int getTileIndex(LocalDirection dir) override
	{
		switch (dir) {
		case LOCAL_DIRECTION_0: return tileIndices[0];
		case LOCAL_DIRECTION_1: return tileIndices[1];
		case LOCAL_DIRECTION_2: return tileIndices[2];
		case LOCAL_DIRECTION_3: return tileIndices[3];
		default: return -1;
		}
	}
	void setTileIndex(LocalDirection dir, int tileIndex) override
	{
		switch (dir) {
		case LOCAL_DIRECTION_0: tileIndices[0] = tileIndex; return;
		case LOCAL_DIRECTION_1: tileIndices[1] = tileIndex; return;
		case LOCAL_DIRECTION_2: tileIndices[2] = tileIndex; return;
		case LOCAL_DIRECTION_3: tileIndices[3] = tileIndex; return;
		}
	}

	LocalPosition getTilePosition(LocalDirection dir) override
	{
		switch (dir) {
		case LOCAL_DIRECTION_0: return tilePositions[0];
		case LOCAL_DIRECTION_1: return tilePositions[1];
		case LOCAL_DIRECTION_2: return tilePositions[2];
		case LOCAL_DIRECTION_3: return tilePositions[3];
		}
	}
	void setTilePosition(LocalDirection dir, LocalPosition pos) override
	{
		switch (dir) {
		case LOCAL_DIRECTION_0: tilePositions[0] = pos; return;
		case LOCAL_DIRECTION_1: tilePositions[1] = pos; return;
		case LOCAL_DIRECTION_2: tilePositions[2] = pos; return;
		case LOCAL_DIRECTION_3: tilePositions[3] = pos; return;
		}
	}

	int getTileAlignmentMap(LocalDirection dir) override
	{
		switch (dir) {
		case LOCAL_DIRECTION_0: return tileAlignmentMaps[0];
		case LOCAL_DIRECTION_1: return tileAlignmentMaps[1];
		case LOCAL_DIRECTION_2: return tileAlignmentMaps[2];
		case LOCAL_DIRECTION_3: return tileAlignmentMaps[3];
		}
	}
	void setTileAlignmentMap(LocalDirection dir, int mapIndex) override
	{
		switch (dir) {
		case LOCAL_DIRECTION_0: tileAlignmentMaps[0] = mapIndex; return;
		case LOCAL_DIRECTION_1: tileAlignmentMaps[1] = mapIndex; return;
		case LOCAL_DIRECTION_2: tileAlignmentMaps[2] = mapIndex; return;
		case LOCAL_DIRECTION_3: tileAlignmentMaps[3] = mapIndex; return;
		}
	}

	float getDirectionMagnitude(LocalDirection dir) override
	{
		switch (dir) {
		case LOCAL_DIRECTION_1: return directionMagnitudes[0];
		case LOCAL_DIRECTION_2: return directionMagnitudes[1];
		case LOCAL_DIRECTION_3: return directionMagnitudes[2];
		case LOCAL_DIRECTION_0: return directionMagnitudes[3];
		default: return -1;
		}
	}
	void setDirectionMagnitude(LocalDirection dir, float mag) override
	{
		switch (dir) {
		case LOCAL_DIRECTION_1: directionMagnitudes[0] = mag;  return;
		case LOCAL_DIRECTION_2: directionMagnitudes[1] = mag;  return;
		case LOCAL_DIRECTION_3: directionMagnitudes[2] = mag;  return;
		case LOCAL_DIRECTION_0: directionMagnitudes[3] = mag;  return;
		}
	}
};

// This manager is responsible for collision solving.  
// Once solved, it will be referenced to update the direciton values for each moving entity.
struct SuperPositionNetwork 
{
	std::vector<SuperPosition> superPositions;
	// When a super position is deleted, its index is added here so a new super position can replace it in the array:
	std::vector<int> freeSuperPositionIndices;
	// List of all the super positions that need to be checked for force distribution steps next simulation step:
	std::vector<int> potentialSuperPositionCollisionIndices;

	SuperPosition* add(SuperPosition superPosition)
	{
		if (freeSuperPositionIndices.size() == 0) {
			superPositions.push_back(superPosition);
			return &superPositions.back();
		}
		else {
			superPositions[freeSuperPositionIndices.back()] = superPosition;
			SuperPosition* p = &superPositions[freeSuperPositionIndices.back()];
			freeSuperPositionIndices.pop_back();
			return p;
		}
	}

	void remove(SuperPosition* pos, std::vector<SuperPosition*>&affectedSuperPositions)
	{
		freeSuperPositionIndices.push_back(pos->index);

		for (LocalDirection d : tnav::NON_STATIC_LOCAL_DIRECTION_LIST) {
			SuperPosition* neighbor = pos->getNeighbor(d);
			if (neighbor == nullptr) {
				continue;
			}

			affectedSuperPositions.push_back(neighbor);

			// Not strictly necessary but will catch errors if neighbors are not propoerly reconnected later:
			LocalDirection returnDirection = tnav::oppositeAlignment(d);
			returnDirection = tnav::getMappedAlignment(pos->getNeighborAlignmentMapIndex(d), returnDirection);
			neighbor->setNeighborToNull(returnDirection);
		}
	}

	void solveCollisions()
	{

	}
};