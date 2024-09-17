#pragma once

#include <iostream>
#include <vector>

struct MetaPositionSideNode;
struct MetaPositionCornerNode;

struct MetaPositionCenterNode {
	float directionMagnitudes[4];
	int neighborAlignmentMaps[16];

	MetaPositionSideNode* sideNodeNeighbors[8];
	MetaPositionCenterNode* centerNodeNeighbors[8];
};

struct MetaPositionSideNode {
	float directionMagnitudes[4];
	int neighborAlignmentMaps[16];

	MetaPositionSideNode* sideNodeNeighbors[8];
	MetaPositionCenterNode* centerNodeNeighbors[4];
	MetaPositionCornerNode* cornerNodeNeighbors[4];
};

struct MetaPositionCornerNode {
	float directionMagnitudes[4];
	int neighborAlignmentMaps[16];

	MetaPositionSideNode* sideNodeNeighbors[8];
	MetaPositionCornerNode* cornerNodeNeighbors[8];
};

// This manager is responsible for collision solving.  
// Once solved, it will be referenced to update the direciton values for each moving entity.
struct MetaPositionNodeManager 
{
	std::vector<MetaPositionCenterNode> centerNodes;
	std::vector<int> freeCenterNodeIndices;

	std::vector<MetaPositionSideNode> sideNodes;
	std::vector<int> freeSideNodeIndices;

	std::vector<MetaPositionCornerNode> cornerNodes;
	std::vector<int> freeCornerNodeIndices;

	std::vector<int> potentialCollisionNodeIndices;

	void solveCollisions()
	{

	}
};