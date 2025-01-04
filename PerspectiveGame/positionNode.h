#pragma once

#include <iostream>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "tileNavigation.h"

enum PositionNodeType {
	NODE_TYPE_CENTER,
	NODE_TYPE_SIDE,
	NODE_TYPE_CORNER,
	NODE_TYPE_ERROR,
};

enum SidePositionNodeType {
	SIDE_NODE_TYPE_HORIZONTAL, 
	SIDE_NODE_TYPE_VERTICAL,
	SIDE_NODE_TYPE_ERROR,
};

class PositionNode {
private:

public:
	PositionNodeType type;
	OrientationType oriType;
	int index;
	glm::vec3 position;

	PositionNode(PositionNodeType t) : type(t) {}
	virtual ~PositionNode() = default;

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

class CenterNode : public PositionNode {
private:
	static const int NUM_NEIGHBORS = 8;
	int neighborIndices[NUM_NEIGHBORS];
	MapType neighborMaps[NUM_NEIGHBORS];
	int tileInfoIndex;

public:
	CenterNode() : PositionNode(NODE_TYPE_CENTER)
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
		oriType = ORIENTATION_TYPE_ERROR;
		position = glm::vec3(FLT_MAX, FLT_MAX, FLT_MAX);
		for (int i = 0; i < NUM_NEIGHBORS; i++) {
			neighborIndices[i] = -1;
			neighborMaps[i] = MAP_TYPE_ERROR;
		}
	}

	LocalAlignment mapToNeighbor(LocalAlignment alignment, LocalDirection toNeighbor) override
	{
		return tnav::map(neighborMaps[toNeighbor], alignment);
	}

	int getTileInfoIndex() { return tileInfoIndex; }
	void setTileInfoIndex(int i) { tileInfoIndex = i; }
};



class SideNode : public PositionNode {
private:
	static const int NUM_NEIGHBORS = 2;
	int neighborIndices[NUM_NEIGHBORS];
	MapType neighborMaps[NUM_NEIGHBORS];
	SidePositionNodeType sideNodeType;

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
	SideNode(SidePositionNodeType sideNodeType) 
		: PositionNode(NODE_TYPE_SIDE)
		, sideNodeType(sideNodeType)
	{
		for (int i = 0; i < NUM_NEIGHBORS; i++) {
			neighborIndices[i] = -1;
			neighborMaps[i] = MAP_TYPE_ERROR;
		}
	}

	SidePositionNodeType getSideNodeType() { return sideNodeType; }
	void setSideNodeType(SidePositionNodeType type) { sideNodeType = type; }

	int getNeighborIndex(LocalDirection dir) override
	{
		return neighborIndices[dirToNeighborIndex(dir)];
	}

	int getNeighborIndexDirect(int i)
	{
		return neighborIndices[i];
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
		oriType = ORIENTATION_TYPE_ERROR;
		position = glm::vec3(FLT_MAX, FLT_MAX, FLT_MAX);
		for (int i = 0; i < NUM_NEIGHBORS; i++) {
			neighborIndices[i] = -1;
			neighborMaps[i] = MAP_TYPE_ERROR;
		}
	}
};

class CornerNode : public PositionNode {
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
	CornerNode() : PositionNode(NODE_TYPE_CORNER)
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
		oriType = ORIENTATION_TYPE_ERROR;
		//type = NODE_TYPE_ERROR;
		position = glm::vec3(FLT_MAX, FLT_MAX, FLT_MAX);
		for (int i = 0; i < NUM_NEIGHBORS; i++) {
			neighborIndices[i] = -1;
			neighborMaps[i] = MAP_TYPE_ERROR;
		}
	}
};








struct TileInfo {
	TileType type;
	int index;
	int centerNodeIndex;
	glm::vec3 color;
	glm::vec2 textureCoordinates[4];
	int siblingIndex;

	TileInfo(TileType type, int index, int siblingIndex, int centerNodeIndex, glm::vec3 color) : 
		type(type), index(index), siblingIndex(siblingIndex), centerNodeIndex(centerNodeIndex), color(color)
	{
		textureCoordinates[0] = glm::vec2(1, 1);
		textureCoordinates[1] = glm::vec2(1, 0);
		textureCoordinates[2] = glm::vec2(0, 0);
		textureCoordinates[3] = glm::vec2(0, 1);
	}

	TileInfo()
	{
		wipe();
		textureCoordinates[0] = glm::vec2(1, 1);
		textureCoordinates[1] = glm::vec2(1, 0);
		textureCoordinates[2] = glm::vec2(0, 0);
		textureCoordinates[3] = glm::vec2(0, 1);
	}

	void wipe()
	{
		index = -1;
		siblingIndex = -1;
		centerNodeIndex = -1;
		type = TILE_TYPE_ERROR;
		color = glm::vec3(FLT_MAX, FLT_MAX, FLT_MAX);
	}
};

struct alignas(32) GPU_PositionNodeInfo {
	alignas(32) int neighborIndices[8];
	alignas(32) int neighborMaps[8];

	alignas(4) int index;
	alignas(4) int tileInfoIndex;
	alignas(4) int padding[6];

	GPU_PositionNodeInfo(PositionNode& node)
	{
		index = node.getIndex();

		SideNode* sideNode;
		CornerNode* cornerNode;

		switch (node.type) {
		case NODE_TYPE_CENTER:
			tileInfoIndex = dynamic_cast<CenterNode&>(node).getTileInfoIndex();

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

struct alignas(32) GPU_TileInfoNode {
	alignas(32) glm::vec2 texCoords[4];

	alignas(16) glm::vec4 color;
	alignas(4) int centerNodeIndex;
	alignas(4) int padding[3];

	GPU_TileInfoNode(TileInfo& info)
	{
		for (int i = 0; i < 4; i++) {
			texCoords[i] = info.textureCoordinates[i];
		}
		color = glm::vec4(info.color, 1.0f);
		centerNodeIndex = info.centerNodeIndex;
	}
};