#pragma once

#include <iostream>
#include <vector>
#include <set>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include "tileNavigation.h"
#include "positionNode.h"
#include "cameraManager.h"

struct PositionNodeNetwork {
private:
	std::vector<PositionNode*> nodes;
	std::vector<int> freeNodeIndices;

	std::vector<TileInfo> tileInfos;
	std::vector<int> freeTileInfoIndices;

	Camera* p_camera;

public: // Rendering:
	GLuint texID;
	GLuint positionNodeInfosBufferID;
	GLuint tileInfosBufferID;
	std::vector<glm::vec2> windowFrustum;

	std::vector<GPU_TileInfoNode> gpuTileInfos;
	std::vector<GPU_PositionNodeInfo> gpuPositionNodeInfos;

	CenterNode CurrentNode;
	int currentMapIndex = 0;
	int currentNodeIndex = 4;

public:
	PositionNodeNetwork(Camera* c) : p_camera(c)
	{
		createTilePair(glm::vec3(0, 0, 0), TILE_TYPE_XY);

		// Rendering:
		glGenBuffers(1, &positionNodeInfosBufferID);
		glGenBuffers(1, &tileInfosBufferID);
	}

	void update()
	{
		gpuPositionNodeInfos.clear();
		for (PositionNode* p : nodes) {
			if (p == nullptr) 
				gpuPositionNodeInfos.push_back(GPU_PositionNodeInfo());
			else
				gpuPositionNodeInfos.push_back(GPU_PositionNodeInfo(*p));
		}

		gpuTileInfos.clear();
		for (TileInfo& i : tileInfos) {
			gpuTileInfos.push_back(GPU_TileInfoNode(i));
		}
	}

	int size()
	{
		return nodes.size();
	}

	void printSize()
	{
		int numCenterNodes = 0, numSideNodes = 0, numCornerNodes = 0;
		for (PositionNode* n : nodes) {
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
			<< std::endl;
	}

	void printCornerNodePositions()
	{
		for (PositionNode* n : nodes) {
			if (n != nullptr && n->type == NODE_TYPE_CORNER)
				vechelp::println(n->position);
	}
	}

	PositionNode* getNode(int index)
	{
		return nodes[index];
	}

	CenterNode* getNode(TileInfo* info)
	{
		return static_cast<CenterNode*>(nodes[info->centerNodeIndex]);
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
		CenterNode* node = static_cast<CenterNode*>(nodes[info->centerNodeIndex]);
		LocalDirection d2 = tnav::map(node->getNeighborMap(d), d);
		SideNode* sideNode = static_cast<SideNode*>(nodes[node->getNeighborIndex(d)]);
		CenterNode* neighborCenterNode = static_cast<CenterNode*>(nodes[sideNode->getNeighborIndex(d2)]);
		return &tileInfos[neighborCenterNode->getTileInfoIndex()];
	}

	PositionNode* getNeighbor(PositionNode* node, LocalDirection toNeighbor)
	{
		return nodes[node->getNeighborIndex(toNeighbor)];
	}

	CenterNode* getSecondNeighbor(CenterNode* node, LocalDirection toNeighbor)
	{
		SideNode* sideNode = static_cast<SideNode*>(nodes[node->getNeighborIndex(toNeighbor)]);
		LocalDirection d = tnav::map(node->getNeighborMap(toNeighbor), toNeighbor);
		return static_cast<CenterNode*>(nodes[sideNode->getNeighborIndex(d)]);
	}

	LocalDirection mapToSecondNeighbor(CenterNode* node, LocalDirection toNeighbor, LocalAlignment alignment)
	{
		SideNode* sideNode = static_cast<SideNode*>(nodes[node->getNeighborIndex(toNeighbor)]);
		alignment = tnav::map(node->getNeighborMap(toNeighbor), alignment);

		toNeighbor = tnav::map(node->getNeighborMap(toNeighbor), toNeighbor);
		return tnav::map(sideNode->getNeighborMap(toNeighbor), alignment);;
	}

	// Will add a node to the nodes list that is unconnected and error prone if it is not connected up!
	// returns an index to the added node.
	int addNode(PositionNodeType type)
	{
		PositionNode* node = nullptr;
		switch (type) {
		case NODE_TYPE_CENTER:
			node = new CenterNode();
			break;
		case NODE_TYPE_SIDE:
			node = (new SideNode(SIDE_NODE_TYPE_ERROR));
			break;
		default: //case NODE_TYPE_CORNER:
			node = (new CornerNode());
			break;
	}

		if (freeNodeIndices.size() > 0) {
			nodes[freeNodeIndices.back()] = node;
			node->setIndex(freeNodeIndices.back()); // all freeNode nodes are wiped
			freeNodeIndices.pop_back();
		}
		else {
			nodes.push_back(node);
			nodes.back()->setIndex((int)nodes.size() - 1);
		}

		return node->getIndex();
	}

	// center nodes inherantly require more information, hence the inputs:
	int addCenterNode(glm::vec3 pos, TileType type)
	{
		int i = addNode(NODE_TYPE_CENTER);
		nodes[i]->setPosition(pos);
		nodes[i]->type = NODE_TYPE_CENTER;
		nodes[i]->oriType = type;
		return i;
	}

	int addSideNode(glm::vec3 pos, OrientationType type)
	{
		int i = addNode(NODE_TYPE_SIDE);
		nodes[i]->setPosition(pos);
		nodes[i]->type = NODE_TYPE_SIDE;
		nodes[i]->oriType = type;
		return i;
	}
	 
	int addCornerNode(glm::vec3 pos, OrientationType type)
	{
		int i = addNode(NODE_TYPE_CORNER);
		nodes[i]->setPosition(pos);
		nodes[i]->type = NODE_TYPE_CORNER;
		nodes[i]->oriType = type;
		return i;
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
		delete nodes[index];
	}

	void colorTile(int index)
	{

		switch (tileInfos[index].type) {
		case TILE_TYPE_XYF: tileInfos[index].color = glm::vec3(1.0f, 0.0f, 0.0f); break;
		case TILE_TYPE_XYB: tileInfos[index].color = glm::vec3(0.5f, 0.0f, 0.0f); break;
		case TILE_TYPE_XZF: tileInfos[index].color = glm::vec3(0.0f, 1.0f, 0.0f); break;
		case TILE_TYPE_XZB: tileInfos[index].color = glm::vec3(0.0f, 0.5f, 0.0f); break;
		case TILE_TYPE_YZF: tileInfos[index].color = glm::vec3(0.0f, 0.0f, 1.0f); break;
		case TILE_TYPE_YZB: tileInfos[index].color = glm::vec3(0.0f, 0.0f, 0.5f); break;
		}
	}

	void removeTileInfo(int index)
	{
		tileInfos[index].wipe();
		freeTileInfoIndices.push_back(index);
	}

	// Adds an UNINITIALIZED tile info to the tileInfos list.  Returns a pointer to the added tileInfo.
	// frontIndex and backIndex are meant to be used as returns.
	void addTileInfoPair(int frontCenterNodeIndex, int backCenterNodeIndex, SuperTileType type, int& frontInfoIndex, int& backInfoIndex)
	{
		if (freeTileInfoIndices.size() > 1) {
			frontInfoIndex = freeTileInfoIndices.back();
			tileInfos[frontInfoIndex].index = freeTileInfoIndices.back();
			freeTileInfoIndices.pop_back();

			backInfoIndex = freeTileInfoIndices.back();
			tileInfos[backInfoIndex].index = freeTileInfoIndices.back();
			freeTileInfoIndices.pop_back();
		}
		else if (freeTileInfoIndices.size() == 1) {
			frontInfoIndex = freeTileInfoIndices.back();
			tileInfos[frontInfoIndex].index = freeTileInfoIndices.back();
			freeTileInfoIndices.pop_back();

			tileInfos.push_back(TileInfo());
			backInfoIndex = (int)tileInfos.size() - 1;
			tileInfos[backInfoIndex].index = (int)tileInfos.size() - 1;
		}
		else {
			tileInfos.push_back(TileInfo());
			frontInfoIndex = (int)tileInfos.size() - 1;
			tileInfos[frontInfoIndex].index = (int)tileInfos.size() - 1;

			tileInfos.push_back(TileInfo());
			backInfoIndex = (int)tileInfos.size() - 1;
			tileInfos[backInfoIndex].index = (int)tileInfos.size() - 1;
		}

		tileInfos[frontInfoIndex].centerNodeIndex = frontCenterNodeIndex;
		tileInfos[frontInfoIndex].type = tnav::getFrontTileType(type);
		tileInfos[frontInfoIndex].siblingIndex = backInfoIndex;

		tileInfos[backInfoIndex].centerNodeIndex = backCenterNodeIndex;
		tileInfos[backInfoIndex].type = tnav::getBackTileType(type);
		tileInfos[backInfoIndex].siblingIndex = frontInfoIndex;


		colorTile(frontInfoIndex);
		colorTile(backInfoIndex);
	}

	// given a position in space, returns all the tiles connected to that point in the network.
	// currently assumes the given position is of a side node!  will error if given a position 
	// relating to a center of corner node.
	std::vector<TileInfo> getConnectedTiles(SideNode* node)
	{
		std::vector<TileInfo> connectedTiles;
		int sideNodeIndex = node->index;
		glm::vec3 pos = nodes[sideNodeIndex]->getPosition();

		for (PositionNode* n : nodes) {
			if (n == nullptr || n->getPosition() != pos || n->getIndex() == sideNodeIndex)
========
		for (PositionNode* n : nodes) {
			if (n->getPosition() != pos || n->getIndex() == sideNodeIndex)
>>>>>>>> 91f7b3b7202d67259637fd123642c59c777ed463:PerspectiveGame/positionNodeNetwork.h
				continue;

			SideNode* s = static_cast<SideNode*>(n);
			for (int i = 0; i < 2; i++) { // side nodes only have 2 neighbors
				if (s->getNeighborIndexDirect(i) == -1)
					continue;

				connectedTiles.push_back(
					tileInfos[
						static_cast<CenterNode*>(nodes[s->getNeighborIndexDirect(i)])->getTileInfoIndex()
					]
				);
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
	void connectTilePair(TileInfo* frontTile)
	{
		TileInfo* backTile = getTileInfo(frontTile->siblingIndex);
		CenterNode* frontCenterNode = static_cast<CenterNode*>(getNode(frontTile->centerNodeIndex));
		CenterNode* backCenterNode = static_cast<CenterNode*>(getNode(backTile->centerNodeIndex));
		SideNode* newSideNode; 

		for (LocalDirection d : tnav::ORTHOGONAL_DIRECTION_SET) {
			int innerFrontSmlNodeIndex = getFirstNeighbor(frontCenterNode, d)->index;
			int innerBackSmlNodeIndex = getFirstNeighbor(backCenterNode, d)->index;

			// add a new side node is none exists:
			if (frontCenterNode->getNeighborIndex(d) == -1) {
				const glm::vec3* toSidesOffsets = tnav::getNodePositionOffsets(tnav::getSuperTileType(frontTile->type));
				glm::vec3 sidePos = frontCenterNode->getPosition() + toSidesOffsets[d];
				newSideNode = static_cast<SideNode*>(nodes[addSideNode(sidePos, ORIENTATION_TYPE_ERROR)]);
				newSideNode->setPosition(sidePos);
			}
			else { // if one exists, just use it:
				newSideNode = static_cast<SideNode*>(getNode(frontCenterNode->getNeighborIndex(d)));
			}
			newSideNode->oriType = frontCenterNode->oriType; // Arbitrary but convenient later.
			newSideNode->setSideNodeType(
				(d == LOCAL_DIRECTION_0 || d == LOCAL_DIRECTION_2)
				? SIDE_NODE_TYPE_HORIZONTAL
				: SIDE_NODE_TYPE_VERTICAL);

			std::vector<TileInfo> linkedTiles = getConnectedTiles(newSideNode);
			if (linkedTiles.size() == 0) {
				// connect the side to just the new tile pair:
				frontCenterNode->setNeighborIndex(d, newSideNode->getIndex());
				frontCenterNode->setNeighborMap(d, MAP_TYPE_IDENTITY);

				backCenterNode->setNeighborIndex(d, newSideNode->getIndex());
				backCenterNode->setNeighborMap(d, tnav::getNeighborMap(d, d));

				newSideNode->setNeighborIndex(tnav::inverse(d), frontCenterNode->getIndex());
				newSideNode->setNeighborMap(tnav::inverse(d), MAP_TYPE_IDENTITY);
				newSideNode->setNeighborIndex(d, backCenterNode->getIndex());
				newSideNode->setNeighborMap(d, tnav::getNeighborMap(d, d));
				continue;
			}

			// reconnect tiles based on heirarchy:
			for (TileInfo linkedTile : linkedTiles) {
				LocalDirection linkedTileDir;
				CenterNode* linkedTileCenterNode = static_cast<CenterNode*>(getNode(linkedTile.centerNodeIndex));

				for (LocalDirection dir : tnav::ORTHOGONAL_DIRECTION_SET) {
					if (nodes[linkedTileCenterNode->getNeighborIndex(dir)]->getPosition() == newSideNode->getPosition()) {
						linkedTileDir = dir;
						break;
					}
				}
				TileInfo* currentNeighbor = getTileInfo(&linkedTile, linkedTileDir);
				CenterNode* currentNeighborCenterNode = getNode(currentNeighbor);

				int currentConnectionPrio = getConnectionPrio(&linkedTile, currentNeighbor);
				int newConnectionPrio1 = getConnectionPrio(&linkedTile, getTileInfo(frontTile->index));
				int newConnectionPrio2 = getConnectionPrio(&linkedTile, getTileInfo(backTile->index));
				if (currentConnectionPrio < newConnectionPrio1 && currentConnectionPrio < newConnectionPrio2) {
					// the current connection is of heavier prio, so we wont switch.
					continue;
				}

				SideNode* linkedSideNode = static_cast<SideNode*>(getNode(linkedTileCenterNode->getNeighborIndex(linkedTileDir)));
				LocalDirection linkedSideNodeOut = tnav::map(linkedTileCenterNode->getNeighborMap(linkedTileDir), linkedTileDir);
				LocalDirection currentNeighborOut = tnav::map(linkedSideNode->getNeighborMap(linkedSideNodeOut), tnav::inverse(linkedSideNodeOut));

				LocalDirection oppD = tnav::inverse(d);

				// we need to connect this side node to the new tile pair:
				if (newConnectionPrio1 < newConnectionPrio2) {
					// connectedSide connects ti to the new front tile in the direction of connectedSideOut
					linkedSideNode->setNeighborIndex(linkedSideNodeOut, frontCenterNode->getIndex());
					linkedSideNode->setNeighborMap(linkedSideNodeOut, tnav::getNeighborMap(linkedSideNodeOut, d));

					frontCenterNode->setNeighborIndex(d, linkedSideNode->getIndex());
					frontCenterNode->setNeighborMap(d, tnav::getNeighborMap(d, linkedSideNodeOut));

					// n connects the new back tile to currentNeighbor
					newSideNode->setNeighborIndex(oppD, currentNeighborCenterNode->getIndex()); // n has same mapping as front tile center node.
					newSideNode->setNeighborMap(oppD, tnav::getNeighborMap(oppD, currentNeighborOut));
					newSideNode->setNeighborIndex(d, backCenterNode->getIndex());
					newSideNode->setNeighborMap(d, tnav::getNeighborMap(d, d));

					currentNeighborCenterNode->setNeighborIndex(currentNeighborOut, newSideNode->getIndex());
					currentNeighborCenterNode->setNeighborMap(currentNeighborOut, tnav::getNeighborMap(currentNeighborOut, oppD));

					backCenterNode->setNeighborIndex(d, newSideNode->getIndex());
					backCenterNode->setNeighborMap(d, tnav::getNeighborMap(d, d));
				}
				else {
					// linkedSideNode connects linkedTile to the new back tile in the direction of connectedSideOut:
					linkedSideNode->oriType = backCenterNode->oriType;
					linkedSideNode->setSideNodeType(
						(d == LOCAL_DIRECTION_0 || d == LOCAL_DIRECTION_2)
						? SIDE_NODE_TYPE_HORIZONTAL
						: SIDE_NODE_TYPE_VERTICAL);
					linkedSideNode->setNeighborIndex(oppD, backCenterNode->index);
					linkedSideNode->setNeighborMap(oppD, MAP_TYPE_IDENTITY);
					linkedSideNode->setNeighborIndex(d, linkedTileCenterNode->getIndex());
					linkedSideNode->setNeighborMap(d, tnav::getNeighborMap(d, linkedTileDir));

					linkedTileCenterNode->setNeighborMap(linkedTileDir, tnav::getNeighborMap(linkedTileDir, d));

					backCenterNode->setNeighborIndex(d, linkedSideNode->getIndex());
					backCenterNode->setNeighborMap(d, MAP_TYPE_IDENTITY);

					// newNode connects the new front tile to currentNeighbor:
					newSideNode->setNeighborIndex(d, currentNeighborCenterNode->getIndex()); // n has same mapping as front tile center node.
					newSideNode->setNeighborMap(d, tnav::getNeighborMap(d, currentNeighborOut));
					newSideNode->setNeighborIndex(oppD, frontCenterNode->getIndex());
					newSideNode->setNeighborMap(oppD, MAP_TYPE_IDENTITY);

					currentNeighborCenterNode->setNeighborIndex(currentNeighborOut, newSideNode->getIndex());
					currentNeighborCenterNode->setNeighborMap(currentNeighborOut, tnav::getNeighborMap(currentNeighborOut, d));

					frontCenterNode->setNeighborIndex(d, newSideNode->getIndex());
					frontCenterNode->setNeighborMap(d, MAP_TYPE_IDENTITY);
				}
				break;
			}
		}
	}

	// returns a pointer to the center node of the newly created tile or nullptr if the tile could not be made.
	TileInfo* createTilePair(glm::vec3 pos, SuperTileType type)
	{
		// check if there is already a tile where we are trying to add one:
		// increment by 2s since we always add/remove 2 tiles at a time (front and back).
		for (int i = 0; i < tileInfos.size(); i += 2) {
			if (tileInfos[i].index == -1) continue;
			if (nodes[tileInfos[i].centerNodeIndex]->getPosition() == pos)
				return nullptr;
		}

		// create the new tile pair and center nodes:
		// indices are used because apparently pointers are unsafe if the vector resizes itself.
		TileType frontType = tnav::getTileType(type, true);
		TileType backType = tnav::getTileType(type, false);
		int newFrontNodeIndex = addCenterNode(pos, frontType);
		int newBackNodeIndex = addCenterNode(pos, backType);
		int newFrontTileIndex, newBackTileIndex;
		addTileInfoPair(newFrontNodeIndex, newBackNodeIndex, type, newFrontTileIndex, newBackTileIndex);
		static_cast<CenterNode*>(nodes[newFrontNodeIndex])->setTileInfoIndex(newFrontTileIndex);
		static_cast<CenterNode*>(nodes[newBackNodeIndex])->setTileInfoIndex(newBackTileIndex);

		connectTilePair(&tileInfos[newFrontTileIndex]);
		reconnectCornerNodes(&tileInfos[newFrontTileIndex]);
		reconnectCornerNodes(&tileInfos[newBackTileIndex]);

		return &tileInfos[newFrontTileIndex];
	}

	void reconnectCornerNodes(TileInfo* frontTileInfo)
	{
		CenterNode* frontNode = static_cast<CenterNode*>(getNode(frontTileInfo->centerNodeIndex));

		// remove all corner nodes associated with this tile pair
		std::set<int> deleteList;
		for (LocalDirection d : tnav::ORTHOGONAL_DIRECTION_SET) {
			CenterNode* neighborCenterNode = getSecondNeighbor(frontNode, d);
			LocalDirection 
				D = tnav::map(getSecondNeighborMap(frontNode, d), tnav::inverse(d)),
				diagonal1 = tnav::combine(D, LocalDirection((D + 1) % 4)),
				diagonal2 = tnav::combine(D, LocalDirection((D + 3) % 4));

			deleteList.insert(neighborCenterNode->getNeighborIndex(diagonal1));
			deleteList.insert(neighborCenterNode->getNeighborIndex(diagonal2));
		}
		deleteList.erase(-1); // gets rid of all invalid indices.

		// removes all connections to the corner nodes associated with this corner pair:
		for (int i : deleteList) {
			CornerNode* c = static_cast<CornerNode*>(getNode(i));
			// remove centerNode -> this corner node connections:
			for (LocalDirection d : tnav::DIAGONAL_DIRECTION_SET) { // ALL corner nodes MUST have 4 connections definitially
				LocalDirection D = tnav::map(c->getNeighborMap(d), tnav::inverse(d));
				int index = c->getNeighborIndex(d);
				if (index == -1) continue;
				CenterNode* centerNode = static_cast<CenterNode*>(getNode(index));
				centerNode->setNeighborIndex(D, -1);
				centerNode->setNeighborMap(D, MAP_TYPE_ERROR);
			}
			removeNode(c->index);
		}

		// cycle through all 4 possible corner nodes and re-add them if they are valid
		for (LocalDirection d : tnav::ORTHOGONAL_DIRECTION_SET) {
			// could be that the deletion earlier resized the underlying node vector:
			frontNode = static_cast<CenterNode*>(getNode(frontTileInfo->centerNodeIndex));
			LocalDirection
				centralNorth = d,
				centralEast = LocalDirection((d + 1) % 4),
				centralSouth = LocalDirection((d + 2) % 4),
				centralWest = LocalDirection((d + 3) % 4),
				centralNorthEast = tnav::combine(centralNorth, centralEast),
				centralNorthWest = tnav::combine(centralNorth, centralWest),
				centralSouthEast = tnav::combine(centralSouth, centralEast),
				centralSouthWest = tnav::combine(centralSouth, centralWest);
			CenterNode
				* northNode = static_cast<CenterNode*>(getSecondNeighbor(frontNode, centralNorth)),
				* eastNode = static_cast<CenterNode*>(getSecondNeighbor(frontNode, centralEast));
			MapType
				goNorth = getSecondNeighborMap(frontNode, centralNorth),
				goEast = getSecondNeighborMap(frontNode, centralEast);
			LocalDirection
				toNorthEast = tnav::map(goNorth, centralEast),
				toEastNorth = tnav::map(goEast, centralNorth);
			CenterNode
				* northEastNode = static_cast<CenterNode*>(getSecondNeighbor(northNode, toNorthEast)),
				* eastNorthNode = static_cast<CenterNode*>(getSecondNeighbor(eastNode, toEastNorth));

			if (northEastNode != eastNorthNode || northEastNode == frontNode) 
				continue; // this corner is degenerate!

			MapType
				northToNorthEast = getSecondNeighborMap(northNode, toNorthEast),
				goNorthEast = tnav::combine(goNorth, northToNorthEast);

			glm::vec3 newCornerNodePosition = frontNode->position
				+ tnav::globalDirToVec3(tnav::localToGlobalDir(frontNode->orientation, centralNorth)) / 2.0f
				+ tnav::globalDirToVec3(tnav::localToGlobalDir(frontNode->orientation, centralEast)) / 2.0f;

			int northNodeIndex = northNode->index,
				eastNodeIndex = eastNode->index,
				northEastNodeIndex = northEastNode->index,
				newCornerNodeIndex = addCornerNode(newCornerNodePosition, frontNode->orientation);

			// addNode() may have resized the underlying vector that keeps nodes, making the pointers
			// error-prone.  just re-find here to avoid potential issues:
			northNode = static_cast<CenterNode*>(getNode(northNodeIndex));
			eastNode = static_cast<CenterNode*>(getNode(eastNodeIndex));
			northEastNode = static_cast<CenterNode*>(getNode(northEastNodeIndex));

			// stitch everything together finally:
			CornerNode* newCornerNode = static_cast<CornerNode*>(getNode(newCornerNodeIndex));
			newCornerNode->setNeighborIndex(centralSouthWest, frontNode->index);
			newCornerNode->setNeighborMap(centralSouthWest, MAP_TYPE_IDENTITY);
			newCornerNode->setNeighborIndex(centralNorthWest, northNode->index);
			newCornerNode->setNeighborMap(centralNorthWest, goNorth);
			newCornerNode->setNeighborIndex(centralSouthEast, eastNode->index);
			newCornerNode->setNeighborMap(centralSouthEast, goEast);
			newCornerNode->setNeighborIndex(centralNorthEast, northEastNode->index);
			newCornerNode->setNeighborMap(centralNorthEast, goNorthEast);

			frontNode->setNeighborIndex(centralNorthEast, newCornerNode->index);
			frontNode->setNeighborMap(centralNorthEast, MAP_TYPE_IDENTITY);

			LocalDirection toCornerNode = tnav::map(goNorth, centralSouthEast);
			northNode->setNeighborIndex(toCornerNode, newCornerNode->index);
			northNode->setNeighborMap(toCornerNode, tnav::inverse(newCornerNode->getNeighborMap(centralNorthWest)));

			toCornerNode = tnav::map(goEast, centralNorthWest);
			eastNode->setNeighborIndex(toCornerNode, newCornerNode->index);
			eastNode->setNeighborMap(toCornerNode, tnav::inverse(newCornerNode->getNeighborMap(centralSouthEast)));

			toCornerNode = tnav::map(goNorthEast, centralSouthWest);
			northEastNode->setNeighborIndex(toCornerNode, newCornerNode->index);
			northEastNode->setNeighborMap(toCornerNode, tnav::inverse(newCornerNode->getNeighborMap(centralNorthEast)));
		}
	}

	void removeTilePair(int tileInfoIndex) { removeTilePair(&tileInfos[tileInfoIndex]); }

	void removeTilePair(TileInfo* t)
	{
		using namespace tnav;

		if (t == nullptr) return;

		TileInfo* siblingTile = getTileInfo(t->siblingIndex);
		int centerNodeIndex = (t->centerNodeIndex);
		int sibCenterNodeIndex = (siblingTile->centerNodeIndex);

		// reconnect edges:
		for (LocalDirection d : tnav::ORTHOGONAL_DIRECTION_SET) {
			TileInfo
				* neighborTile1 = getTileInfo(t, d),
				* neighborTile2 = getTileInfo(t->siblingIndex, d);
			if (neighborTile1 == siblingTile) {
				// edges that dont connect to anything can be simply removed, 
				// as no reconnection is necessary:
				removeNode(getNode(centerNodeIndex)->getNeighborIndex(d));
				continue;
			}

			CenterNode* centerNode1 = static_cast<CenterNode*>(getNode(centerNodeIndex));
			SideNode* sideNode1 = static_cast<SideNode*>(getNode(centerNode1->getNeighborIndex(d)));
			CenterNode* neighborCenterNode1 = static_cast<CenterNode*>(getNode(neighborTile1->centerNodeIndex));
			CenterNode* centerNode2 = static_cast<CenterNode*>(getNode(sibCenterNodeIndex));
			SideNode* sideNode2 = static_cast<SideNode*>(getNode(centerNode2->getNeighborIndex(d)));
			CenterNode* neighborCenterNode2 = static_cast<CenterNode*>(getNode(neighborTile2->centerNodeIndex));
			SideNode* newNode = static_cast<SideNode*>(getNode(addNode(NODE_TYPE_SIDE)));

			LocalDirection 
				n1ToNew = mapToSecondNeighbor(centerNode1, d, inverse(d)),
				n2ToNew = mapToSecondNeighbor(centerNode2, d, inverse(d)),
				newToN1 = inverse(n1ToNew),
				newToN2 = n1ToNew;

			MapType 
				newToN2Map = getNeighborMap(newToN2, n2ToNew),
				n2ToNewMap = inverse(newToN2Map);

			// While we could edit the existing nodes, its conceptually simpler to just make a fresh one:
			newNode->setPosition(sideNode1->getPosition());
			newNode->oriType = neighborCenterNode1->oriType; // makes sure the transition from center node type -> new side node type is possible
			newNode->setSideNodeType(
				(n1ToNew == LOCAL_DIRECTION_0 || n1ToNew == LOCAL_DIRECTION_2)
				? SIDE_NODE_TYPE_HORIZONTAL
				: SIDE_NODE_TYPE_VERTICAL);

			neighborCenterNode1->setNeighborIndex(n1ToNew, newNode->getIndex());
			neighborCenterNode1->setNeighborMap(n1ToNew, MAP_TYPE_IDENTITY);
			newNode->setNeighborIndex(newToN2, neighborCenterNode2->getIndex());
			newNode->setNeighborMap(newToN2, newToN2Map);
			neighborCenterNode2->setNeighborIndex(n2ToNew, newNode->getIndex());
			neighborCenterNode2->setNeighborMap(n2ToNew, n2ToNewMap);
			newNode->setNeighborIndex(newToN1, neighborCenterNode1->getIndex());
			newNode->setNeighborMap(newToN1, MAP_TYPE_IDENTITY);

			removeNode(sideNode1->getIndex());
			removeNode(sideNode2->getIndex());
		}

		for (LocalDirection d : tnav::DIAGONAL_DIRECTION_SET) {
			CenterNode* c = static_cast<CenterNode*>(getNode(centerNodeIndex));
			if (c->getNeighborIndex(d) != -1) {
				LocalDirection D = tnav::map(c->getNeighborMap(d), tnav::inverse(d));
				CornerNode* C = static_cast<CornerNode*>(getNode(c->getNeighborIndex(d)));
				C->setNeighborIndex(D, -1);
				C->setNeighborMap(D, MAP_TYPE_ERROR);
			}

			c = static_cast<CenterNode*>(getNode(sibCenterNodeIndex));
			if (c->getNeighborIndex(d) != -1) {
				LocalDirection D = tnav::map(c->getNeighborMap(d), tnav::inverse(d));
				CornerNode* C = static_cast<CornerNode*>(getNode(c->getNeighborIndex(d)));
				C->setNeighborIndex(D, -1);
				C->setNeighborMap(D, MAP_TYPE_ERROR);
			}
		}
		
		removeTileInfo(static_cast<CenterNode*>(getNode(centerNodeIndex))->getTileInfoIndex());
		removeTileInfo(static_cast<CenterNode*>(getNode(sibCenterNodeIndex))->getTileInfoIndex());
		removeNode(centerNodeIndex);
		removeNode(sibCenterNodeIndex);

<<<<<<<< HEAD:PerspectiveGame/tileNodeNetwork.h
		for (int i : affectedCenterNodeIndices) {
			reconnectCornerNodes(getTileInfo(static_cast<CenterNode*>(getNode(i))->getTileInfoIndex()));
		}
========
		// TODO: add/remove corner nodes based on new geometry:
>>>>>>>> 91f7b3b7202d67259637fd123642c59c777ed463:PerspectiveGame/positionNodeNetwork.h
	}
};