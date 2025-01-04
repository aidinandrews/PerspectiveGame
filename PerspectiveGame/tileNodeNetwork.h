#pragma once

#include <iostream>
#include <vector>
#include <set>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include "tileNavigation.h"
#include "tileNode.h"
#include "cameraManager.h"

struct TileNodeNetwork {
private:
	std::vector<TileNode> nodes;
	std::vector<int> freeNodeIndices;

	std::vector<TileInfo> tileInfos;
	std::vector<int> freeTileInfoIndices;

	Camera* p_camera;

public: // Rendering:
	GLuint texID;
	GLuint positionNodeInfosBufferID;
	GLuint tileInfosBufferID;
	std::vector<glm::vec2> windowFrustum;

	std::vector<GPU_TileInfo> gpuTileInfos;
	std::vector<GPU_TileNodeInfo> gpuPositionNodeInfos;

	TileNode CurrentNode;
	int currentMapIndex = 0;
	int currentNodeIndex = 4;

public:
	TileNodeNetwork(Camera* c) : p_camera(c)
	{
		createTilePair(glm::vec3(0, 0, 0), TILE_TYPE_XY);
		CurrentNode = nodes[0];

		// Rendering:
		glGenBuffers(1, &positionNodeInfosBufferID);
		glGenBuffers(1, &tileInfosBufferID);
	}

	void update()
	{
		gpuPositionNodeInfos.clear();
		for (TileNode p : nodes) {
			gpuPositionNodeInfos.push_back(GPU_TileNodeInfo(p));
		}

		gpuTileInfos.clear();
		for (TileInfo& i : tileInfos) {
			gpuTileInfos.push_back(GPU_TileInfo(i));
		}
	}

	int size()
	{
		return (int)nodes.size();
	}

	void printSize()
	{
		/*int numCenterNodes = 0, numSideNodes = 0, numCornerNodes = 0;
		for (TileNode* n : nodes) {
			if (n == nullptr) continue;
			switch (n->type) {
			case NODE_TYPE_CENTER: numCenterNodes++; break;
			case NODE_TYPE_SIDE: numSideNodes++; break;
			case NODE_TYPE_CORNER: numCornerNodes++; break;
			}
		}
		std::cout
			<< "\nnum center nodes: " << numCenterNodes
			<< "\nnum side nodes: " << numSideNodes
			<< "\nnum corner nodes: " << numCornerNodes
			<< "\ntotal num nodes: " << numCenterNodes + numSideNodes + numCornerNodes
			<< std::endl;*/
	}

	void printCornerNodePositions()
	{
		/*for (TileNode* n : nodes) {
			if (n != nullptr && n->type == NODE_TYPE_CORNER)
				vechelp::println(n->pos);
		}*/
	}

	TileNode* getNode(int index)
	{
		return &nodes[index];
	}

	TileNode* getNode(TileInfo* info)
	{
		return &nodes[info->getCenterNodeIndex()];
	}

	TileInfo* getTileInfo(int index)
	{
		return &tileInfos[index];
	}

	int numTileInfos() { return (int)tileInfos.size(); }

	TileInfo* getTileInfo(int tileInfoIndex, LocalDirection d)
	{
		return getTileInfo(&tileInfos[tileInfoIndex], d);
	}

	// Gives the tile info of the neighbor tile in the direction of d.
	TileInfo* getTileInfo(TileInfo* info, LocalDirection d)
	{
		return &tileInfos[info->getNeighborIndex(d)];
	}

	TileNode* getFirstNeighbor(TileNode* node, LocalDirection toNeighbor)
	{
		return &nodes[node->getNeighborIndex(toNeighbor)];
	}

	TileNode* getSecondNeighbor(TileNode* node, LocalDirection d)
	{
		TileNode* getFirstNeighbor;

		getFirstNeighbor = &nodes[node->getNeighborIndex(d)];
		d = tnav::map(node->getNeighborMap(d), d);

		return &nodes[getFirstNeighbor->getNeighborIndex(d)];
	}

	MapType getSecondNeighborMap(TileNode* node, LocalDirection d)
	{
		MapType m;
		TileNode* getFirstNeighbor;

		getFirstNeighbor = &nodes[node->getNeighborIndex(d)];
		m = tnav::combine(node->getNeighborMap(d), getFirstNeighbor->getNeighborMap(d));
		d = tnav::map(node->getNeighborMap(d), d);

		return tnav::combine(m, getFirstNeighbor->getNeighborMap(d));
	}

	LocalDirection mapToSecondNeighbor(TileNode* node, LocalDirection toNeighbor, LocalAlignment alignment)
	{
		return tnav::map(getSecondNeighborMap(node, toNeighbor), alignment);
	}

	TileNode* getFourthNeighbor(TileNode* node, LocalDirection d)
	{
		TileNode
			* getFirstNeighbor,
			* secondNeighbor,
			* thirdNeighbor;

		getFirstNeighbor = &nodes[node->getNeighborIndex(d)];
		d = tnav::map(node->getNeighborMap(d), d);

		secondNeighbor = &nodes[getFirstNeighbor->getNeighborIndex(d)];
		d = tnav::map(getFirstNeighbor->getNeighborMap(d), d);

		thirdNeighbor = &nodes[secondNeighbor->getNeighborIndex(d)];
		d = tnav::map(secondNeighbor->getNeighborMap(d), d);

		return &nodes[thirdNeighbor->getNeighborIndex(d)];
	}

	MapType getFourthNeighborMap(TileNode* node, LocalDirection d)
	{
		MapType m = node->getNeighborMap(d);
		TileNode
			* firstNeighbor,
			* secondNeighbor,
			* thirdNeighbor;

		firstNeighbor = &nodes[node->getNeighborIndex(d)];
		d = tnav::map(node->getNeighborMap(d), d);
		m = tnav::combine(m, firstNeighbor->getNeighborMap(d));

		secondNeighbor = &nodes[firstNeighbor->getNeighborIndex(d)];
		d = tnav::map(firstNeighbor->getNeighborMap(d), d);
		m = tnav::combine(m, secondNeighbor->getNeighborMap(d));

		thirdNeighbor = &nodes[secondNeighbor->getNeighborIndex(d)];
		d = tnav::map(secondNeighbor->getNeighborMap(d), d);
		return tnav::combine(m, thirdNeighbor->getNeighborMap(d));
	}

	LocalDirection mapToFourthNeighbor(TileNode* node, LocalDirection toNeighbor, LocalAlignment alignment)
	{
		return tnav::map(getFourthNeighborMap(node, toNeighbor), alignment);
	}
	 
	// Will add a node to the nodes list that is unconnected and error prone if it is not connected up!
	// returns an index to the added node.
	int addNode(glm::vec3 pos, TileNodeType type, BasisType basis)
	{
		int r;

		if (freeNodeIndices.size() > 0) {
			r = freeNodeIndices.back();
			nodes[r].setIndex(freeNodeIndices.back()); // all freeNode nodes are wiped
			nodes[r].resizeArrays();
			freeNodeIndices.pop_back();
		}
		else {
			r = (int)nodes.size();
			nodes.push_back(TileNode(r, type, basis, pos));
			nodes.back().setIndex(r);
		}

		nodes[r].pos = pos;
		nodes[r].type = type;
		nodes[r].basis = basis;

		return r;
	}

	//// center nodes inherantly require more information, hence the inputs:
	//int addCenterNode(glm::vec3 pos, BasisType basisType)
	//{
	//	int i = addNode(NODE_TYPE_CENTER);
	//	nodes[i]->setPosition(pos);
	//	nodes[i]->basisType = NODE_TYPE_CENTER;
	//	nodes[i]->basis = basisType;
	//	return i;
	//}

	//int addSideNode(glm::vec3 pos, BasisType type)
	//{
	//	int i = addNode(NODE_TYPE_SIDE);
	//	nodes[i]->setPosition(pos);
	//	nodes[i]->basisType = NODE_TYPE_SIDE;
	//	nodes[i]->basis = type;
	//	return i;
	//}

	//int addCornerNode(glm::vec3 pos, BasisType type)
	//{
	//	int i = addNode(NODE_TYPE_CORNER);
	//	nodes[i]->setPosition(pos);
	//	nodes[i]->basisType = NODE_TYPE_CORNER;
	//	nodes[i]->basis = type;
	//	return i;
	//}

	void removeNode(int index)
	{
		freeNodeIndices.push_back(index);
		nodes[index].wipe();
	}

	void colorTile(int index)
	{

		switch (tileInfos[index].getType()) {
		case TILE_TYPE_XYF: tileInfos[index].setColor(glm::vec3(1.0f, 0.0f, 0.0f)); break;
		case TILE_TYPE_XYB: tileInfos[index].setColor(glm::vec3(0.5f, 0.0f, 0.0f)); break;
		case TILE_TYPE_XZF: tileInfos[index].setColor(glm::vec3(0.0f, 1.0f, 0.0f)); break;
		case TILE_TYPE_XZB: tileInfos[index].setColor(glm::vec3(0.0f, 0.5f, 0.0f)); break;
		case TILE_TYPE_YZF: tileInfos[index].setColor(glm::vec3(0.0f, 0.0f, 1.0f)); break;
		case TILE_TYPE_YZB: tileInfos[index].setColor(glm::vec3(0.0f, 0.0f, 0.5f)); break;
		}
	}

	void removeTileInfo(int index)
	{
		tileInfos[index].wipe();
		freeTileInfoIndices.push_back(index);
	}

	// Adds an UNINITIALIZED tile info to the tileInfos list.  Returns a pointer to the added tileInfo.
	// frontIndex and backIndex are meant to be used as returns.
	void addTilePair(int frontCenterNodeIndex, int backCenterNodeIndex, SuperTileType type, int& frontInfoIndex, int& backInfoIndex, BasisType frontBasis, BasisType backBasis)
	{
		if (freeTileInfoIndices.size() > 1) {
			frontInfoIndex = freeTileInfoIndices.back();
			tileInfos[frontInfoIndex].setIndex(freeTileInfoIndices.back());
			freeTileInfoIndices.pop_back();

			backInfoIndex = freeTileInfoIndices.back();
			tileInfos[backInfoIndex].setIndex(freeTileInfoIndices.back());
			freeTileInfoIndices.pop_back();
		}
		else if (freeTileInfoIndices.size() == 1) {
			frontInfoIndex = freeTileInfoIndices.back();
			tileInfos[frontInfoIndex].setIndex(freeTileInfoIndices.back());
			freeTileInfoIndices.pop_back();

			tileInfos.push_back(TileInfo());
			backInfoIndex = (int)tileInfos.size() - 1;
			tileInfos[backInfoIndex].setIndex((int)tileInfos.size() - 1);
		}
		else {
			tileInfos.push_back(TileInfo());
			frontInfoIndex = (int)tileInfos.size() - 1;
			tileInfos[frontInfoIndex].setIndex((int)tileInfos.size() - 1);

			tileInfos.push_back(TileInfo());
			backInfoIndex = (int)tileInfos.size() - 1;
			tileInfos[backInfoIndex].setIndex((int)tileInfos.size() - 1);
		}

		tileInfos[frontInfoIndex].setCenterNodeIndex(frontCenterNodeIndex);
		tileInfos[frontInfoIndex].setType(tnav::getFrontTileType(type));
		tileInfos[frontInfoIndex].setSiblingIndex(backInfoIndex);
		tileInfos[frontInfoIndex].basis = frontBasis;

		tileInfos[backInfoIndex].setCenterNodeIndex(backCenterNodeIndex);
		tileInfos[backInfoIndex].setType(tnav::getBackTileType(type));
		tileInfos[backInfoIndex].setSiblingIndex(frontInfoIndex);
		tileInfos[backInfoIndex].basis = backBasis;


		colorTile(frontInfoIndex);
		colorTile(backInfoIndex);
	}

	// given a position in space, returns all the tiles connected to that point in the network.
	// currently assumes the given position is of a side node!  will error if given a position 
	// relating to a center of corner node.
	std::vector<TileInfo> getConnectedTiles(TileNode* node)
	{
		std::vector<TileInfo> connectedTiles;

		if (node->type != TILE_NODE_TYPE_LRG_EDGE)
			return connectedTiles;

		int sideNodeIndex = node->index;
		glm::vec3 pos = nodes[sideNodeIndex].getPosition();

		for (TileNode& n : nodes) {
			if (n.getPosition() != pos || n.getIndex() == sideNodeIndex) continue;

			for (LocalDirection d : tnav::ORTHOGONAL_DIRECTION_SET) {
				if (n.getNeighborIndex(d) == -1) continue;
				TileNode* neighbor = getSecondNeighbor(&n, d);
				// The other possibility is a large corner node or nullptr
				if (neighbor != nullptr && neighbor->type == TILE_NODE_TYPE_LRG_CNTR)
					connectedTiles.push_back(tileInfos[neighbor->getTileInfoIndex(0)]);
			}
		}

		return connectedTiles;
	}

	// returns how prioritized the connection between these two tiles should be.
	// 0 == high prio, 1 == medium, 2 = low, 3 = should not be connected in the first place!
	int getConnectionPrio(TileInfo* a, TileInfo* b)
	{

		glm::vec3 aN = tnav::getNormal(a->type);
		glm::vec3 aP = getNode(a->centerNodeIndex)->getPosition();
		glm::vec3 bN = tnav::getNormal(b->type);
		glm::vec3 bP = getNode(b->centerNodeIndex)->getPosition();
		if (aP + (0.5f * aN) == bP + (0.5f * bN)) {
			// | a
			// |-> 
			// |___^___ b     inner connection
			return 0;	
		}
		else if (aN == bN && glm::length(aP - bP) == 1.0f) {
			SuperTileType superType = tnav::getSuperTileType(a->type);
			if ((superType == TILE_TYPE_XY && aP.z == bP.z) ||
				(superType == TILE_TYPE_XZ && aP.y == bP.y) ||
				(superType == TILE_TYPE_YZ && aP.x == bP.x)) {
				//
				// a__^__.__^__b   flat connection
				return 1;
			}
		}
		else if (aP - (0.5f * aN) == bP - (0.5f * bN)) {
			//  a|
			// <-| 
			//   |_______
			//       V  b   outer connection
			return 2;
		}
		// no possible connection
		return INT_MAX;
	}

	// returns a pointer to the center node of the newly created tile or nullptr if the tile could not be made.
	TileInfo* createTilePair(glm::vec3 pos, SuperTileType type)
	{
		// check if there is already a tile where we are trying to add one:
		// increment by 2s since we always add/remove 2 tiles at a time (front and back).
		for (int i = 0; i < tileInfos.size(); i += 2) {
			if (tileInfos[i].index == -1) continue;
			if (nodes[tileInfos[i].centerNodeIndex].getPosition() == pos)
				return nullptr;
		}

		// create the new tile pair and center nodes:
		// indices are used because apparently pointers are unsafe if the vector resizes itself.
		TileType frontBasis = tnav::getTileType(type, true);
		TileType backBasis = tnav::getTileType(type, false);
		int
			frontNodeIndex = addNode(pos, TILE_NODE_TYPE_LRG_CNTR, frontBasis),
			backNodeIndex = addNode(pos, TILE_NODE_TYPE_LRG_CNTR, backBasis),
			//frontCornerDir01 = addNode(pos, TILE_NODE_TYPE_MED, frontBasis),
			//frontCornerDir12 = addNode(pos, TILE_NODE_TYPE_MED, frontBasis),
			//frontCornerDir23 = addNode(pos, TILE_NODE_TYPE_MED, frontBasis),
			//frontCornerDir30 = addNode(pos, TILE_NODE_TYPE_MED, frontBasis),
			//backCornerDir01 = addNode(pos, TILE_NODE_TYPE_MED, backBasis),
			//backCornerDir12 = addNode(pos, TILE_NODE_TYPE_MED, backBasis),
			//backCornerDir23 = addNode(pos, TILE_NODE_TYPE_MED, backBasis),
			//backCornerDir30 = addNode(pos, TILE_NODE_TYPE_MED, backBasis),
			frontSideDir0 = addNode(pos, TILE_NODE_TYPE_SML_INNR_HZTL, frontBasis),
			frontSideDir1 = addNode(pos, TILE_NODE_TYPE_SML_INNR_VTCL, frontBasis),
			frontSideDir2 = addNode(pos, TILE_NODE_TYPE_SML_INNR_HZTL, frontBasis),
			frontSideDir3 = addNode(pos, TILE_NODE_TYPE_SML_INNR_VTCL, frontBasis),
			backSideDir0 = addNode(pos, TILE_NODE_TYPE_SML_INNR_HZTL, backBasis),
			backSideDir1 = addNode(pos, TILE_NODE_TYPE_SML_INNR_VTCL, backBasis),
			backSideDir2 = addNode(pos, TILE_NODE_TYPE_SML_INNR_HZTL, backBasis),
			backSideDir3 = addNode(pos, TILE_NODE_TYPE_SML_INNR_VTCL, backBasis);

		// connect these mf's up:
		for (LocalDirection d : tnav::DIRECTION_SET) {
			getNode(frontNodeIndex)->setNeighborMap(d, MAP_TYPE_IDENTITY);
			getNode(backNodeIndex)->setNeighborMap(d, MAP_TYPE_IDENTITY);
		}

		getNode(frontNodeIndex)->setNeighborIndex(LOCAL_DIRECTION_0, frontSideDir0);
		getNode(frontNodeIndex)->setNeighborIndex(LOCAL_DIRECTION_1, frontSideDir1);
		getNode(frontNodeIndex)->setNeighborIndex(LOCAL_DIRECTION_2, frontSideDir2);
		getNode(frontNodeIndex)->setNeighborIndex(LOCAL_DIRECTION_3, frontSideDir3);
		//getNode(frontNodeIndex)->setNeighborIndex(LOCAL_DIRECTION_0_1, frontCornerDir01);
		//getNode(frontNodeIndex)->setNeighborIndex(LOCAL_DIRECTION_1_2, frontCornerDir12);
		//getNode(frontNodeIndex)->setNeighborIndex(LOCAL_DIRECTION_2_3, frontCornerDir23);
		//getNode(frontNodeIndex)->setNeighborIndex(LOCAL_DIRECTION_3_0, frontCornerDir30);

		getNode(backNodeIndex)->setNeighborIndex(LOCAL_DIRECTION_0, backSideDir0);
		getNode(backNodeIndex)->setNeighborIndex(LOCAL_DIRECTION_1, backSideDir1);
		getNode(backNodeIndex)->setNeighborIndex(LOCAL_DIRECTION_2, backSideDir2);
		getNode(backNodeIndex)->setNeighborIndex(LOCAL_DIRECTION_3, backSideDir3);
		//getNode(backNodeIndex)->setNeighborIndex(LOCAL_DIRECTION_0_1, backCornerDir01);
		//getNode(backNodeIndex)->setNeighborIndex(LOCAL_DIRECTION_1_2, backCornerDir12);
		//getNode(backNodeIndex)->setNeighborIndex(LOCAL_DIRECTION_2_3, backCornerDir23);
		//getNode(backNodeIndex)->setNeighborIndex(LOCAL_DIRECTION_3_0, backCornerDir30);

		getNode(frontSideDir0)->setNeighborIndex(LOCAL_DIRECTION_2, frontNodeIndex);
		getNode(frontSideDir1)->setNeighborIndex(LOCAL_DIRECTION_3, frontNodeIndex);
		getNode(frontSideDir2)->setNeighborIndex(LOCAL_DIRECTION_0, frontNodeIndex);
		getNode(frontSideDir3)->setNeighborIndex(LOCAL_DIRECTION_1, frontNodeIndex);
		getNode(frontSideDir0)->setNeighborMap(LOCAL_DIRECTION_2, MAP_TYPE_IDENTITY);
		getNode(frontSideDir1)->setNeighborMap(LOCAL_DIRECTION_3, MAP_TYPE_IDENTITY);
		getNode(frontSideDir2)->setNeighborMap(LOCAL_DIRECTION_0, MAP_TYPE_IDENTITY);
		getNode(frontSideDir3)->setNeighborMap(LOCAL_DIRECTION_1, MAP_TYPE_IDENTITY);
		//getNode(frontCornerDir01)->setNeighborIndex(LOCAL_DIRECTION_2_3, frontNodeIndex);
		//getNode(frontCornerDir12)->setNeighborIndex(LOCAL_DIRECTION_3_0, frontNodeIndex);
		//getNode(frontCornerDir23)->setNeighborIndex(LOCAL_DIRECTION_0_1, frontNodeIndex);
		//getNode(frontCornerDir30)->setNeighborIndex(LOCAL_DIRECTION_1_2, frontNodeIndex);

		getNode(backSideDir0)->setNeighborIndex(LOCAL_DIRECTION_2, backNodeIndex);
		getNode(backSideDir1)->setNeighborIndex(LOCAL_DIRECTION_3, backNodeIndex);
		getNode(backSideDir2)->setNeighborIndex(LOCAL_DIRECTION_0, backNodeIndex);
		getNode(backSideDir3)->setNeighborIndex(LOCAL_DIRECTION_1, backNodeIndex);
		getNode(backSideDir0)->setNeighborMap(LOCAL_DIRECTION_2, MAP_TYPE_IDENTITY);
		getNode(backSideDir1)->setNeighborMap(LOCAL_DIRECTION_3, MAP_TYPE_IDENTITY);
		getNode(backSideDir2)->setNeighborMap(LOCAL_DIRECTION_0, MAP_TYPE_IDENTITY);
		getNode(backSideDir3)->setNeighborMap(LOCAL_DIRECTION_1, MAP_TYPE_IDENTITY);
		//getNode(backCornerDir01)->setNeighborIndex(LOCAL_DIRECTION_2_3, backNodeIndex);
		//getNode(backCornerDir12)->setNeighborIndex(LOCAL_DIRECTION_3_0, backNodeIndex);
		//getNode(backCornerDir23)->setNeighborIndex(LOCAL_DIRECTION_0_1, backNodeIndex);
		//getNode(backCornerDir30)->setNeighborIndex(LOCAL_DIRECTION_1_2, backNodeIndex);

		int newFrontTileIndex = -1, newBackTileIndex = -1;
		addTilePair(frontNodeIndex, backNodeIndex, type, newFrontTileIndex, newBackTileIndex, frontBasis, backBasis);
		nodes[frontNodeIndex].setTileInfoIndex(0, newFrontTileIndex);
		nodes[backNodeIndex].setTileInfoIndex(0, newBackTileIndex);

		connectTilePair(&tileInfos[newFrontTileIndex]);
		reconnectCornerNodes(&tileInfos[newFrontTileIndex]);
		reconnectCornerNodes(&tileInfos[newBackTileIndex]);

		reconnectTileInfo(&tileInfos[newFrontTileIndex]);
		reconnectTileInfo(&tileInfos[newBackTileIndex]);

		return &tileInfos[newFrontTileIndex];
	}

	void reconnectTileInfo(TileInfo* tile)
	{
		TileNode* center = getNode(tile->centerNodeIndex);
		for (LocalDirection d : tnav::ORTHOGONAL_DIRECTION_SET) {
			tile->setNighborIndex(d, getFourthNeighbor(center, d)->getTileInfoIndex(0));
			tile->setNeighborMap(d, getFourthNeighborMap(center, d));
		}
	}

	// Given a tile pair, will connect OR reconnect all the side nodes of that tile to the world.
	// function assumes that the center and inner ring of nodes are connected already.
	void connectTilePair(TileInfo* frontTile)
	{
		TileInfo* backTile = getTileInfo(frontTile->siblingIndex);
		TileNode* frontCenterNode = getNode(frontTile->centerNodeIndex);
		TileNode* backCenterNode = getNode(backTile->centerNodeIndex);
		TileNode* newSideNode;

		for (LocalDirection d : tnav::ORTHOGONAL_DIRECTION_SET) {
			int innerFrontSmlNodeIndex = getFirstNeighbor(frontCenterNode, d)->index;
			int innerBackSmlNodeIndex = getFirstNeighbor(backCenterNode, d)->index;

			// add a new side node is none exists:
			if (getNode(innerFrontSmlNodeIndex)->getNeighborIndex(d) == -1) {
				const glm::vec3* toSidesOffsets = tnav::getNodePositionOffsets(tnav::getSuperTileType(frontTile->type));
				glm::vec3 sidePos = frontCenterNode->getPosition() + toSidesOffsets[d];
				newSideNode = &nodes[addNode(sidePos, TILE_NODE_TYPE_LRG_EDGE, BASIS_TYPE_ERROR)];
				newSideNode->setPosition(sidePos);
			}
			else { // if one exists, just use it:
				newSideNode = getNode(frontCenterNode->getNeighborIndex(d));
			}
			newSideNode->basis = frontCenterNode->basis; // Arbitrary but convenient later.

			std::vector<TileInfo> linkedTiles = getConnectedTiles(newSideNode);
			//LocalDirection cwDiagNodeDir = tnav::combine(d, LocalDirection((d + 1) % 4));
			//LocalDirection ccwDiagNodeDir = tnav::combine(d, LocalDirection((d + 3) % 4));
			if (linkedTiles.size() == 0) {
				// connect the side to just the new tile pair:
				MapType flipMap = tnav::getNeighborMap(d, d);

				getNode(innerFrontSmlNodeIndex)->setNeighborIndex(d, newSideNode->getIndex());
				getNode(innerFrontSmlNodeIndex)->setNeighborMap(d, MAP_TYPE_IDENTITY);

				getNode(innerBackSmlNodeIndex)->setNeighborIndex(d, newSideNode->getIndex());
				getNode(innerBackSmlNodeIndex)->setNeighborMap(d, flipMap);

				//TileNode* frontInnerCornerNode1 = getFirstNeighbor(frontCenterNode, cwDiagNodeDir);
				//frontInnerCornerNode1->setNeighborIndex(ccwDiagNodeDir, newSideNode->index);
				//frontInnerCornerNode1->setNeighborMap(ccwDiagNodeDir, MAP_TYPE_IDENTITY);

				//TileNode* frontInnerCornerNode2 = getFirstNeighbor(frontCenterNode, ccwDiagNodeDir);
				//frontInnerCornerNode2->setNeighborIndex(cwDiagNodeDir, newSideNode->index);
				//frontInnerCornerNode2->setNeighborMap(cwDiagNodeDir, MAP_TYPE_IDENTITY);

				//TileNode* backInnerCornerNode1 = getFirstNeighbor(backCenterNode, cwDiagNodeDir);
				//backInnerCornerNode1->setNeighborIndex(ccwDiagNodeDir, newSideNode->index);
				//backInnerCornerNode1->setNeighborMap(ccwDiagNodeDir, flipMap);

				//TileNode* backInnerCornerNode2 = getFirstNeighbor(backCenterNode, ccwDiagNodeDir);
				//backInnerCornerNode1->setNeighborIndex(cwDiagNodeDir, newSideNode->index);
				//backInnerCornerNode1->setNeighborMap(cwDiagNodeDir, flipMap);
				
				newSideNode->setNeighborIndex(d, getNode(innerBackSmlNodeIndex)->getIndex());
				newSideNode->setNeighborIndex(tnav::inverse(d), getNode(innerFrontSmlNodeIndex)->getIndex());
				//newSideNode->setNeighborIndex(tnav::inverse(ccwDiagNodeDir), frontInnerCornerNode1->getIndex());
				//newSideNode->setNeighborIndex(tnav::inverse(cwDiagNodeDir), frontInnerCornerNode2->getIndex());
				//newSideNode->setNeighborIndex(cwDiagNodeDir, backInnerCornerNode1->getIndex());
				//newSideNode->setNeighborIndex(ccwDiagNodeDir, backInnerCornerNode2->getIndex());
				newSideNode->setNeighborMap(d, flipMap);
				newSideNode->setNeighborMap(tnav::inverse(d), MAP_TYPE_IDENTITY);
				//newSideNode->setNeighborMap(tnav::inverse(ccwDiagNodeDir), MAP_TYPE_IDENTITY);
				//newSideNode->setNeighborMap(tnav::inverse(cwDiagNodeDir), MAP_TYPE_IDENTITY);
				//newSideNode->setNeighborMap(cwDiagNodeDir, flipMap);
				//newSideNode->setNeighborMap(ccwDiagNodeDir, flipMap);

				continue;
			}

			// reconnect tiles based on heirarchy:
			for (TileInfo& linkedTile : linkedTiles) {
				LocalDirection linkedTileDir;
				TileNode* linkedTileCenterNode = getNode(linkedTile.centerNodeIndex);
				TileNode* linkedTileInnerSideNode = getFirstNeighbor(linkedTileCenterNode, d);

				for (LocalDirection dir : tnav::ORTHOGONAL_DIRECTION_SET) {
					if (getSecondNeighbor(linkedTileCenterNode, dir)->getPosition() == newSideNode->getPosition()) {
						linkedTileDir = dir;
						break;
					}
				}

				TileInfo* linkedTileNeighbor = getTileInfo(&linkedTile, linkedTileDir);
				TileNode* linkedTileNeighborCenterNode = getNode(linkedTileNeighbor->centerNodeIndex);

				int existingConnectionPrio = tnav::getConnectionPriority(linkedTile.type, linkedTileNeighbor->type);
				int newConnectionPrio1 = tnav::getConnectionPriority(linkedTile.type, getTileInfo(frontTile->index)->type);
				int newConnectionPrio2 = tnav::getConnectionPriority(linkedTile.type, getTileInfo(backTile->index)->type);
				if (existingConnectionPrio < newConnectionPrio1 && existingConnectionPrio < newConnectionPrio2) {
					// the current connection is of heavier prio, so we wont switch.
					continue;
				}

				TileNode* linkedSideNode = getSecondNeighbor(linkedTileCenterNode, linkedTileDir);
				LocalDirection
					linkedSideNodeOut = mapToSecondNeighbor(linkedTileCenterNode, linkedTileDir, linkedTileDir),
					linkedNeighborOut = mapToSecondNeighbor(linkedSideNode, linkedSideNodeOut, tnav::inverse(linkedSideNodeOut)),
					oppD = tnav::inverse(d);
				TileNode* linkedNeighborInnerSideNode = getFirstNeighbor(linkedTileNeighborCenterNode, linkedNeighborOut);

				// we need to connect this side node to the new tile pair:
				if (newConnectionPrio1 < newConnectionPrio2) {
					// connectedSide connects ti to the new front tile in the direction of connectedSideOut
					linkedSideNode->setNeighborIndex(linkedSideNodeOut, getNode(innerFrontSmlNodeIndex)->getIndex());
					linkedSideNode->setNeighborMap(linkedSideNodeOut, tnav::getNeighborMap(linkedSideNodeOut, d));

					getNode(innerFrontSmlNodeIndex)->setNeighborIndex(d, linkedSideNode->getIndex());
					getNode(innerFrontSmlNodeIndex)->setNeighborMap(d, tnav::getNeighborMap(d, linkedSideNodeOut));

					// n connects the new back tile to currentNeighbor
					newSideNode->setNeighborIndex(oppD, linkedNeighborInnerSideNode->getIndex()); // n has same mapping as front tile center node.
					newSideNode->setNeighborMap(oppD, tnav::getNeighborMap(oppD, linkedNeighborOut));
					newSideNode->setNeighborIndex(d, getNode(innerBackSmlNodeIndex)->getIndex());
					newSideNode->setNeighborMap(d, tnav::getNeighborMap(d, d));

					linkedNeighborInnerSideNode->setNeighborIndex(linkedNeighborOut, newSideNode->getIndex());
					linkedNeighborInnerSideNode->setNeighborMap(linkedNeighborOut, tnav::getNeighborMap(linkedNeighborOut, oppD));

					getNode(innerBackSmlNodeIndex)->setNeighborIndex(d, newSideNode->getIndex());
					getNode(innerBackSmlNodeIndex)->setNeighborMap(d, tnav::getNeighborMap(d, d));
				}
				else {
					// linkedSideNode connects linkedTile to the new back tile in the direction of connectedSideOut:
					linkedSideNode->basis = backCenterNode->basis;
					linkedSideNode->setNeighborIndex(oppD, getNode(innerBackSmlNodeIndex)->index);
					linkedSideNode->setNeighborMap(oppD, MAP_TYPE_IDENTITY);
					linkedSideNode->setNeighborIndex(d, linkedTileInnerSideNode->getIndex());
					linkedSideNode->setNeighborMap(d, tnav::getNeighborMap(d, linkedTileDir));

					linkedTileInnerSideNode->setNeighborMap(linkedTileDir, tnav::getNeighborMap(linkedTileDir, d));

					getNode(innerBackSmlNodeIndex)->setNeighborIndex(d, linkedSideNode->getIndex());
					getNode(innerBackSmlNodeIndex)->setNeighborMap(d, MAP_TYPE_IDENTITY);

					// newNode connects the new front tile to currentNeighbor:
					newSideNode->setNeighborIndex(d, linkedNeighborInnerSideNode->getIndex()); // n has same mapping as front tile center node.
					newSideNode->setNeighborMap(d, tnav::getNeighborMap(d, linkedNeighborOut));
					newSideNode->setNeighborIndex(oppD, getNode(innerFrontSmlNodeIndex)->getIndex());
					newSideNode->setNeighborMap(oppD, MAP_TYPE_IDENTITY);

					linkedNeighborInnerSideNode->setNeighborIndex(linkedNeighborOut, newSideNode->getIndex());
					linkedNeighborInnerSideNode->setNeighborMap(linkedNeighborOut, tnav::getNeighborMap(linkedNeighborOut, d));

					getNode(innerFrontSmlNodeIndex)->setNeighborIndex(d, newSideNode->getIndex());
					getNode(innerFrontSmlNodeIndex)->setNeighborMap(d, MAP_TYPE_IDENTITY);
				}
				break;
			}
		}
	}

	void reconnectCornerNodes(TileInfo* frontTileInfo)
	{
		//LrgNode* frontNode = static_cast<LrgNode*>(getNode(frontTileInfo->centerNodeIndex));

		//// remove all corner nodes associated with this tile pair
		//std::set<int> deleteList;
		//for (LocalDirection d : tnav::ORTHOGONAL_DIRECTION_SET) {
		//	LrgNode* neighborCenterNode = getFourthNeighbor(frontNode, d);
		//	LocalDirection 
		//		D = tnav::map(getFourthNeighborMap(frontNode, d), tnav::inverse(d)),
		//		diagonal1 = tnav::combine(D, LocalDirection((D + 1) % 4)),
		//		diagonal2 = tnav::combine(D, LocalDirection((D + 3) % 4));

		//	deleteList.insert(neighborCenterNode->getNeighborIndex(diagonal1));
		//	deleteList.insert(neighborCenterNode->getNeighborIndex(diagonal2));
		//}
		//deleteList.erase(-1); // gets rid of all invalid indices.

		//// removes all connections to the corner nodes associated with this corner pair:
		//for (int i : deleteList) {
		//	MedNode* c = static_cast<MedNode*>(getNode(i));
		//	// remove centerNode -> this corner node connections:
		//	for (LocalDirection d : tnav::DIAGONAL_DIRECTION_SET) { // ALL corner nodes MUST have 4 connections definitially
		//		LocalDirection D = tnav::map(c->getNeighborMap(d), tnav::inverse(d));
		//		int index = c->getNeighborIndex(d);
		//		if (index == -1) continue;
		//		LrgNode* centerNode = static_cast<LrgNode*>(getNode(index));
		//		centerNode->setNeighborIndex(D, -1);
		//		centerNode->setNeighborMap(D, MAP_TYPE_ERROR);
		//	}
		//	removeNode(c->index);
		//}

		//// cycle through all 4 possible corner nodes and re-add them if they are valid
		//for (LocalDirection d : tnav::ORTHOGONAL_DIRECTION_SET) {
		//	// could be that the deletion earlier resized the underlying node vector:
		//	frontNode = static_cast<LrgNode*>(getNode(frontTileInfo->centerNodeIndex));
		//	LocalDirection
		//		centralNorth = d,
		//		centralEast = LocalDirection((d + 1) % 4),
		//		centralSouth = LocalDirection((d + 2) % 4),
		//		centralWest = LocalDirection((d + 3) % 4),
		//		centralNorthEast = tnav::combine(centralNorth, centralEast),
		//		centralNorthWest = tnav::combine(centralNorth, centralWest),
		//		centralSouthEast = tnav::combine(centralSouth, centralEast),
		//		centralSouthWest = tnav::combine(centralSouth, centralWest);
		//	LrgNode
		//		* northNode = static_cast<LrgNode*>(getFourthNeighbor(frontNode, centralNorth)),
		//		* eastNode = static_cast<LrgNode*>(getFourthNeighbor(frontNode, centralEast));
		//	MapType
		//		goNorth = getFourthNeighborMap(frontNode, centralNorth),
		//		goEast = getFourthNeighborMap(frontNode, centralEast);
		//	LocalDirection
		//		toNorthEast = tnav::map(goNorth, centralEast),
		//		toEastNorth = tnav::map(goEast, centralNorth);
		//	LrgNode
		//		* northEastNode = static_cast<LrgNode*>(getFourthNeighbor(northNode, toNorthEast)),
		//		* eastNorthNode = static_cast<LrgNode*>(getFourthNeighbor(eastNode, toEastNorth));

		//	if (northEastNode != eastNorthNode || northEastNode == frontNode) 
		//		continue; // this corner is degenerate!
		//	
		//	MapType
		//		northToNorthEast = getFourthNeighborMap(northNode, toNorthEast),
		//		goNorthEast = tnav::combine(goNorth, northToNorthEast);

		//	glm::vec3 newCornerNodePosition = frontNode->pos
		//		+ tnav::globalDirToVec3(tnav::localToGlobalDir(frontNode->basis, centralNorth)) / 2.0f
		//		+ tnav::globalDirToVec3(tnav::localToGlobalDir(frontNode->basis, centralEast)) / 2.0f;

		//	int northNodeIndex = northNode->index,
		//		eastNodeIndex = eastNode->index,
		//		northEastNodeIndex = northEastNode->index,
		//		newCornerNodeIndex = addCornerNode(newCornerNodePosition, frontNode->basis);

		//	// addNode() may have resized the underlying vector that keeps nodes, making the pointers
		//	// error-prone.  just re-find here to avoid potential issues:
		//	northNode = static_cast<LrgNode*>(getNode(northNodeIndex));
		//	eastNode = static_cast<LrgNode*>(getNode(eastNodeIndex));
		//	northEastNode = static_cast<LrgNode*>(getNode(northEastNodeIndex));

		//	// stitch everything together finally:
		//	MedNode* newCornerNode = static_cast<MedNode*>(getNode(newCornerNodeIndex));
		//	newCornerNode->setNeighborIndex(centralSouthWest, frontNode->index);
		//	newCornerNode->setNeighborMap(centralSouthWest, MAP_TYPE_IDENTITY);
		//	newCornerNode->setNeighborIndex(centralNorthWest, northNode->index);
		//	newCornerNode->setNeighborMap(centralNorthWest, goNorth);
		//	newCornerNode->setNeighborIndex(centralSouthEast, eastNode->index);
		//	newCornerNode->setNeighborMap(centralSouthEast, goEast);
		//	newCornerNode->setNeighborIndex(centralNorthEast, northEastNode->index);
		//	newCornerNode->setNeighborMap(centralNorthEast, goNorthEast);

		//	frontNode->setNeighborIndex(centralNorthEast, newCornerNode->index);
		//	frontNode->setNeighborMap(centralNorthEast, MAP_TYPE_IDENTITY);

		//	LocalDirection toCornerNode = tnav::map(goNorth, centralSouthEast);
		//	northNode->setNeighborIndex(toCornerNode, newCornerNode->index);
		//	northNode->setNeighborMap(toCornerNode, tnav::inverse(newCornerNode->getNeighborMap(centralNorthWest)));

		//	toCornerNode = tnav::map(goEast, centralNorthWest);
		//	eastNode->setNeighborIndex(toCornerNode, newCornerNode->index);
		//	eastNode->setNeighborMap(toCornerNode, tnav::inverse(newCornerNode->getNeighborMap(centralSouthEast)));

		//	toCornerNode = tnav::map(goNorthEast, centralSouthWest);
		//	northEastNode->setNeighborIndex(toCornerNode, newCornerNode->index);
		//	northEastNode->setNeighborMap(toCornerNode, tnav::inverse(newCornerNode->getNeighborMap(centralNorthEast)));
		//}
	}

	void removeTilePair(int tileInfoIndex) { removeTilePair(&tileInfos[tileInfoIndex]); }

	void removeTilePair(TileInfo* t)
	{
		//using namespace tnav;

		//if (t == nullptr) return;

		//TileInfo* siblingTile = getTileInfo(t->siblingIndex);
		//int centerNodeIndex = (t->centerNodeIndex);
		//int sibCenterNodeIndex = (siblingTile->centerNodeIndex);

		//std::set<int> affectedCenterNodeIndices; // for managing degenerate corners later

		//// reconnect edges:
		//for (LocalDirection d : tnav::ORTHOGONAL_DIRECTION_SET) {
		//	TileInfo
		//		* neighborTile1 = getTileInfo(t, d),
		//		* neighborTile2 = getTileInfo(t->siblingIndex, d);
		//	if (neighborTile1 == siblingTile) {
		//		// edges that dont connect to anything can be simply removed, 
		//		// as no reconnection is necessary:
		//		removeNode(getNode(centerNodeIndex)->getNeighborIndex(d));
		//		continue;
		//	}

		//	affectedCenterNodeIndices.insert(neighborTile1->centerNodeIndex);
		//	affectedCenterNodeIndices.insert(neighborTile2->centerNodeIndex);

		//	LrgNode* centerNode1 = static_cast<LrgNode*>(getNode(centerNodeIndex));
		//	SmlNode* sideNode1 = static_cast<SmlNode*>(getNode(centerNode1->getNeighborIndex(d)));
		//	LrgNode* neighborCenterNode1 = static_cast<LrgNode*>(getNode(neighborTile1->centerNodeIndex));
		//	LrgNode* centerNode2 = static_cast<LrgNode*>(getNode(sibCenterNodeIndex));
		//	SmlNode* sideNode2 = static_cast<SmlNode*>(getNode(centerNode2->getNeighborIndex(d)));
		//	LrgNode* neighborCenterNode2 = static_cast<LrgNode*>(getNode(neighborTile2->centerNodeIndex));
		//	SmlNode* newNode = static_cast<SmlNode*>(getNode(addNode(NODE_TYPE_SIDE)));
		//	
		//	LocalDirection 
		//		n1ToNew = mapToFourthNeighbor(centerNode1, d, inverse(d)),
		//		n2ToNew = mapToFourthNeighbor(centerNode2, d, inverse(d)),
		//		newToN1 = inverse(n1ToNew),
		//		newToN2 = n1ToNew;
		//	
		//	MapType 
		//		newToN2Map = getNeighborMap(newToN2, n2ToNew),
		//		n2ToNewMap = inverse(newToN2Map);
		//	
		//	// While we could edit the existing nodes, its conceptually simpler to just make a fresh one:
		//	newNode->setPosition(sideNode1->getPosition());
		//	newNode->basis = neighborCenterNode1->basis; // makes sure the transition from center node type -> new side node type is possible
		//	newNode->setSideNodeType(
		//		(n1ToNew == LOCAL_DIRECTION_0 || n1ToNew == LOCAL_DIRECTION_2)
		//		? SIDE_NODE_TYPE_HORIZONTAL
		//		: SIDE_NODE_TYPE_VERTICAL);

		//	neighborCenterNode1->setNeighborIndex(n1ToNew, newNode->getIndex());
		//	neighborCenterNode1->setNeighborMap(n1ToNew, MAP_TYPE_IDENTITY);
		//	newNode->setNeighborIndex(newToN2, neighborCenterNode2->getIndex());
		//	newNode->setNeighborMap(newToN2, newToN2Map);
		//	neighborCenterNode2->setNeighborIndex(n2ToNew, newNode->getIndex());
		//	neighborCenterNode2->setNeighborMap(n2ToNew, n2ToNewMap);
		//	newNode->setNeighborIndex(newToN1, neighborCenterNode1->getIndex());
		//	newNode->setNeighborMap(newToN1, MAP_TYPE_IDENTITY);

		//	removeNode(sideNode1->getIndex());
		//	removeNode(sideNode2->getIndex());
		//}

		//for (LocalDirection d : tnav::DIAGONAL_DIRECTION_SET) {
		//	LrgNode* c = static_cast<LrgNode*>(getNode(centerNodeIndex));
		//	if (c->getNeighborIndex(d) != -1) {
		//		LocalDirection D = tnav::map(c->getNeighborMap(d), tnav::inverse(d));
		//		MedNode* C = static_cast<MedNode*>(getNode(c->getNeighborIndex(d)));
		//		C->setNeighborIndex(D, -1);
		//		C->setNeighborMap(D, MAP_TYPE_ERROR);
		//	}

		//	c = static_cast<LrgNode*>(getNode(sibCenterNodeIndex));
		//	if (c->getNeighborIndex(d) != -1) {
		//		LocalDirection D = tnav::map(c->getNeighborMap(d), tnav::inverse(d));
		//		MedNode* C = static_cast<MedNode*>(getNode(c->getNeighborIndex(d)));
		//		C->setNeighborIndex(D, -1);
		//		C->setNeighborMap(D, MAP_TYPE_ERROR);
		//	}
		//}
		//
		//removeTileInfo(static_cast<LrgNode*>(getNode(centerNodeIndex))->getTileInfoIndex());
		//removeTileInfo(static_cast<LrgNode*>(getNode(sibCenterNodeIndex))->getTileInfoIndex());
		//removeNode(centerNodeIndex);
		//removeNode(sibCenterNodeIndex);

		//for (int i : affectedCenterNodeIndices) {
		//	reconnectCornerNodes(getTileInfo(static_cast<LrgNode*>(getNode(i))->getTileInfoIndex()));
		//}
	}
};