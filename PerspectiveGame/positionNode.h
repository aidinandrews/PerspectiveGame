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
	glm::vec3 position;
	int neighborIndices[8];
	int neighborMapIndices[8];

public:
	PositionNode()
	{
		index = -1;
		position = glm::vec3(0, 0, 0);
		for (int i = 0; i < 8; i++) {
			neighborIndices[i] = -1;
			neighborMapIndices[i] = -1;
		}
	}

	PositionNode(int index, int tileInfoIndex, glm::vec3 position,
				 int neighborIndex0, int neighborMapIndex0, int neighborIndex1, int neighborMapIndex1,
				 int neighborIndex2, int neighborMapIndex2, int neighborIndex3, int neighborMapIndex3,
				 int neighborIndex01, int neighborMapIndex01, int neighborIndex12, int neighborMapIndex12,
				 int neighborIndex23, int neighborMapIndex23, int neighborIndex30, int neighborMapIndex30)
		: index(index), tileInfoIndex(tileInfoIndex), position(position)
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

	LocalAlignment mapToNeighbor(LocalAlignment alignment, LocalDirection toNeighbor)
	{
		return tnav::getMappedAlignment(neighborMapIndices[toNeighbor], alignment);
	}

	int getNeighborIndex(LocalDirection toNeighbor) { return neighborIndices[toNeighbor]; }
	int getNeighborMapIndex(LocalDirection toNeighbor) { return neighborMapIndices[toNeighbor]; }

	int getIndex() { return index; }
	int getTileInfoIndex() { return tileInfoIndex; }
};

struct TileInfo {
	int nodeIndex;
	TileType tileType;
	glm::vec3 tileColor;
	glm::vec2 textureCoordinates[4];

	TileInfo(int nodeIndex, TileType tileType, glm::vec3 tileColor) : nodeIndex(nodeIndex), tileType(tileType), tileColor(tileColor)
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

		for (int i = 0; i < 8; i++) {
			neighborIndices[i] = node.getNeighborIndex(LocalDirection(i));
			neighborMapIndices[i] = node.getNeighborMapIndex(LocalDirection(i));
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