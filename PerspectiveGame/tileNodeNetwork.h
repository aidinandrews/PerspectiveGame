#pragma once

#include <iostream>
#include <vector>
#include <set>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include "tileNavigation.h"
#include "tileNode.h"
#include "tile.h"
#include "cameraManager.h"

struct TileNodeNetwork {
private:
	std::vector<TileNode*> nodes;
	std::vector<int> freeNodeIndices;

	std::vector<Tile> tiles;
	std::vector<int> freeTileInfoIndices;

	Camera* p_camera;
	ForceManager* p_forceManager;

public: // Rendering:
	GLuint texID;
	GLuint positionNodeInfosBufferID;
	GLuint tilesBufferID;
	std::vector<glm::vec2> windowFrustum;

	std::vector<GPU_Tile> gpuTiles;
	std::vector<GPU_TileNodeInfo> gpuPositionNodeInfos;

	CenterNode CurrentNode;
	int currentMapIndex = 0;
	int currentNodeIndex = 4;

public:
	TileNodeNetwork(Camera* c, ForceManager* fm)
		: p_camera(c)
		, p_forceManager(fm)
	{
		for (int x = 0; x < 1; x++) {
			for (int y = 0; y < 1; y++) {
				createTilePair(glm::vec3(float(x), float(y), 0), TILE_TYPE_XY);
			}
		}

		// Rendering:
		glGenBuffers(1, &positionNodeInfosBufferID);
		glGenBuffers(1, &tilesBufferID);
	}

	void update()
	{
		gpuPositionNodeInfos.clear();
		for (TileNode* p : nodes) {
			if (p == nullptr) 
				gpuPositionNodeInfos.push_back(GPU_TileNodeInfo());
			else
				gpuPositionNodeInfos.push_back(GPU_TileNodeInfo(*p));
		}

		gpuTiles.clear();
		for (Tile& i : tiles) {
			gpuTiles.push_back(GPU_Tile(i));
		}

		
	}

	void checkCornerConnections()
	{
		/*for (auto n : nodes) {
			if (n == nullptr || n->type != NODE_TYPE_DEGENERATE) {
				continue;
			}
			DegenerateCornerNode* degen = static_cast<DegenerateCornerNode*>(n);
			for (auto i : degen->componentPairIndices) {
				if (getNodeViaForceComponentIndex(i)->type != NODE_TYPE_CENTER)
					std::cout << i << " has BAD CONNECTION!" << std::endl;

				if (degen->componentPairIndices.size() != degen->numDegenComponents) {
					std::cout << "INVALID PAIR COUNT" << std::endl;
				}
			}
		}*/
	}

	int size() { return (int)nodes.size(); }

	void printSize()
	{
		std::vector<int> degenConnections;
		std::vector<glm::vec3> degenPositions;
		int numCenterNodes = 0, numSideNodes = 0, numCornerNodes = 0, numDegenNodes = 0;
		for (TileNode* n : nodes) {
			if (n == nullptr) continue;
			switch (n->type) {
			case NODE_TYPE_CENTER: numCenterNodes++; break;
			case NODE_TYPE_SIDE: numSideNodes++; break;
         case NODE_TYPE_CORNER: numCornerNodes++; break;
         case NODE_TYPE_DEGENERATE: numDegenNodes++;
				DegenerateCornerNode* d = static_cast<DegenerateCornerNode*>(n);
            degenConnections.push_back(d->numConnectedTiles());
				degenPositions.push_back(d->position);
            break;
			}
		}
		std::cout
			<< "\nnum center nodes: " << numCenterNodes
			<< "\nnum side nodes: " << numSideNodes
			<< "\nnum corner nodes: " << numCornerNodes
			<< "\nnum degenerate nodes: " << numDegenNodes
			<< "\ntotal num nodes: " << numCenterNodes + numSideNodes + numCornerNodes + numDegenNodes
			<< std::endl;
		for (int i = 0; i < degenConnections.size(); i++) {
			vechelp::print(degenPositions[i]);
			std::cout <<", " << degenConnections[i] << "\n";
		}
		std::cout<<std::endl;
	}

	void printCornerNodePositions()
	{
		for (TileNode* n : nodes) {
			if (n != nullptr && n->type == NODE_TYPE_CORNER)
				vechelp::println(n->position);
		}
	}

	TileNode* getNode(int index)
	{
		return nodes[index];
	}

	CenterNode* getNode(Tile* info)
	{
		return static_cast<CenterNode*>(nodes[info->centerNodeIndex]);
	}

	Tile* getTile(int index)
	{
		return &tiles[index];
	}

	int numTileInfos() { return (int)tiles.size(); }

	Tile* getTile(int tileInfoIndex, LocalDirection d)
	{
		return getTile(tiles[tileInfoIndex], d);
	}

	// Gives the tile info of the neighbor tile in the direction of d.
	Tile* getTile(Tile& info, LocalDirection d)
	{
		CenterNode* node = static_cast<CenterNode*>(nodes[info.centerNodeIndex]);
		LocalDirection d2 = tnav::map(node->getNeighborMap(d), d);
		SideNode* sideNode = static_cast<SideNode*>(nodes[node->getNeighborIndex(d)]);
		CenterNode* neighborCenterNode = static_cast<CenterNode*>(nodes[sideNode->getNeighborIndex(d2)]);
		return &tiles[neighborCenterNode->getTileIndex()];
	}

	TileNode* getNeighbor(TileNode& node, LocalDirection toNeighbor)
	{
		return nodes[node.getNeighborIndex(toNeighbor)];
	}

	CenterNode* getSecondNeighbor(CenterNode& node, LocalDirection toNeighbor)
	{
		SideNode* sideNode = static_cast<SideNode*>(nodes[node.getNeighborIndex(toNeighbor)]);
		LocalDirection d = tnav::map(node.getNeighborMap(toNeighbor), toNeighbor);
		return static_cast<CenterNode*>(nodes[sideNode->getNeighborIndex(d)]);
	}

	MapType getSecondNeighborMap(CenterNode& node, LocalDirection toNeighbor)
	{
		SideNode* sideNode = static_cast<SideNode*>(nodes[node.getNeighborIndex(toNeighbor)]);
		LocalDirection d = tnav::map(node.getNeighborMap(toNeighbor), toNeighbor);
		return tnav::combine(node.getNeighborMap(toNeighbor), sideNode->getNeighborMap(d));
	}

	LocalDirection mapToSecondNeighbor(CenterNode& node, LocalDirection toNeighbor, LocalAlignment alignment)
	{
		SideNode* sideNode = static_cast<SideNode*>(nodes[node.getNeighborIndex(toNeighbor)]);
		alignment = tnav::map(node.getNeighborMap(toNeighbor), alignment);
		
		toNeighbor = tnav::map(node.getNeighborMap(toNeighbor), toNeighbor);
		return tnav::map(sideNode->getNeighborMap(toNeighbor), alignment);;
	}
	 
	// Will add a node to the nodes list that is unconnected and error prone if it is not connected up!
	// returns an index to the added node.
	int addNode(TileNodeType type, glm::vec3 pos, OrientationType orientation)
	{
		TileNode* node = nullptr;
		switch (type) {
		case NODE_TYPE_CENTER:
			node = new CenterNode();
			break;
		case NODE_TYPE_SIDE:
			node = (new SideNode(SIDE_NODE_TYPE_ERROR));
			break;
		case NODE_TYPE_CORNER:
			node = new CornerNode();
			break;
		case NODE_TYPE_DEGENERATE:
			node = new DegenerateCornerNode();
			break;
		default:
			return -1;
		}
		node->position = pos;
		node->orientation = orientation;

		if (freeNodeIndices.size() > 0) {
			nodes[freeNodeIndices.back()] = node;
			node->setIndex(freeNodeIndices.back()); // all freeNode nodes are wiped
			node->forceListIndex = node->index * 4;
			freeNodeIndices.pop_back();
		}
		else {
			nodes.push_back(node);
			nodes.back()->setIndex((int)nodes.size() - 1);
			node->forceListIndex = p_forceManager->addForce(LOCAL_DIRECTION_STATIC, node->index);
		}

		return node->getIndex();
	}

	// center nodes inherantly require more information, hence the inputs:
	int addCenterNode(glm::vec3 pos, TileType orientation)
	{
		return addNode(NODE_TYPE_CENTER, pos, orientation);
	}

	int addSideNode(glm::vec3 pos, OrientationType orientation)
	{
		return addNode(NODE_TYPE_SIDE, pos, orientation);
	}

	int addCornerNode(glm::vec3 pos, OrientationType orientation)
	{
		return addNode(NODE_TYPE_CORNER, pos, orientation);
	}

	int addDegenerateNode(DegenerateCornerNode& degen)
	{
		int newDegeni = addNode(NODE_TYPE_DEGENERATE, degen.position, ORIENTATION_TYPE_ERROR);
		DegenerateCornerNode* newDegen = static_cast<DegenerateCornerNode*>(getNode(newDegeni));
		newDegen->resizeComponentList(degen.numDegenComponents);
		for (int i = 0; i < degen.numDegenComponents; i++)
			newDegen->componentPairIndices[i] = degen.componentPairIndices[i];

		return newDegeni;
	}

	int addDegenNode(glm::vec3 pos, int forceComponentIndexA1, int forceComponentIndexA2, int forceComponentIndexB1, int forceComponentIndexB2)
	{
		if (forceComponentIndexA1 == -1 || forceComponentIndexA2 == -1 || forceComponentIndexB1 == -1 || forceComponentIndexB2 == -1) {
			return -1;
		}
		if (abs(forceComponentIndexA1 - forceComponentIndexA2) > 3 || abs(forceComponentIndexB1 - forceComponentIndexB2) > 3) {
			return -1;
		}

		int i = addNode(NODE_TYPE_DEGENERATE, pos, ORIENTATION_TYPE_ERROR);
		DegenerateCornerNode* degenNode = static_cast<DegenerateCornerNode*>(getNode(i));
		degenNode->componentPairIndices[0] = forceComponentIndexA1;
		degenNode->componentPairIndices[1] = forceComponentIndexA2;
		degenNode->componentPairIndices[2] = forceComponentIndexB1;
		degenNode->componentPairIndices[3] = forceComponentIndexB2;
		return i;
	}

	int addDegenNode(CornerNode& cornerNode)
	{
		DegenerateCornerNode* degenNode = new DegenerateCornerNode();
		degenNode->index = cornerNode.index;
		degenNode->position = cornerNode.position;
		degenNode->forceListIndex = cornerNode.forceListIndex;
		degenNode->resizeComponentList(8);

		for (LocalDirection d : tnav::DIAGONAL_DIRECTION_SET) {
			CenterNode* neighbor = static_cast<CenterNode*>(getNode(cornerNode.getNeighborIndex(d)));
			int forceListIndex = neighbor->forceListIndex;
			
			LocalDirection toCorner = d;
			toCorner = tnav::map(cornerNode.getNeighborMap(toCorner), tnav::inverse(toCorner));
			neighbor->setNeighborMap(toCorner, MAP_TYPE_ERROR);
			
			auto components = tnav::getAlignmentComponents(toCorner);
			degenNode->componentPairIndices[2*(d - 4) + 0] = forceListIndex + (int)components[0];
			degenNode->componentPairIndices[2*(d - 4) + 1] = forceListIndex + (int)components[1];
		}

		p_forceManager->setForce(degenNode->forceListIndex, LOCAL_DIRECTION_STATIC);

		delete nodes[cornerNode.index];
		nodes[degenNode->index] = degenNode;
		return degenNode->index;
	}

	void removeNode(int index)
	{
		p_forceManager->removeForce(index * 4);

		if (index == nodes.size() - 1) {
			delete nodes[index];
			nodes.pop_back();
		}
		else {
			freeNodeIndices.push_back(index);
			delete nodes[index];
			nodes[index] = nullptr;
		}
	}

	void colorTile(int index)
	{
		switch (tiles[index].type) {
		case TILE_TYPE_XYF: tiles[index].color = glm::vec3(1.0f, 0.0f, 0.0f); break;
		case TILE_TYPE_XYB: tiles[index].color = glm::vec3(0.5f, 0.0f, 0.0f); break;
		case TILE_TYPE_XZF: tiles[index].color = glm::vec3(0.0f, 1.0f, 0.0f); break;
		case TILE_TYPE_XZB: tiles[index].color = glm::vec3(0.0f, 0.5f, 0.0f); break;
		case TILE_TYPE_YZF: tiles[index].color = glm::vec3(0.0f, 0.0f, 1.0f); break;
		case TILE_TYPE_YZB: tiles[index].color = glm::vec3(0.0f, 0.0f, 0.5f); break;
		}
	}

	void removeTile(int index)
	{
		tiles[index].wipe();
		freeTileInfoIndices.push_back(index);
	}

	// Adds an UNINITIALIZED tile info to the tileInfos list.  Returns a pointer to the added tileInfo.
	// frontIndex and backIndex are meant to be used as returns.
	void addTilePair(int frontCenterNodeIndex, int backCenterNodeIndex, SuperTileType type, int& frontInfoIndex, int& backInfoIndex)
	{
		if (freeTileInfoIndices.size() > 1) {
			frontInfoIndex = freeTileInfoIndices.back();
			tiles[frontInfoIndex].index = freeTileInfoIndices.back();
			freeTileInfoIndices.pop_back();

			backInfoIndex = freeTileInfoIndices.back();
			tiles[backInfoIndex].index = freeTileInfoIndices.back();
			freeTileInfoIndices.pop_back();
		}
		else if (freeTileInfoIndices.size() == 1) {
			frontInfoIndex = freeTileInfoIndices.back();
			tiles[frontInfoIndex].index = freeTileInfoIndices.back();
			freeTileInfoIndices.pop_back();

			tiles.push_back(Tile());
			backInfoIndex = (int)tiles.size() - 1;
			tiles[backInfoIndex].index = (int)tiles.size() - 1;
		}
		else {
			tiles.push_back(Tile());
			frontInfoIndex = (int)tiles.size() - 1;
			tiles[frontInfoIndex].index = (int)tiles.size() - 1;

			tiles.push_back(Tile());
			backInfoIndex = (int)tiles.size() - 1;
			tiles[backInfoIndex].index = (int)tiles.size() - 1;
		}

		tiles[frontInfoIndex].centerNodeIndex = frontCenterNodeIndex;
		tiles[frontInfoIndex].type = tnav::getFrontTileType(type);
		tiles[frontInfoIndex].siblingIndex = backInfoIndex;

		tiles[backInfoIndex].centerNodeIndex = backCenterNodeIndex;
		tiles[backInfoIndex].type = tnav::getBackTileType(type);
		tiles[backInfoIndex].siblingIndex = frontInfoIndex;

		colorTile(frontInfoIndex);
		colorTile(backInfoIndex);
	}

	// given a position in space, returns all the tiles connected to that point in the network.
	// currently assumes the given position is of a side node!  will error if given a position 
	// relating to a center of corner node.
	std::vector<Tile> getConnectedTiles(SideNode* node)
	{
		std::vector<Tile> connectedTiles;
		int sideNodeIndex = node->index;
		glm::vec3 pos = nodes[sideNodeIndex]->getPosition();

		for (TileNode* n : nodes) {
			if (n == nullptr || n->getPosition() != pos || n->getIndex() == sideNodeIndex)
				continue;

			SideNode* s = static_cast<SideNode*>(n);
			for (int i = 0; i < 2; i++) { // side nodes only have 2 neighbors
				if (s->getNeighborIndexDirect(i) == -1)
					continue;

				connectedTiles.push_back(
					tiles[
						static_cast<CenterNode*>(nodes[s->getNeighborIndexDirect(i)])->getTileIndex()
					]
				);
			}
		}

		return connectedTiles;
	}

	// returns how prioritized the connection between these two tiles should be.
	// 0 == high prio, 1 == medium, 2 = low, 3 = should not be connected in the first place!
	int getConnectionPrio(Tile* a, Tile* b)
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
	Tile* createTilePair(glm::vec3 pos, SuperTileType type)
	{
		// check if there is already a tile where we are trying to add one:
		// increment by 2s since we always add/remove 2 tiles at a time (front and back).
		for (int i = 0; i < tiles.size(); i += 2) {
			if (tiles[i].index == -1) continue;
			if (nodes[tiles[i].centerNodeIndex]->getPosition() == pos)
				return nullptr;
		}

		// create the new tile pair and center nodes:
		// indices are used because apparently pointers are unsafe if the vector resizes itself.
		TileType frontType = tnav::getTileType(type, true);
		TileType backType = tnav::getTileType(type, false);
		int newFrontNodeIndex = addCenterNode(pos, frontType);
		int newBackNodeIndex = addCenterNode(pos, backType);
		int newFrontTileIndex, newBackTileIndex;
		addTilePair(newFrontNodeIndex, newBackNodeIndex, type, newFrontTileIndex, newBackTileIndex);
		static_cast<CenterNode*>(nodes[newFrontNodeIndex])->setTileInfoIndex(newFrontTileIndex);
		static_cast<CenterNode*>(nodes[newBackNodeIndex])->setTileInfoIndex(newBackTileIndex);

		connectSideNodes(tiles[newFrontTileIndex]);
		connectOrCreateCornerNodes(tiles[newFrontTileIndex]);

		reconnectTile(tiles[newFrontTileIndex]);
		reconnectTile(tiles[newBackTileIndex]);

		checkCornerConnections();

		return &tiles[newFrontTileIndex];
	}

	// Given a tile pair, will connect OR reconnect all the side nodes of that tile to the world.
	void connectSideNodes(Tile& frontTile)
	{
		Tile* backTile = getTile(frontTile.siblingIndex);
		CenterNode* frontCenterNode = static_cast<CenterNode*>(getNode(frontTile.centerNodeIndex));
		CenterNode* backCenterNode = static_cast<CenterNode*>(getNode(backTile->centerNodeIndex));
		SideNode* newSideNode;

		for (LocalDirection d : tnav::ORTHOGONAL_DIRECTION_SET) {
			// add a new side node is none exists:
			if (frontCenterNode->getNeighborIndex(d) == -1) {
				const glm::vec3* toSidesOffsets = tnav::getNodePositionOffsets(tnav::getSuperTileType(frontTile.type));
				glm::vec3 sidePos = frontCenterNode->getPosition() + toSidesOffsets[d];
				newSideNode = static_cast<SideNode*>(nodes[addSideNode(sidePos, ORIENTATION_TYPE_ERROR)]);
				newSideNode->setPosition(sidePos);
			}
			else { // if one exists, just use it:
				newSideNode = static_cast<SideNode*>(getNode(frontCenterNode->getNeighborIndex(d)));
			}
			newSideNode->orientation = frontCenterNode->orientation; // Arbitrary but convenient later.
			newSideNode->setSideNodeType(
				(d == LOCAL_DIRECTION_0 || d == LOCAL_DIRECTION_2)
				? SIDE_NODE_TYPE_HORIZONTAL
				: SIDE_NODE_TYPE_VERTICAL);

			std::vector<Tile> linkedTiles = getConnectedTiles(newSideNode);
			if (linkedTiles.size() == 0) {
				connectTilePairSidesToThemselves(frontTile, *newSideNode, d);
				continue;
			}

			// reconnect tiles based on heirarchy:
			for (Tile linkedTile : linkedTiles) {
				LocalDirection linkedTileDir;
				CenterNode* linkedTileCenterNode = static_cast<CenterNode*>(getNode(linkedTile.centerNodeIndex));

				for (LocalDirection dir : tnav::ORTHOGONAL_DIRECTION_SET) {
					if (nodes[linkedTileCenterNode->getNeighborIndex(dir)]->getPosition() == newSideNode->getPosition()) {
						linkedTileDir = dir;
						break;
					}
				}
				Tile* currentNeighbor = getTile(linkedTile, linkedTileDir);
				CenterNode* currentNeighborCenterNode = getNode(currentNeighbor);

				int currentConnectionPrio = getConnectionPrio(&linkedTile, currentNeighbor);
				int newConnectionPrio1 = getConnectionPrio(&linkedTile, getTile(frontTile.index));
				int newConnectionPrio2 = getConnectionPrio(&linkedTile, getTile(backTile->index));
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
					linkedSideNode->orientation = backCenterNode->orientation;
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
			for (Tile& t : linkedTiles) {
				reconnectTile(tiles[t.index]);
			}
		}
	}

	// merges node2 into node1.
	DegenerateCornerNode* merge(DegenerateCornerNode* node1, DegenerateCornerNode* node2)
	{
		if (node1 == nullptr && node2 == nullptr) return nullptr;
		if (node1 == nullptr) return node2;
		if (node2 == nullptr) return node1;
		if (node1 == node2) return node1;

		for (int i : node2->componentPairIndices) node1->componentPairIndices.push_back(i);
		node1->numDegenComponents += node2->numDegenComponents;

		for (int i = 0; i < node2->numConnectedTiles(); i++) {
			LocalDirection d = node2->neighborToThisNode(i);
			CenterNode* c = getNodeViaForceComponentIndex(node2->componentPairIndices[i * 2]);
			c->setNeighborIndex(d, node1->index);
			// map is already ERROR as node2 must be degen as well.
		}

		removeNode(node2->index);

		return node1;
	}


	void connectTilePairSidesToThemselves(Tile& t, SideNode& newSideNode, LocalDirection toSibling)
	{
		CenterNode* centerNode = static_cast<CenterNode*>(getNode(t.centerNodeIndex));
		CenterNode* siblingNode = static_cast<CenterNode*>(getNode(getTile(t.siblingIndex)->centerNodeIndex));

		centerNode->setNeighborIndex(toSibling, newSideNode.getIndex());
		centerNode->setNeighborMap(toSibling, MAP_TYPE_IDENTITY);

		siblingNode->setNeighborIndex(toSibling, newSideNode.getIndex());
		siblingNode->setNeighborMap(toSibling, tnav::getNeighborMap(toSibling, toSibling));

		newSideNode.setNeighborIndex(tnav::inverse(toSibling), centerNode->getIndex());
		newSideNode.setNeighborMap(tnav::inverse(toSibling), MAP_TYPE_IDENTITY);
		newSideNode.setNeighborIndex(toSibling, siblingNode->getIndex());
		newSideNode.setNeighborMap(toSibling, tnav::getNeighborMap(toSibling, toSibling));
	}

	// This function assumes that there is a tile connected in one of or both of the components of toCorner!
	DegenerateCornerNode* connectCornerToDegenNode(Tile& tile, LocalDirection toCorner)
	{
		const LocalDirection* components = tnav::getAlignmentComponents(toCorner);
		LocalDirection component1 = components[0], component2 = components[1];
		CenterNode* centerNode = static_cast<CenterNode*>(getNode(tile.centerNodeIndex));
		Tile* siblingTile = getTile(tile.siblingIndex);
		CenterNode* siblingNode = static_cast<CenterNode*>(getNode(siblingTile->centerNodeIndex));

		auto findCorner = [this](CenterNode& n, LocalDirection c1, LocalDirection c2, MapType& centerToCornerMap) {
			auto neighbor = getSecondNeighbor(n, c1);
			auto toNeighborMap = getSecondNeighborMap(n, c1);
			auto neighborToCorner = tnav::map(toNeighborMap, tnav::combine(tnav::inverse(c1), c2));

			if (neighbor->getNeighborIndex(neighborToCorner) == -1) {
				centerToCornerMap = MAP_TYPE_IDENTITY; // cause it will be merged later with the other one.
				return static_cast<DegenerateCornerNode*>(nullptr);
			}
			else {
				auto corner = getNode(neighbor->getNeighborIndex(neighborToCorner));
				centerToCornerMap = tnav::combine(toNeighborMap, neighbor->getNeighborMap(neighborToCorner));
				if (corner->type == NODE_TYPE_CORNER)
					return static_cast<DegenerateCornerNode*>(getNode(addDegenNode(*static_cast<CornerNode*>(corner))));
				else return static_cast<DegenerateCornerNode*>(corner);
			}
			};

		MapType maps[4];
		// gather all the corner nodes this new tile connects to:
		std::set<DegenerateCornerNode*> degenNodes;
		degenNodes.insert(findCorner(*centerNode, component1, component2, maps[0]));
		degenNodes.insert(findCorner(*centerNode, component2, component1, maps[1]));
		degenNodes.insert(findCorner(*siblingNode, component1, component2, maps[2]));
		degenNodes.insert(findCorner(*siblingNode, component2, component1, maps[3]));
		
		// merge all of them into one degenerate node:
		DegenerateCornerNode* degen = nullptr;
		for (auto n : degenNodes) degen = merge(degen, n);

		// connect the center node/sibling node to this new merged degen node:
		centerNode->setNeighborIndex(toCorner, degen->index);
		MapType m = (maps[0] == maps[1]) ? maps[0] : tnav::combine(maps[0], maps[1]);
		centerNode->setNeighborMap(toCorner, m);
		
		siblingNode->setNeighborIndex(toCorner, degen->index);
		m = (maps[2] == maps[3]) ? maps[2] : tnav::combine(maps[2], maps[3]);
		siblingNode->setNeighborMap(toCorner, m);

		degen->addDegenPair(centerNode->forceListIndex + (int)component1,
								  centerNode->forceListIndex + (int)component2);
		degen->addDegenPair(siblingNode->forceListIndex + (int)component1,
								  siblingNode->forceListIndex + (int)component2);

		return degen;
	}

	// adds any degen nodes to corners of a tile pair that are connected to nothing but the tile pair.
	void connectOrCreateCornerNodes(Tile& tile)
	{
		CenterNode* centerNode = static_cast<CenterNode*>(getNode(tile.centerNodeIndex));
		Tile* siblingTile = getTile(tile.siblingIndex);
		CenterNode* siblingNode = static_cast<CenterNode*>(getNode(siblingTile->centerNodeIndex));

		for (auto component1 : tnav::ORTHOGONAL_DIRECTION_SET) {
			LocalDirection component2 = LocalDirection((component1 + 1) % 4);
			LocalDirection toCorner = tnav::combine(component1, component2);
			
			if (getSecondNeighbor(*centerNode, component1) != siblingNode ||
				 getSecondNeighbor(*centerNode, component2) != siblingNode) {
				connectCornerToDegenNode(tile, toCorner);
			}
			else {
				glm::vec3 degenPos = centerNode->position + tnav::getCenterToNeighborVec(tile.type, toCorner) / 2.0f;
				int i = addDegenNode(degenPos,
											centerNode->forceListIndex  + (int)component1,
											centerNode->forceListIndex  + (int)component2,
											siblingNode->forceListIndex + (int)component1,
											siblingNode->forceListIndex + (int)component2);
				centerNode->setNeighborIndex(toCorner, i);
				siblingNode->setNeighborIndex(toCorner, i);
			}
		}

		for (auto d : tnav::DIAGONAL_DIRECTION_SET) {
			DegenerateCornerNode* degen = static_cast<DegenerateCornerNode*>(getNode(centerNode->getNeighborIndex(d)));
			if (degen->numConnectedTiles() >= 4) 
				tryAddCornerNodes(*degen);
		}
	}

	void reconnectTile(Tile& tile)
	{
		CenterNode* centerNode = static_cast<CenterNode*>(nodes[tile.centerNodeIndex]);
		for (LocalDirection d : tnav::ORTHOGONAL_DIRECTION_SET) {
			MapType m = getSecondNeighborMap(*centerNode, d);
			CenterNode* neighborCenterNode = getSecondNeighbor(*centerNode, d);
			tile.setNeighborMap(d, m);
			tile.setNeighborIndex(d, neighborCenterNode->getTileIndex());
		}
	}

	CenterNode* getNodeViaForceComponentIndex(int index)
	{
		CenterNode* c = static_cast<CenterNode*>(nodes[p_forceManager->getNodeIndex(index)]);
		return c;
	}

	int replaceWithCornerNode(DegenerateCornerNode& degen, OrientationType orientation)
	{
		CornerNode* corner = new CornerNode;
		corner->index = degen.index;
		corner->position = degen.position;
		corner->orientation = orientation;
		corner->forceListIndex = degen.forceListIndex;

		p_forceManager->setForce(corner->forceListIndex, LOCAL_DIRECTION_STATIC);
		
		delete nodes[degen.index];
		nodes[corner->index] = corner;

		return corner->index;
	}

	// given a degen
	std::vector<int> tryAddCornerNodes(DegenerateCornerNode& degenNode)
	{
		std::vector<int> cornerIndices;

		if (degenNode.numConnectedTiles() < 4) return cornerIndices;

		// gather all the center nodes we need to connect:
		std::set<CenterNode*> centerNodes;
		std::vector<LocalDirection> toCorners;
		for (int i = 0; i < degenNode.numConnectedTiles(); i++) // / 2 as there are two components/connected neighbor tile
			centerNodes.insert(getNodeViaForceComponentIndex(degenNode.componentPairIndices[i * 2]));

		// sort them into sets, each set connecting to one of the two new center nodes:
		std::vector<CenterNode*> sortedNodes;
		std::vector<MapType> cornerToNeighborMaps;
		int cornersToAdd = sortConnectedNeighbors(degenNode, centerNodes, toCorners, cornerToNeighborMaps, sortedNodes);

		if (cornersToAdd == 0) return cornerIndices;

		DegenerateCornerNode degenCopy = degenNode;

		for (int seti = 0; seti < cornersToAdd; seti++) {
			if (seti == 0)
				cornerIndices.push_back(replaceWithCornerNode(degenNode, sortedNodes[seti * 4]->orientation));
			else
				cornerIndices.push_back(addCornerNode(degenCopy.position, sortedNodes[seti * 4]->orientation));

			CornerNode* cornerNode = static_cast<CornerNode*>(getNode(cornerIndices.back()));

			for (int subseti = 0; subseti < 4; subseti++) {
				CenterNode* centerNode = sortedNodes[seti * 4 + subseti];
				LocalDirection toCorner = toCorners[seti * 4 + subseti];
				MapType cornerToCenterMap = cornerToNeighborMaps[seti * 4 + subseti];
				
				degenCopy.removeConnection(centerNode->forceListIndex);

				centerNode->setNeighborIndex(toCorner, cornerNode->index);
				centerNode->setNeighborMap(toCorner, tnav::inverse(cornerToCenterMap));

				LocalDirection cornerToCenter = tnav::map(tnav::inverse(cornerToCenterMap), tnav::inverse(toCorner));
				cornerNode->setNeighborIndex(cornerToCenter, centerNode->index);
				cornerNode->setNeighborMap(cornerToCenter, cornerToCenterMap);
			}
		}

		// It may be that there is some left over connections still needing to be connected to a degenerate node
		if (degenCopy.numConnectedTiles() > 0) {
			DegenerateCornerNode* newDegen = static_cast<DegenerateCornerNode*>(getNode(addDegenerateNode(degenCopy)));
			// connect up the center nodes that are still connected to a degen node:
			for (int i = 0; i < newDegen->numConnectedTiles(); i++) {
				LocalDirection d = newDegen->neighborToThisNode(i);
				CenterNode* c = getNodeViaForceComponentIndex(newDegen->componentPairIndices[i * 2]);
				c->setNeighborIndex(d, newDegen->index);
			}
		}

		return cornerIndices;
	}

	// given an unsorted vector of neighbor nodes to a degenerate node, sorts the vector into sets of 4 neighbors
	// that all surround what should become a center node.  returns the number of sets center nodes that should be created.
	// i.e. if there are 8 neighbors but only 4 should surround a corner node, 1 will be returned.
	int sortConnectedNeighbors(DegenerateCornerNode& degenNode,
								  std::set<CenterNode*> centerNodes,
								  std::vector<LocalDirection>& toCorners,
								  std::vector<MapType>& maps,
								  std::vector<CenterNode*>& sortedNodes)
	{
		std::vector<CenterNode*> nodesToCheck, checkedNodes, degenNodes;
		std::vector<MapType> mapsToCheck, checkedMaps;


		while (centerNodes.size() > 0) {
			auto it = centerNodes.begin(); // find the subset of nodes connected to this node
			nodesToCheck.push_back(*it);
			centerNodes.erase(*it);
			mapsToCheck.push_back(MAP_TYPE_IDENTITY); // potential new corner node assumed to have orientation matching front centerNode element.

			while (nodesToCheck.size() > 0) {
				// look around the node to find neighbors also in the list, and add them to the set:
				for (auto d : tnav::ORTHOGONAL_DIRECTION_SET) {
					CenterNode* neighbor = getSecondNeighbor(*nodesToCheck.front(), d);

					// it is possible for there to be more than one direction leading to a node in the set
					// if that node is siblings with the front of nodesToCheck.  Checking that the direction
					// could lead to the degenNode's position makes sure that the direction is not that duplicate!
					auto diag1 = tnav::combine(d, LocalDirection((d + 1) % 4));
					auto diag2 = tnav::combine(d, LocalDirection((d + 3) % 4));
					glm::vec3 pos1 = getNode(nodesToCheck.front()->getNeighborIndex(diag1))->position;
					glm::vec3 pos2 = getNode(nodesToCheck.front()->getNeighborIndex(diag2))->position;
					
					if ((pos1 == degenNode.position || pos2 == degenNode.position) && 
						 centerNodes.find(neighbor) != centerNodes.end()) {
						nodesToCheck.push_back(neighbor);

						MapType toNeighbor = getSecondNeighborMap(*nodesToCheck.front(), d);
						mapsToCheck.push_back(tnav::combine(mapsToCheck.front(), toNeighbor));

						centerNodes.erase(neighbor);
					}
				}
				// All the possible neighbors have been checked for the initial node,
				// but we may now need to check the connections to its neighbors, if they were added to nodesToCheck.
				checkedNodes.push_back(nodesToCheck.front());
				checkedMaps.push_back(mapsToCheck.front());
				
				nodesToCheck.erase(nodesToCheck.begin());
				mapsToCheck.erase(mapsToCheck.begin());
			}

			if (checkedNodes.size() == 4) {
				sortedNodes.insert(sortedNodes.end(), checkedNodes.begin(), checkedNodes.end());
				maps.insert(maps.end(), checkedMaps.begin(), checkedMaps.end());
			}
			else degenNodes.insert(degenNodes.end(), checkedNodes.begin(), checkedNodes.end());

			checkedNodes.clear();
			checkedMaps.clear();
		}

		centerNodes.clear();
		int numCorners = sortedNodes.size() / 4;
		sortedNodes.insert(sortedNodes.end(), degenNodes.begin(), degenNodes.end());

		
		// find the direction to the degenerate node/corner node(s) from each center node:
		for (auto n : sortedNodes) 
			for (auto d : tnav::DIAGONAL_DIRECTION_SET)
				if (getNode(n->getNeighborIndex(d)) == &degenNode) {
					toCorners.push_back(d); break;
				}

		return numCorners;
	}

	void removeTilePair(int tileInfoIndex) { removeTilePair(&tiles[tileInfoIndex]); }

	void removeTilePair(Tile* t)
	{
		using namespace tnav;

		if (t == nullptr) return;

		Tile* siblingTile = getTile(t->siblingIndex);
		int centerNodeIndex = (t->centerNodeIndex);
		int sibCenterNodeIndex = (siblingTile->centerNodeIndex);

		std::set<int> affectedTileIndices; // for managing degenerate corners later

		// reconnect edges:
		for (LocalDirection d : tnav::ORTHOGONAL_DIRECTION_SET) {
			Tile
				* neighborTile1 = getTile(*t, d),
				* neighborTile2 = getTile(t->siblingIndex, d);
			if (neighborTile1 == siblingTile) {
				// edges that dont connect to anything can be simply removed, 
				// as no reconnection is necessary:
				removeNode(getNode(centerNodeIndex)->getNeighborIndex(d));
				continue;
			}

			affectedTileIndices.insert(neighborTile1->index);
			affectedTileIndices.insert(neighborTile2->index);

			CenterNode* centerNode1 = static_cast<CenterNode*>(getNode(centerNodeIndex));
			SideNode* sideNode1 = static_cast<SideNode*>(getNode(centerNode1->getNeighborIndex(d)));
			CenterNode* neighborCenterNode1 = static_cast<CenterNode*>(getNode(neighborTile1->centerNodeIndex));
			CenterNode* centerNode2 = static_cast<CenterNode*>(getNode(sibCenterNodeIndex));
			SideNode* sideNode2 = static_cast<SideNode*>(getNode(centerNode2->getNeighborIndex(d)));
			CenterNode* neighborCenterNode2 = static_cast<CenterNode*>(getNode(neighborTile2->centerNodeIndex));
			
			LocalDirection 
				n1ToNew = mapToSecondNeighbor(*centerNode1, d, inverse(d)),
				n2ToNew = mapToSecondNeighbor(*centerNode2, d, inverse(d)),
				newToN1 = inverse(n1ToNew),
				newToN2 = n1ToNew;
			
			MapType 
				newToN2Map = getNeighborMap(newToN2, n2ToNew),
				n2ToNewMap = inverse(newToN2Map);
			
			// While we could edit the existing nodes, its conceptually simpler to just make a fresh one:
			SideNode* newNode = static_cast<SideNode*>(getNode(
				addSideNode(sideNode1->getPosition(), 
							neighborCenterNode1->orientation))); // makes sure the transition from center node type -> new side node type is possible
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

		nodes;

		// remove/reconnect corner nodes:
		for (auto d : tnav::DIAGONAL_DIRECTION_SET) {
			TileNode* neighbor1 = getNode(getNode(centerNodeIndex)->getNeighborIndex(d));
			TileNode* neighbor2 = getNode(getNode(sibCenterNodeIndex)->getNeighborIndex(d));
			bool neighborsSame = neighbor1 == neighbor2;
			DegenerateCornerNode* cornerNode1, * cornerNode2 = nullptr;
			
			if (neighbor1->type == NODE_TYPE_CORNER) {
				int i = addDegenNode(*static_cast<CornerNode*>(neighbor1));
				cornerNode1 = static_cast<DegenerateCornerNode*>(getNode(i));
			}
			else cornerNode1 = static_cast<DegenerateCornerNode*>(neighbor1);

			if (!neighborsSame) {
				if (neighbor2->type == NODE_TYPE_CORNER) {
					int i = addDegenNode(*static_cast<CornerNode*>(neighbor2));
					cornerNode2 = static_cast<DegenerateCornerNode*>(getNode(i));
				}
				else cornerNode2 = static_cast<DegenerateCornerNode*>(neighbor2);
			}
			
			// pair can be merged into one degen node then split into center node(s) and/or degen node(s) after:
			DegenerateCornerNode* degen = merge(cornerNode1, cornerNode2);
			degen->removeConnection(getNode(centerNodeIndex)->forceListIndex);
			degen->removeConnection(getNode(sibCenterNodeIndex)->forceListIndex);

			if (degen->numConnectedTiles() == 0) 
				removeNode(degen->index);
			else if (degen->numConnectedTiles() >= 4) 
				tryAddCornerNodes(*degen);
		}
		
		removeTile(static_cast<CenterNode*>(getNode(centerNodeIndex))->getTileIndex());
		removeTile(static_cast<CenterNode*>(getNode(sibCenterNodeIndex))->getTileIndex());
		removeNode(centerNodeIndex);
		removeNode(sibCenterNodeIndex);

		for (int i : affectedTileIndices) reconnectTile(*getTile(i));

		checkCornerConnections();
	}
};