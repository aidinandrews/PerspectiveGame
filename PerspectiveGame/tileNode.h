#pragma once

#include <iostream>

#include "tileNavigation.h"

enum TileNodeType {
	NODE_TYPE_CENTER,
	NODE_TYPE_SIDE,
	NODE_TYPE_CORNER,
	NODE_TYPE_DEGENERATE,
	NODE_TYPE_ERROR,
};

enum SideTileNodeType {
	SIDE_NODE_TYPE_HORIZONTAL, 
	SIDE_NODE_TYPE_VERTICAL,
	SIDE_NODE_TYPE_ERROR,
};

class TileNode {
private:

public:
	TileNodeType type;
	OrientationType orientation;
	int index;
	int forceListIndex;
	glm::vec3 position;

	TileNode(TileNodeType t) : type(t) {
		index = -1;
		forceListIndex = -1;
		position = glm::vec3(FLT_MAX, FLT_MAX, FLT_MAX);
		orientation = ORIENTATION_TYPE_ERROR;
	}
	virtual ~TileNode() = default;

	glm::vec3 getPosition() { return position; }
	void setPosition(glm::vec3 pos) { position = pos; }

	virtual int getNeighborIndex(LocalDirection dir) = 0;
	virtual void setNeighborIndex(LocalDirection dir, int neighborIndex) = 0;

	virtual MapType getNeighborMap(LocalDirection dir) = 0;
	virtual void setNeighborMap(LocalDirection dir, MapType type) = 0;

	virtual LocalAlignment mapToNeighbor(LocalAlignment alignment, LocalDirection toNeighbor) = 0;

	virtual void wipe() = 0;

	int getIndex() { return index; }
	void setIndex(int i) { index = i; }
};

class CenterNode : public TileNode {
private:
	static const int NUM_NEIGHBORS = 8;
	int neighborIndices[NUM_NEIGHBORS];
	MapType neighborMaps[NUM_NEIGHBORS];
	int tileInfoIndex;

public:
	bool hasEntity;

public:
	CenterNode() : TileNode(NODE_TYPE_CENTER)
	{
		wipe();
	}

	int getNeighborIndex(LocalDirection dir) override { return neighborIndices[dir];	}
	void setNeighborIndex(LocalDirection dir, int neighbor) override { neighborIndices[dir] = neighbor; }

	MapType getNeighborMap(LocalDirection dir) override { return neighborMaps[dir]; }
	void setNeighborMap(LocalDirection dir, MapType map) override { neighborMaps[dir] = map; }

	void wipe() override
	{
		index = -1;
		tileInfoIndex = -1;
		//type = NODE_TYPE_ERROR;
		orientation = ORIENTATION_TYPE_ERROR;
		position = glm::vec3(FLT_MAX, FLT_MAX, FLT_MAX);
		for (int i = 0; i < NUM_NEIGHBORS; i++) {
			neighborIndices[i] = -1;
			neighborMaps[i] = MAP_TYPE_ERROR;
		}
		hasEntity = false;
	}

	LocalAlignment mapToNeighbor(LocalAlignment alignment, LocalDirection toNeighbor) override
	{
		return tnav::map(neighborMaps[toNeighbor], alignment);
	}

	int getTileIndex() { return tileInfoIndex; }
	void setTileInfoIndex(int i) { tileInfoIndex = i; }
};

class SideNode : public TileNode {
private:
	static const int NUM_NEIGHBORS = 2;
	int neighborIndices[NUM_NEIGHBORS];
	MapType neighborMaps[NUM_NEIGHBORS];
	SideTileNodeType sideNodeType;

public:

private:
	int dirToNeighborIndex(LocalDirection dir)
	{
		// side nodes only need to 'see' nodes in two directions, so the neighbors are stored in an
		// array of size 2, but the local direction to that neighbor could be either 0, 1, 2, or 3,
		// depending on the sideNodeType of the side node.

		switch (sideNodeType) {
		case SIDE_NODE_TYPE_HORIZONTAL:
			switch (dir) {
			case LOCAL_DIRECTION_0: return 0;
			case LOCAL_DIRECTION_2: return 1;
			default: return -1;
			}
		case SIDE_NODE_TYPE_VERTICAL:
			switch (dir) {
			case LOCAL_DIRECTION_1: return 0;
			case LOCAL_DIRECTION_3: return 1;
			default: return -1;
			}
		default: return -1;
		}
	}

public:
	SideNode(SideTileNodeType sideNodeType) 
		: TileNode(NODE_TYPE_SIDE)
		, sideNodeType(sideNodeType)
	{
		for (int i = 0; i < NUM_NEIGHBORS; i++) {
			neighborIndices[i] = -1;
			neighborMaps[i] = MAP_TYPE_ERROR;
		}
	}

	SideTileNodeType getSideNodeType() { return sideNodeType; }
	void setSideNodeType(SideTileNodeType type) { sideNodeType = type; }

	int getNeighborIndex(LocalDirection dir) override
	{
		return neighborIndices[dirToNeighborIndex(dir)];
	}

	int getNeighborIndexDirect(int i)
	{
		return neighborIndices[i];
	}

	MapType getNeighborMapDirect(int i)
	{
		return neighborMaps[i];
	}

	LocalDirection getLocalDirDirect(int i)
	{
		switch (sideNodeType) {
		case SIDE_NODE_TYPE_HORIZONTAL:
			switch (i) {
			case 0: return LOCAL_DIRECTION_0;
			case 1: return LOCAL_DIRECTION_2;
			default: return LOCAL_DIRECTION_ERROR;
			}
		case SIDE_NODE_TYPE_VERTICAL:
			switch (i) {
			case 0: return LOCAL_DIRECTION_1;
			case 1: return LOCAL_DIRECTION_3;
			default: return LOCAL_DIRECTION_ERROR;
			}
		default: return LOCAL_DIRECTION_ERROR;
		}
	}

	void setNeighborIndex(LocalDirection dir, int neighborIndex) override
	{
		neighborIndices[dirToNeighborIndex(dir)] = neighborIndex;
	}

	MapType getNeighborMap(LocalDirection dir) override
	{
		return neighborMaps[dirToNeighborIndex(dir)];
	}

	void setNeighborMap(LocalDirection dir, MapType type) override
	{
		neighborMaps[dirToNeighborIndex(dir)] = type;
	}

	LocalAlignment mapToNeighbor(LocalAlignment alignment, LocalDirection toNeighbor) override
	{
		return tnav::map(neighborMaps[dirToNeighborIndex(toNeighbor)], alignment);
	}

	void wipe() override
	{
		index = -1;
		//type = NODE_TYPE_ERROR;
		orientation = ORIENTATION_TYPE_ERROR;
		position = glm::vec3(FLT_MAX, FLT_MAX, FLT_MAX);
		for (int i = 0; i < NUM_NEIGHBORS; i++) {
			neighborIndices[i] = -1;
			neighborMaps[i] = MAP_TYPE_ERROR;
		}
	}
};

// No basis for this node, entities cannot exist whithin it.
class DegenerateCornerNode : public TileNode
{
public:
	// list of indices to the ForceList who CANNOT both be true, as the resulting force would face a degenerate tile.
	// used to invert the force, as the two given indices can determine the two indices not given.
	std::vector<int> componentPairIndices;
	int numDegenComponents;

public:
	DegenerateCornerNode() : TileNode(NODE_TYPE_DEGENERATE)
	{
		// it is assumed that a new degenerate node is connected to only 2 tiles, who must be siblings.
		//componentPairIndices = new int[4];
		componentPairIndices.resize(4);
		numDegenComponents = 4;
	}

	// Unneeded virtual functions:
	int getNeighborIndex(LocalDirection dir) { return -1; }
	void setNeighborIndex(LocalDirection dir, int neighborIndex) {}
	MapType getNeighborMap(LocalDirection dir) { return MAP_TYPE_ERROR; }
	void setNeighborMap(LocalDirection dir, MapType type) {}
	LocalAlignment mapToNeighbor(LocalAlignment alignment, LocalDirection toNeighbor) { return LOCAL_ALIGNMENT_ERROR; }
	void wipe() {}

	void addDegenPair(int componentIndexA1, int componentIndexA2)
	{
		/*int* newList = new int[numDegenComponents + 2];
		for (int i = 0; i < numDegenComponents; i++) newList[i] = componentPairIndices[i];
		newList[numDegenComponents + 0] = componentIndexA;
		newList[numDegenComponents + 1] = componentIndexB;
		numDegenComponents += 2;
		delete[] componentPairIndices;
		componentPairIndices = newList;*/
		componentPairIndices.push_back(componentIndexA1);
		componentPairIndices.push_back(componentIndexA2);
		numDegenComponents += 2;
	}

	// returns false if the node has no pairs to remove/the given pair is not in the list.
	bool removeDegenPair(int componentIndexA, int componentIndexB)
	{
		if (numDegenComponents == 0) return false;

		for (int i = 0; i < numDegenComponents; i++) {
			if (componentPairIndices[i] == componentIndexA ||
				componentPairIndices[i] == componentIndexB) {
				componentPairIndices.erase(componentPairIndices.begin() + i--);
			}
		}
		numDegenComponents -= 2;

		return true;
	}

	int numConnectedTiles() { return numDegenComponents / 2; }

	void resizeComponentList(int newSize) {
		componentPairIndices.resize(newSize);
		numDegenComponents = newSize;
	}
};

class CornerNode : public TileNode {
private:
	static const int NUM_NEIGHBORS = 4;
	int neighborIndices[NUM_NEIGHBORS];
	MapType neighborMaps[NUM_NEIGHBORS];

public:

private:
	// Corner nodes only need to 'see' nodes in 4 directions, so the neighbors are stored in an
	// array of size 4, but the local direction to that neighbor could be either 
	// 0_1, 1_2, 2_3, or 3_0, which map to int values of 4, 5, 6, and 7, respectively.
	// here we simply map the intuative local direction to an index.
	int dirToNeighborIndex(LocalDirection dir)
	{

		switch (dir) {
		case LOCAL_DIRECTION_0_1: return 0;
		case LOCAL_DIRECTION_1_2: return 1;
		case LOCAL_DIRECTION_2_3: return 2;
		case LOCAL_DIRECTION_3_0: return 3;
		default: return -1;
		}
	}

public:
	CornerNode() : TileNode(NODE_TYPE_CORNER)
	{
		for (int i = 0; i < NUM_NEIGHBORS; i++) {
			neighborIndices[i] = -1;
			neighborMaps[i] = MAP_TYPE_ERROR;
		}
	}

	int getNeighborIndex(LocalDirection dir) override
	{
		return neighborIndices[dirToNeighborIndex(dir)];
	}

	void setNeighborIndex(LocalDirection dir, int neighborIndex) override
	{
		neighborIndices[dirToNeighborIndex(dir)] = neighborIndex;
	}

	MapType getNeighborMap(LocalDirection dir) override
	{
		return neighborMaps[dirToNeighborIndex(dir)];
	}

	void setNeighborMap(LocalDirection dir, MapType type) override
	{
		neighborMaps[dirToNeighborIndex(dir)] = type;
	}

	LocalAlignment mapToNeighbor(LocalAlignment alignment, LocalDirection toNeighbor) override
	{
		return tnav::map(neighborMaps[dirToNeighborIndex(toNeighbor)], alignment);
	}

	void wipe() override
	{
		index = -1;
		orientation = ORIENTATION_TYPE_ERROR;
		//type = NODE_TYPE_ERROR;
		position = glm::vec3(FLT_MAX, FLT_MAX, FLT_MAX);
		for (int i = 0; i < NUM_NEIGHBORS; i++) {
			neighborIndices[i] = -1;
			neighborMaps[i] = MAP_TYPE_ERROR;
		}
	}
};

struct alignas(32) GPU_TileNodeInfo {
	alignas(32) int neighborIndices[8];
	alignas(32) int neighborMaps[8];

	alignas(4) int index;
	alignas(4) int tileInfoIndex;
	alignas(4) int padding[6];

	GPU_TileNodeInfo()
	{
		index = -1;
		tileInfoIndex = -1;
		for (int i = 0; i < 8; i++) {
			neighborIndices[i] = -1;
			neighborMaps[i] = -1;
		}
	}

	GPU_TileNodeInfo(TileNode& node)
	{
		index = node.getIndex();

		SideNode* sideNode;
		CornerNode* cornerNode;

		switch (node.type) {
		case NODE_TYPE_CENTER:
			tileInfoIndex = dynamic_cast<CenterNode&>(node).getTileIndex();

			for (LocalDirection d : tnav::DIRECTION_SET) {
				neighborIndices[d] = node.getNeighborIndex(d);
				neighborMaps[d] = node.getNeighborMap(d);
			}
			break;
		case NODE_TYPE_SIDE:
			tileInfoIndex = -1;

			for (LocalDirection d : tnav::DIRECTION_SET) {
				neighborIndices[d] = -1;
				neighborMaps[d] = -1;
			}
			
			sideNode = static_cast<SideNode*>(&node);
			switch (sideNode->getSideNodeType()) {
			case SIDE_NODE_TYPE_HORIZONTAL:
				neighborIndices[LOCAL_DIRECTION_0] = sideNode->getNeighborIndex(LOCAL_DIRECTION_0);
				neighborIndices[LOCAL_DIRECTION_2] = sideNode->getNeighborIndex(LOCAL_DIRECTION_2);

				neighborMaps[LOCAL_DIRECTION_0] = sideNode->getNeighborMap(LOCAL_DIRECTION_0);
				neighborMaps[LOCAL_DIRECTION_2] = sideNode->getNeighborMap(LOCAL_DIRECTION_2);
				break;
			case SIDE_NODE_TYPE_VERTICAL:
				neighborIndices[LOCAL_DIRECTION_1] = sideNode->getNeighborIndex(LOCAL_DIRECTION_1);
				neighborIndices[LOCAL_DIRECTION_3] = sideNode->getNeighborIndex(LOCAL_DIRECTION_3);

				neighborMaps[LOCAL_DIRECTION_1] = sideNode->getNeighborMap(LOCAL_DIRECTION_1);
				neighborMaps[LOCAL_DIRECTION_3] = sideNode->getNeighborMap(LOCAL_DIRECTION_3);
				break;
			}
			break;

		case NODE_TYPE_CORNER:
			tileInfoIndex = -1;

			for (LocalDirection d : tnav::DIRECTION_SET) {
				neighborIndices[d] = -1;
				neighborMaps[d] = -1;
			}

			cornerNode = static_cast<CornerNode*>(&node);
			neighborIndices[LOCAL_DIRECTION_0_1] = cornerNode->getNeighborIndex(LOCAL_DIRECTION_0_1);
			neighborIndices[LOCAL_DIRECTION_1_2] = cornerNode->getNeighborIndex(LOCAL_DIRECTION_1_2);
			neighborIndices[LOCAL_DIRECTION_2_3] = cornerNode->getNeighborIndex(LOCAL_DIRECTION_2_3);
			neighborIndices[LOCAL_DIRECTION_3_0] = cornerNode->getNeighborIndex(LOCAL_DIRECTION_3_0);

			neighborMaps[LOCAL_DIRECTION_0_1] = cornerNode->getNeighborMap(LOCAL_DIRECTION_0_1);
			neighborMaps[LOCAL_DIRECTION_1_2] = cornerNode->getNeighborMap(LOCAL_DIRECTION_1_2);
			neighborMaps[LOCAL_DIRECTION_2_3] = cornerNode->getNeighborMap(LOCAL_DIRECTION_2_3);
			neighborMaps[LOCAL_DIRECTION_3_0] = cornerNode->getNeighborMap(LOCAL_DIRECTION_3_0);
			break;
		}
	}
};