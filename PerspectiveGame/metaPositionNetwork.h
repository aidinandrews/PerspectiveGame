#pragma once

#include <iostream>
#include <vector>

#include "tileNavigation.h"

struct MetaNodeTileInfo {
	int tileIndex;
	LocalPosition position;
	int alignmentMapIndex;
};

enum MetaNodeType {
	META_NODE_TYPE_CENTER,
	META_NODE_TYPE_SIDE,
	META_NODE_TYPE_CORNER,
	META_NODE_TYPE_ERROR,
};

struct MetaNode {
public:
	MetaNodeType type;
	int index;

public:
	MetaNode() : type(META_NODE_TYPE_ERROR), index(-1) {}

	virtual MetaNode* getNodeNeighbor(LocalDirection dir) {}
	virtual void setNodeNeighbor(LocalDirection dir, MetaCenterNode* node) {}
	virtual void setNodeNeighbor(LocalDirection dir, MetaSideNode* node) {}
	virtual void setNodeNeighbor(LocalDirection dir, MetaCornerNode* node) {}
	virtual void setNodeNeighborToNull(LocalDirection dir) {}

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

struct MetaSideNode;
struct MetaCornerNode;

// ON CREATION OF A TILE PAIR:
// make a list of all the meta nodes they touch (A),
// make a list of all the meta node neighbors of A (B),
// delete all nodes in A,
// create a new set of nodes that touch the pair and add them to B.
// * if a node would be unsafe, skip it's creation.
// update/reconnect all nodes in B.

// ON DELETION OF A TILE PAIR:
// make a list of all the meta nodes (A),
// make a list of all the meta node neighbors of A (B),
// make a list of (tile index + local SIDE position) (C) that:
//    point to nodes in the same 3D position 
//    connect to tiles other than the pair
// delete all nodes of A,
// for each pair of positions in C, create a new node and add it to B,
// update/reconnect all nodes in B.
//

// Tiles point to 1 of these
// Exactly 2 overlapping center nodes in the same 3D position due to sibling tiles.
struct MetaCenterNode : public MetaNode {
	int tileIndex;
	// Position in tile is always LOCAL_POSITION_CENTER.
	// node->tile map is always identity.
	
private:
	float directionMagnitudes[4];
	MetaSideNode* sideNodeNeighbors[4];
	MetaCornerNode* cornerNodeNeighbors[4];
	int neighborAlignmentMaps[8];

public:
	MetaCenterNode(MetaNodeType type, int tileIndex)
	{
		type = META_NODE_TYPE_CENTER;

		this->tileIndex = tileIndex;
		for (int i = 0; i < 4; i++) { 
			directionMagnitudes[i] = 0; 
			sideNodeNeighbors[i] = nullptr;
			cornerNodeNeighbors[i] = nullptr;
		}
		for (int i = 0; i < 8; i++) {
			neighborAlignmentMaps[i] = -1;
		}
	}

	MetaNode* getNodeNeighbor(LocalDirection dir) override 
	{
		switch (dir) {
		case LOCAL_DIRECTION_1: return sideNodeNeighbors[0];
		case LOCAL_DIRECTION_2: return sideNodeNeighbors[1];
		case LOCAL_DIRECTION_3: return sideNodeNeighbors[2];
		case LOCAL_DIRECTION_0: return sideNodeNeighbors[3];
		case LOCAL_DIRECTION_0_1: return cornerNodeNeighbors[0];
		case LOCAL_DIRECTION_1_2: return cornerNodeNeighbors[1];
		case LOCAL_DIRECTION_2_3: return cornerNodeNeighbors[2];
		case LOCAL_DIRECTION_3_0: return cornerNodeNeighbors[3];
		default: return nullptr;
		}
	}
	void setNodeNeighbor(LocalDirection dir, MetaSideNode* node) override 
	{
		switch (dir) {
		case LOCAL_DIRECTION_0: sideNodeNeighbors[0] = node; return;
		case LOCAL_DIRECTION_1: sideNodeNeighbors[1] = node; return;
		case LOCAL_DIRECTION_2: sideNodeNeighbors[2] = node; return;
		case LOCAL_DIRECTION_3: sideNodeNeighbors[3] = node; return;
		}
	}
	void setNodeNeighbor(LocalDirection dir, MetaCornerNode* node) override 
	{
		switch (dir) {
		case LOCAL_DIRECTION_0_1: cornerNodeNeighbors[0] = node; return;
		case LOCAL_DIRECTION_1_2: cornerNodeNeighbors[1] = node; return;
		case LOCAL_DIRECTION_2_3: cornerNodeNeighbors[2] = node; return;
		case LOCAL_DIRECTION_3_0: cornerNodeNeighbors[3] = node; return;
		}
	}
	void setNodeNeighborToNull(LocalDirection dir) override
	{
		switch (dir) {
		case LOCAL_DIRECTION_0: sideNodeNeighbors[0] = nullptr; return;
		case LOCAL_DIRECTION_1: sideNodeNeighbors[1] = nullptr; return;
		case LOCAL_DIRECTION_2: sideNodeNeighbors[2] = nullptr; return;
		case LOCAL_DIRECTION_3: sideNodeNeighbors[3] = nullptr; return;
		case LOCAL_DIRECTION_0_1: cornerNodeNeighbors[0] = nullptr; return;
		case LOCAL_DIRECTION_1_2: cornerNodeNeighbors[1] = nullptr; return;
		case LOCAL_DIRECTION_2_3: cornerNodeNeighbors[2] = nullptr; return;
		case LOCAL_DIRECTION_3_0: cornerNodeNeighbors[3] = nullptr; return;
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

enum MetaSideNodeOrientation { 
	META_SIDE_NODE_ORIENTATION_0_2, 
	META_SIDE_NODE_ORIENTATION_1_3, 
	META_SIDE_NODE_ORIENTATION_ERROR 
};

// Tiles point to 4 of these.
// Min of 1, max of 4 overlapping side nodes the same 3D position.
struct MetaSideNode : public MetaNode {
	MetaSideNodeOrientation orientation;
	
private:
	float directionMagnitudes[4];
	MetaCenterNode* centerNodeNeighbors[2];
	MetaSideNode* sideNodeNeighbors[4];
	MetaCornerNode* cornerNodeNeighbors[2];
	int neighborAlignmentMaps[8];

	int tileIndices[2];
	LocalPosition tilePositions[2];
	int tileAlignmentMaps[2];

public:
	MetaSideNode(MetaNodeTileInfo tileInfo0, MetaNodeTileInfo tileInfo1)
	{
		MetaNodeTileInfo* tileInfos[4] = { &tileInfo0, &tileInfo1 };

		index = -1;
		type = META_NODE_TYPE_SIDE;
		orientation = META_SIDE_NODE_ORIENTATION_ERROR;
		for (int i = 0; i < 4; i++) { directionMagnitudes[i] = 0; }
		for (int i = 0; i < 2; i++) { 
			centerNodeNeighbors[i] = nullptr;
			neighborAlignmentMaps[i] = -1; 

			tileIndices[i] = tileInfos[i]->tileIndex;
			tilePositions[i] = tileInfos[i]->position;
			tileAlignmentMaps[i] = tileInfos[i]->alignmentMapIndex;
		}
	}

	MetaNode* getNodeNeighbor(LocalDirection dir) override
	{
		switch (orientation) {
		case META_SIDE_NODE_ORIENTATION_0_2:
			switch (dir) {
			case LOCAL_DIRECTION_0: return centerNodeNeighbors[0];
			case LOCAL_DIRECTION_1: return cornerNodeNeighbors[0];
			case LOCAL_DIRECTION_2: return centerNodeNeighbors[1];
			case LOCAL_DIRECTION_3: return cornerNodeNeighbors[1];
			case LOCAL_DIRECTION_0_1: return sideNodeNeighbors[0];
			case LOCAL_DIRECTION_1_2: return sideNodeNeighbors[1];
			case LOCAL_DIRECTION_2_3: return sideNodeNeighbors[2];
			case LOCAL_DIRECTION_3_0: return sideNodeNeighbors[3];
			default: return nullptr;
			}
		case META_SIDE_NODE_ORIENTATION_1_3:
			switch (dir) {
			case LOCAL_DIRECTION_0: return cornerNodeNeighbors[0];
			case LOCAL_DIRECTION_1: return centerNodeNeighbors[0];
			case LOCAL_DIRECTION_2: return cornerNodeNeighbors[1];
			case LOCAL_DIRECTION_3: return centerNodeNeighbors[1];
			case LOCAL_DIRECTION_0_1: return sideNodeNeighbors[0];
			case LOCAL_DIRECTION_1_2: return sideNodeNeighbors[1];
			case LOCAL_DIRECTION_2_3: return sideNodeNeighbors[2];
			case LOCAL_DIRECTION_3_0: return sideNodeNeighbors[3];
			default: return nullptr;
			}
		}
	}
	void setNodeNeighbor(LocalDirection dir, MetaCenterNode* node) override
	{
		switch (orientation) {
		case META_SIDE_NODE_ORIENTATION_0_2:
			switch (dir) {
			case LOCAL_DIRECTION_0: centerNodeNeighbors[0] = node; return;
			case LOCAL_DIRECTION_2: centerNodeNeighbors[1] = node; return;
			}
		case META_SIDE_NODE_ORIENTATION_1_3:
			switch (dir) {
			case LOCAL_DIRECTION_1: centerNodeNeighbors[0] = node; return;
			case LOCAL_DIRECTION_3: centerNodeNeighbors[1] = node; return;
			}
		}
	}
	void setNodeNeighbor(LocalDirection dir, MetaSideNode* node) override
	{
		switch (dir) {
		case LOCAL_DIRECTION_0_1: sideNodeNeighbors[0] = node; return;
		case LOCAL_DIRECTION_1_2: sideNodeNeighbors[1] = node; return;
		case LOCAL_DIRECTION_2_3: sideNodeNeighbors[2] = node; return;
		case LOCAL_DIRECTION_3_0: sideNodeNeighbors[3] = node; return;
		}
	}
	void setNodeNeighbor(LocalDirection dir, MetaCornerNode* node) override
	{
		switch (orientation) {
		case META_SIDE_NODE_ORIENTATION_0_2:
			switch (dir) {
			case LOCAL_DIRECTION_1: cornerNodeNeighbors[0] = node; return;
			case LOCAL_DIRECTION_3: cornerNodeNeighbors[1] = node; return;
			}
		case META_SIDE_NODE_ORIENTATION_1_3:
			switch (dir) {
			case LOCAL_DIRECTION_0: cornerNodeNeighbors[0] = node; return;
			case LOCAL_DIRECTION_2: cornerNodeNeighbors[1] = node; return;
			}
		}
	}
	void setNodeNeighborToNull(LocalDirection dir) override
	{
		switch (orientation) {
		case META_SIDE_NODE_ORIENTATION_0_2:
			switch (dir) {
			case LOCAL_DIRECTION_0: centerNodeNeighbors[0] = nullptr; return;
			case LOCAL_DIRECTION_1: cornerNodeNeighbors[0] = nullptr; return;
			case LOCAL_DIRECTION_2: centerNodeNeighbors[1] = nullptr; return;
			case LOCAL_DIRECTION_3: cornerNodeNeighbors[1] = nullptr; return;
			case LOCAL_DIRECTION_0_1: sideNodeNeighbors[0] = nullptr; return;
			case LOCAL_DIRECTION_1_2: sideNodeNeighbors[1] = nullptr; return;
			case LOCAL_DIRECTION_2_3: sideNodeNeighbors[2] = nullptr; return;
			case LOCAL_DIRECTION_3_0: sideNodeNeighbors[3] = nullptr; return;
			}
		case META_SIDE_NODE_ORIENTATION_1_3:
			switch (dir) {
			case LOCAL_DIRECTION_0: cornerNodeNeighbors[0] = nullptr; return;
			case LOCAL_DIRECTION_1: centerNodeNeighbors[0] = nullptr; return;
			case LOCAL_DIRECTION_2: cornerNodeNeighbors[1] = nullptr; return;
			case LOCAL_DIRECTION_3: centerNodeNeighbors[1] = nullptr; return;
			case LOCAL_DIRECTION_0_1: sideNodeNeighbors[0] = nullptr; return;
			case LOCAL_DIRECTION_1_2: sideNodeNeighbors[1] = nullptr; return;
			case LOCAL_DIRECTION_2_3: sideNodeNeighbors[2] = nullptr; return;
			case LOCAL_DIRECTION_3_0: sideNodeNeighbors[3] = nullptr; return;
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
		case META_SIDE_NODE_ORIENTATION_0_2:
			switch (dir) {
			case LOCAL_DIRECTION_0: return tileIndices[0];
			case LOCAL_DIRECTION_2: return tileIndices[1];
			default: return -1;
			}
		case META_SIDE_NODE_ORIENTATION_1_3:
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
		case META_SIDE_NODE_ORIENTATION_0_2:
			switch (dir) {
			case LOCAL_DIRECTION_0: tileIndices[0]=tileIndex; return;
			case LOCAL_DIRECTION_2: tileIndices[1] = tileIndex; return;
			}
		case META_SIDE_NODE_ORIENTATION_1_3:
			switch (dir) {
			case LOCAL_DIRECTION_1: tileIndices[0]=tileIndex; return;
			case LOCAL_DIRECTION_3: tileIndices[1] = tileIndex; return;
			}
		}
	}

	LocalPosition getTilePosition(LocalDirection dir) override
	{
		switch (orientation) {
		case META_SIDE_NODE_ORIENTATION_0_2:
			switch (dir) {
			case LOCAL_DIRECTION_0: return tilePositions[0];
			case LOCAL_DIRECTION_2: return tilePositions[1];
			default: return LOCAL_POSITION_ERROR;
			}
		case META_SIDE_NODE_ORIENTATION_1_3:
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
		case META_SIDE_NODE_ORIENTATION_0_2:
			switch (dir) {
			case LOCAL_DIRECTION_0: tilePositions[0]=pos; return;
			case LOCAL_DIRECTION_2: tilePositions[1]=pos; return;
			}
		case META_SIDE_NODE_ORIENTATION_1_3:
			switch (dir) {
			case LOCAL_DIRECTION_1: tilePositions[0]=pos; return;
			case LOCAL_DIRECTION_3: tilePositions[1]=pos; return;
			}
		}
	}

	int getTileAlignmentMap(LocalDirection dir) override
	{
		switch (orientation) {
		case META_SIDE_NODE_ORIENTATION_0_2:
			switch (dir) {
			case LOCAL_DIRECTION_0: return tileAlignmentMaps[0];
			case LOCAL_DIRECTION_2: return tileAlignmentMaps[1];
			default: return -1;
			}
		case META_SIDE_NODE_ORIENTATION_1_3:
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
		case META_SIDE_NODE_ORIENTATION_0_2:
			switch (dir) {
			case LOCAL_DIRECTION_0: tileAlignmentMaps[0]=mapIndex; return;
			case LOCAL_DIRECTION_2: tileAlignmentMaps[1]=mapIndex; return;
			}
		case META_SIDE_NODE_ORIENTATION_1_3:
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
// Min of 1, max of 2 overlapping corner nodes in same 3D position.  
// More than 2 means unsafe corner and we dont worry about those.
struct MetaCornerNode : public MetaNode {
private:
	float directionMagnitudes[4];

	MetaCenterNode* centerNodeNeighbors[4];
	MetaSideNode* sideNodeNeighbors[4];
	int neighborAlignmentMaps[8];

	int tileIndices[4];
	LocalPosition tilePositions[4];
	int tileAlignmentMaps[4];

public:
	MetaCornerNode(MetaNodeTileInfo tileInfo0, MetaNodeTileInfo tileInfo1,
				   MetaNodeTileInfo tileInfo2, MetaNodeTileInfo tileInfo3)
	{
		MetaNodeTileInfo* tileInfos[4] = { &tileInfo0, &tileInfo1, &tileInfo2, &tileInfo3 };

		type = META_NODE_TYPE_CORNER;
		for (int i = 0; i < 8; i++) { neighborAlignmentMaps[i] = -1; }
		for (int i = 0; i < 4; i++) { 
			directionMagnitudes[i] = 0; 
			centerNodeNeighbors[i] = nullptr;

			tileIndices[i] = tileInfos[i]->tileIndex;
			tilePositions[i] = tileInfos[i]->position;
			tileAlignmentMaps[i] = tileInfos[i]->alignmentMapIndex;
		}
	}

	MetaNode* getNodeNeighbor(LocalDirection dir) override
	{
		switch (dir) {
		case LOCAL_DIRECTION_0: return sideNodeNeighbors[0];
		case LOCAL_DIRECTION_1: return sideNodeNeighbors[0];
		case LOCAL_DIRECTION_2: return sideNodeNeighbors[1];
		case LOCAL_DIRECTION_3: return sideNodeNeighbors[1];
		case LOCAL_DIRECTION_0_1: return centerNodeNeighbors[0];
		case LOCAL_DIRECTION_1_2: return centerNodeNeighbors[1];
		case LOCAL_DIRECTION_2_3: return centerNodeNeighbors[2];
		case LOCAL_DIRECTION_3_0: return centerNodeNeighbors[3];
		default: return nullptr;
		}
	}
	void setNodeNeighbor(LocalDirection dir, MetaCenterNode* node) override
	{
		switch (dir) {
		case LOCAL_DIRECTION_0_1: centerNodeNeighbors[0] = node;
		case LOCAL_DIRECTION_1_2: centerNodeNeighbors[1] = node;
		case LOCAL_DIRECTION_2_3: centerNodeNeighbors[2] = node;
		case LOCAL_DIRECTION_3_0: centerNodeNeighbors[3] = node;
		}
	}
	void setNodeNeighbor(LocalDirection dir, MetaSideNode* node) override
	{
		switch (dir) {
		case LOCAL_DIRECTION_0: sideNodeNeighbors[0] = node;
		case LOCAL_DIRECTION_1: sideNodeNeighbors[0] = node;
		case LOCAL_DIRECTION_2: sideNodeNeighbors[1] = node;
		case LOCAL_DIRECTION_3: sideNodeNeighbors[1] = node;
		}
	}
	void setNodeNeighborToNull(LocalDirection dir) override
	{
		switch (dir) {
		case LOCAL_DIRECTION_0: sideNodeNeighbors[0] = nullptr;
		case LOCAL_DIRECTION_1: sideNodeNeighbors[0] = nullptr;
		case LOCAL_DIRECTION_2: sideNodeNeighbors[1] = nullptr;
		case LOCAL_DIRECTION_3: sideNodeNeighbors[1] = nullptr;
		case LOCAL_DIRECTION_0_1: centerNodeNeighbors[0] = nullptr;
		case LOCAL_DIRECTION_1_2: centerNodeNeighbors[1] = nullptr;
		case LOCAL_DIRECTION_2_3: centerNodeNeighbors[2] = nullptr;
		case LOCAL_DIRECTION_3_0: centerNodeNeighbors[3] = nullptr;
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
struct MetaNodeNetwork 
{
	std::vector<MetaNode> nodes;
	// When a node is deleted, its index is added here so a new node can replace it in the array:
	std::vector<int> freeNodeIndices;
	// List of all the nodes that need to be checked for force distribution steps next simulation step:
	std::vector<int> potentialNodeCollisionIndices;

	MetaNode* addNode(MetaNode node)
	{
		if (freeNodeIndices.size() == 0) {
			nodes.push_back(node);
			return &nodes.back();
		}
		else {
			nodes[freeNodeIndices.back()] = node;
			MetaNode* p = &nodes[freeNodeIndices.back()];
			freeNodeIndices.pop_back();
			return p;
		}
	}

	void deleteNode(MetaNode* node, std::vector<MetaNode*>&affectedNodes)
	{
		freeNodeIndices.push_back(node->index);

		for (LocalDirection d : tnav::NON_STATIC_LOCAL_DIRECTION_LIST) {
			MetaNode* neighbor = node->getNodeNeighbor(d);
			if (neighbor == nullptr) {
				continue;
			}

			affectedNodes.push_back(neighbor);

			// Not strictly necessary but will catch errors if neighbors are not propoerly reconnected later:
			LocalDirection neighborToNode = tnav::oppositeAlignment(d);
			neighborToNode = tnav::getMappedAlignment(node->getNeighborAlignmentMapIndex(d), neighborToNode);
			neighbor->setNodeNeighborToNull(neighborToNode);
		}
	}

	void solveCollisions()
	{

	}
};