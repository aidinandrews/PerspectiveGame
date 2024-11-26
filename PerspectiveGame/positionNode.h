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
		index = -1;
		tileInfoIndex = -1;
		mappingID = MAP_ID_ERROR;
		position = glm::vec3(FLT_MAX, FLT_MAX, FLT_MAX);
		for (int i = 0; i < 8; i++) {
			neighborIndices[i] = -1;
			neighborMapIndices[i] = -1;
		}
	}

	PositionNode(int index, int tileInfoIndex, MappingID id, glm::vec3 position,
				 int neighborIndex0, int neighborMapIndex0, int neighborIndex1, int neighborMapIndex1,
				 int neighborIndex2, int neighborMapIndex2, int neighborIndex3, int neighborMapIndex3,
				 int neighborIndex01, int neighborMapIndex01, int neighborIndex12, int neighborMapIndex12,
				 int neighborIndex23, int neighborMapIndex23, int neighborIndex30, int neighborMapIndex30)
		: index(index), tileInfoIndex(tileInfoIndex), mappingID(id), position(position)
	{
		neighborIndices[0] = neighborIndex0;
		neighborIndices[1] = neighborIndex1;
		neighborIndices[2] = neighborIndex2;
		neighborIndices[3] = neighborIndex3;
		neighborIndices[4] = neighborIndex01;
		neighborIndices[5] = neighborIndex12;
		neighborIndices[6] = neighborIndex23;
		neighborIndices[7] = neighborIndex30;

		neighborMapIndices[0] = neighborMapIndex0;
		neighborMapIndices[1] = neighborMapIndex1;
		neighborMapIndices[2] = neighborMapIndex2;
		neighborMapIndices[3] = neighborMapIndex3;
		neighborMapIndices[4] = neighborMapIndex01;
		neighborMapIndices[5] = neighborMapIndex12;
		neighborMapIndices[6] = neighborMapIndex23;
		neighborMapIndices[7] = neighborMapIndex30;
	}

	glm::vec3 getPosition() { return position; }
	void setPosition(glm::vec3 pos) { position = pos; }

	int getNeighborIndex(LocalDirection toNeighbor) { return neighborIndices[toNeighbor]; }
	void setNeighborIndex(int index, LocalDirection toNeighbor) { neighborIndices[toNeighbor] = index; }

	int getNeighborMap(LocalDirection toNeighbor) { return neighborMapIndices[toNeighbor]; }
	void setNeighborMapID(int index, LocalDirection toNeighbor) { neighborMapIndices[toNeighbor] = index; }

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
};

struct TileInfo {
	TileType type;
	int index;
	int nodeIndex;
	glm::vec3 tileColor;
	glm::vec2 textureCoordinates[4];

	TileInfo(TileType type, int index, int nodeIndex, glm::vec3 tileColor) : type(type), index(index), nodeIndex(nodeIndex), tileColor(tileColor)
	{
		textureCoordinates[0] = glm::vec2(1, 1);
		textureCoordinates[1] = glm::vec2(1, 0);
		textureCoordinates[2] = glm::vec2(0, 0);
		textureCoordinates[3] = glm::vec2(0, 1);
	}

	TileInfo() : type(TILE_TYPE_ERROR), index(-1), nodeIndex(-1), tileColor(glm::vec3(FLT_MAX, FLT_MAX, FLT_MAX))
	{
		textureCoordinates[0] = glm::vec2(1, 1);
		textureCoordinates[1] = glm::vec2(1, 0);
		textureCoordinates[2] = glm::vec2(0, 0);
		textureCoordinates[3] = glm::vec2(0, 1);
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
		color = glm::vec4(info.tileColor, 1.0f);
		nodeIndex = info.nodeIndex;
	}
};