#pragma once

#include <iostream>
#include <vector>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include "tileNavigation.h"
#include "positionNode.h"

struct PositionNodeNetwork {
private:
	std::vector<PositionNode> nodes;
	std::vector<int> freeNodeIndices;

	std::vector<TileInfo> tileInfos;
	std::vector<int> freeTileInfoIndices;

public: // Rendering:
	GLuint texID;
	GLuint positionNodeInfosBufferID;
	GLuint tileInfosBufferID;
	std::vector<glm::vec2> windowFrustum;

	std::vector<GPU_TileInfoNode> gpuTileInfos;
	std::vector<GPU_PositionNodeInfo> gpuPositionNodeInfos;

	PositionNode CurrentNode;
	int currentMapIndex = 0;
	int currentNodeIndex = 4;

public:

	PositionNodeNetwork()
	{
		// Initially, there must be at least one tile pair:
		/* side '0' node:     */ nodes.push_back(PositionNode(0, -1, glm::vec3(0.5, 0.0, 0.0), 5, 6, -1, -1, 4, 0, -1, -1, 1, 2, 1, 0, 3, 0, 3, 2));
		/* side '1' node:     */ nodes.push_back(PositionNode(1, -1, glm::vec3(0.0, -0.5, 0.0), -1, -1, 5, 4, -1, -1, 4, 0, 0, 2, 2, 2, 2, 0, 0, 0));
		/* side '2' node:     */ nodes.push_back(PositionNode(2, -1, glm::vec3(-0.5, 0.0, 0.0), 4, 0, -1, -1, 5, 6, -1, -1, 1, 0, 1, 2, 5, 6, 3, 0));
		/* side '3' node:     */ nodes.push_back(PositionNode(3, -1, glm::vec3(0.0, 0.5, 0.0), -1, -1, 4, 0, -1, -1, 5, 4, 0, 0, 2, 0, 2, 2, 0, 2));
		/* front center node: */ nodes.push_back(PositionNode(4, 0, glm::vec3(0.0, 0.0, 0.0), 0, 0, 1, 0, 2, 0, 3, 0, -1, -1, -1, -1, -1, -1, -1, -1));
		/* back center node:  */ nodes.push_back(PositionNode(5, 1, glm::vec3(0.0, 0.0, 0.0), 0, 6, 1, 4, 2, 6, 3, 4, -1, -1, -1, -1, -1, -1, -1, -1));

		tileInfos.push_back(TileInfo(4, TILE_TYPE_XYF, glm::vec3(0, 0, 1)));
		tileInfos.push_back(TileInfo(4, TILE_TYPE_XYB, glm::vec3(0, 0, 0.5)));

		// Rendering:
		glGenBuffers(1, &positionNodeInfosBufferID);
		glGenBuffers(1, &tileInfosBufferID);
	}

	void update()
	{
		gpuPositionNodeInfos.clear();
		for (PositionNode& p : nodes) {
			gpuPositionNodeInfos.push_back(GPU_PositionNodeInfo(p));
		}

		gpuTileInfos.clear();
		for (TileInfo& i : tileInfos) {
			gpuTileInfos.push_back(GPU_TileInfoNode(i));
		}
	}

	PositionNode* getNode(int index)
	{
		return &nodes[index];
	}

	TileInfo* getTileInfo(int index)
	{
		return &tileInfos[index];
	}

	PositionNode* getNeighbor(PositionNode* node, LocalDirection toNeighbor)
	{
		return &nodes[node->getNeighborIndex(toNeighbor)];
	}
};