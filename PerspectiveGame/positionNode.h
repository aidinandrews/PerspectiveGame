#pragma once

#include <iostream>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "tileNavigation.h"

struct PositionNode {
private:
	int index;
	int tileInfoIndex; // -1 if the node is not a central node.
	MappingID mappingID; // indicates the mapping needed to go from this node to others
	glm::vec3 position;
	int neighborIndices[8];
	int neighborMapIndices[8];

public:
	PositionNode()
	{
		wipe();
	}

	glm::vec3 getPosition() { return position; }
	void setPosition(glm::vec3 pos) { position = pos; }

	int getNeighborIndex(LocalDirection toNeighbor) { return neighborIndices[toNeighbor]; }
	void setNeighborIndex(int index, LocalDirection toNeighbor) { neighborIndices[toNeighbor] = index; }

	int getNeighborMap(LocalDirection toNeighbor) { return neighborMapIndices[toNeighbor]; }
	void setNeighborMap(int index, LocalDirection toNeighbor) { neighborMapIndices[toNeighbor] = index; }

	int getIndex() { return index; }
	void setIndex(int index) { this->index = index; }

	int getTileInfoIndex() { return tileInfoIndex; }
	void setTileInfoIndex(int index) { tileInfoIndex = index; }

	MappingID getMappingID() { return mappingID; }
	void setMappingID(MappingID id) { mappingID = id; }

	LocalAlignment mapToNeighbor(LocalAlignment alignment, LocalDirection toNeighbor)
	{
		return tnav::map(neighborMapIndices[toNeighbor], alignment);
	}

	void wipe()
	{
		index = -1;
		tileInfoIndex = -1;
		mappingID = MAP_ID_ERROR;
		position = glm::vec3(FLT_MAX, FLT_MAX, FLT_MAX);
		for (auto d : tnav::DIRECTION_SET) {
			neighborIndices[d] = -1;
			neighborMapIndices[d] = -1;
		}
	}
};

struct TileInfo {
	TileType type;
	int index;
	int nodeIndex;
	glm::vec3 color;
	glm::vec2 textureCoordinates[4];
	int siblingIndex;

	TileInfo(TileType type, int index, int siblingIndex, int nodeIndex, glm::vec3 color) : 
		type(type), index(index), siblingIndex(siblingIndex), nodeIndex(nodeIndex), color(color)
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
		nodeIndex = -1;
		type = TILE_TYPE_ERROR;
		color = glm::vec3(FLT_MAX, FLT_MAX, FLT_MAX);
	}
};

struct alignas(32) GPU_PositionNodeInfo {
	alignas(32) int neighborIndices[8];
	alignas(32) int neighborMapIndices[8];

	alignas(4) int index;
	alignas(4) int tileInfoIndex;
	alignas(4) int padding[6];

	GPU_PositionNodeInfo(PositionNode& node)
	{
		index = node.getIndex();
		tileInfoIndex = node.getTileInfoIndex();

		for (LocalDirection d : tnav::DIRECTION_SET) {
			neighborIndices[d] = node.getNeighborIndex(d);
			neighborMapIndices[d] = node.getNeighborMap(d);
		}
	}
};

struct alignas(32) GPU_TileInfoNode {
	alignas(32) glm::vec2 texCoords[4];

	alignas(16) glm::vec4 color;
	alignas(4) int nodeIndex;
	alignas(4) int padding[3];

	GPU_TileInfoNode(TileInfo& info)
	{
		for (int i = 0; i < 4; i++) {
			texCoords[i] = info.textureCoordinates[i];
		}
		color = glm::vec4(info.color, 1.0f);
		nodeIndex = info.nodeIndex;
	}
};