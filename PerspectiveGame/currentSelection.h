#pragma once

#include <iostream>

#include "inputManager.h"
//#include "tileManager.h"
#include "entity.h"
#include "entityManager.h"
#include "basisManager.h"
#include "buttonManager.h"
#include "cameraManager.h"
#include "positionNodeNetwork.h"
#include "pov.h"

struct QueuedEntity {
	int tileIndex;
	EntityType type;
	LocalOrientation orientation;
	LocalDirection direction;

	QueuedEntity(int tileIndex, EntityType type, LocalDirection direction, LocalOrientation orientation) :
		tileIndex(tileIndex), type(type), orientation(orientation), direction(direction)
	{}
};

struct CurrentSelection {

	enum RelativeTileOrientation {
		RELATIVE_TILE_ORIENTATION_UP,
		RELATIVE_TILE_ORIENTATION_FLAT,
		RELATIVE_TILE_ORIENTATION_DOWN
	};

	InputManager* p_inputManager;
	//TileManager* p_tileManager;
	EntityManager* p_entityManager;
	ButtonManager* p_buttonManager;
	BasisManager* p_basisManager;
	Camera* p_camera;
	PositionNodeNetwork* p_nodeNetwork;
	POV* p_pov;

	PositionNode* hoveredTile;
	int	hoveredTileConnectionIndex;
	POV* addTileParentPOV;
	LocalDirection addTileParentAddDirection;

	TileInfo heldTileInfo;
	glm::vec3 heldTilePos;

	//Tile::Basis heldBasis;
	Entity* heldEntity;
	LocalDirection heldEntityDirection;

	std::vector<QueuedEntity> queuedEntities;

	bool canEditEntities;
	bool canEditBases;
	bool canEditTiles;

	bool tryingToAddTile;
	glm::vec3 heldTileColor;
	RelativeTileOrientation heldTileRelativeOrientation;

	bool leftClick = false;

	CurrentSelection(InputManager* im, EntityManager* em, ButtonManager* bm, Camera* (cam),
					 BasisManager* bam, PositionNodeNetwork* nn, POV* pov) : p_inputManager(im),
		p_entityManager(em), p_buttonManager(bm), p_camera(cam), p_basisManager(bam), p_nodeNetwork(nn), p_pov(pov)
	{
		Button* b = &p_buttonManager->buttons[ButtonManager::pov3d3rdPersonViewButtonIndex];
		addTileParentPOV = new POV(p_nodeNetwork, p_camera, b);
		hoveredTile = nullptr;
		hoveredTileConnectionIndex = 0;

		canEditTiles = false;
		tryingToAddTile = false;
		tryingToAddTile = true; // TESTING, TEMP!

		heldTileRelativeOrientation = CurrentSelection::RELATIVE_TILE_ORIENTATION_DOWN;
		heldTileColor = glm::vec3(1, 0, 0);

		canEditEntities = true;
		//heldEntity = new Entity(-1, -1, ENTITY_TYPE_OMNI, LOCAL_DIRECTION_0, LOCAL_ORIENTATION_0, glm::vec4(0, 0, 0, 0));
		canEditBases = false;
		//heldBasis.type = BASIS_TYPE_FORCE_SINK;
	}

	~CurrentSelection()
	{
		delete heldEntity;
		delete addTileParentPOV;
	}

	void addQueuedEntities()
	{
		for (QueuedEntity e : queuedEntities) {
			//p_entityManager->createEntity(e.tileIndex, e.type, e.direction, e.orientation);
		}
		queuedEntities.clear();
	}

	void findHoveredTile()
	{
		// find what tile the cursor is hovering over:
		// This algorithm mirrors the on in 2D3rdPersonPOV frag shader, which is documented better.
		Button* sceneView = &p_buttonManager->buttons[ButtonManager::pov2d3rdPersonViewButtonIndex]; // CHANGE TO BE MORE GENERIC
		glm::mat4 inWindowToWorldSpace =
			glm::inverse(p_camera->getProjectionMatrix((float)sceneView->pixelWidth(), (float)sceneView->pixelHeight()));
		glm::vec2 cursorWorldPos = glm::vec2(inWindowToWorldSpace * glm::vec4(-CursorScreenPos.x, CursorScreenPos.y, 0, 1));
		POV targetPOV = *p_pov;

		// If the cursor is inside the povTile, then we dont have to do the move complex steps:
		if (cursorWorldPos.x > 0 && cursorWorldPos.x < 1 && cursorWorldPos.y > 0 && cursorWorldPos.y < 1) {
			*addTileParentPOV = *p_pov;
			// TODO: nodes have tile centers at (n, n) whereas cursor world pos/GPU has tile centers at (0.5n, 0.5n)
			// syncronize these!
			if (abs(cursorWorldPos.x - 0.5f) > abs(cursorWorldPos.y - 0.5f)) {
				addTileParentAddDirection = (cursorWorldPos.x > 0.5f) ? p_pov->getEast() : p_pov->getWest();
			}
			else {
				addTileParentAddDirection = (cursorWorldPos.y > 0.5f) ? p_pov->getNorth() : p_pov->getSouth();
			}
		}

		glm::vec2 povPos = (glm::vec2)p_camera->viewPlanePos;
		glm::vec2 povToCursorPos = cursorWorldPos - povPos;
		float totalDist = glm::length(povToCursorPos);
		glm::vec2 stepDist = totalDist / abs(povToCursorPos);

		bool goingEast = povToCursorPos.x > 0.0f;
		bool goingNorth = povToCursorPos.y > 0.0f;

		glm::vec2 runningDist;
		runningDist.x = goingEast ? (1.0f - povPos.x) * stepDist.x : povPos.x * stepDist.x;
		runningDist.y = goingNorth ? (1.0f - povPos.y) * stepDist.y : povPos.y * stepDist.y;

		const int MAX_STEPS = 1000; int stepCount = 0;
		float currentDist = 0;

		// Raycast:
		while (stepCount++ < MAX_STEPS) {
			if (runningDist.x > totalDist && runningDist.y > totalDist) break; // We have arrived!
			
			if (runningDist.x < runningDist.y) {
				runningDist.x += stepDist.x;
				addTileParentAddDirection = goingEast ? targetPOV.getEast() : targetPOV.getWest();
			}
			else { // runningDist.x > runningDist.y
				runningDist.y += stepDist.y;
				addTileParentAddDirection = goingNorth ? targetPOV.getNorth() : targetPOV.getSouth();
			}

			*addTileParentPOV = targetPOV; // keeps the last pov for later
			targetPOV.shiftTileSimple(addTileParentAddDirection);
		}
		hoveredTile = targetPOV.getNode();
	}

	void findPreviewTile()
	{
		PositionNode* node = addTileParentPOV->getNode();
		TileType addParentTileType = (TileType)node->getNodeType();
		node = p_nodeNetwork->getNode(node->getNeighborIndex(addTileParentAddDirection));
		glm::vec3 sideNodePos = node->getPosition();
		glm::vec3 toSideNode = 2.0f * (sideNodePos - addTileParentPOV->getNode()->getPosition());

		TileType newTileType;
		//glm::vec3 sideToCenterNodePos;
		switch (heldTileRelativeOrientation) {
		case RELATIVE_TILE_ORIENTATION_FLAT:
			heldTilePos = sideNodePos + 0.5f * toSideNode;
			newTileType = addParentTileType;
			break;
		case RELATIVE_TILE_ORIENTATION_UP:
			heldTilePos = sideNodePos + 0.5f * tnav::getNormal(addParentTileType);
			newTileType = tnav::getTileType(-toSideNode);
			break;
		default: // RELATIVE_TILE_ORIENTATION_DOWN:
			heldTilePos = sideNodePos - 0.5f * tnav::getNormal(addParentTileType);
			newTileType = tnav::getTileType(toSideNode);
			break;
		}

		glm::vec3 color = tnav::getNormal(newTileType);
		if (color.x < 0 || color.y < 0 || color.z < 0) { color *= -0.8; }
		heldTileInfo = TileInfo(newTileType,-1, -1, -1, color);
	}

	void tryEditTiles()
	{
		using namespace tnav;

		if (p_inputManager->leftClicked()) {
			p_nodeNetwork->createTilePair(heldTilePos, tnav::getSuperTileType(heldTileInfo.type));
		}
		else if (p_inputManager->rightClicked()) {
			if (hoveredTile != p_pov->getNode())
				p_nodeNetwork->deleteTilePair(p_nodeNetwork->getTileInfo(hoveredTile->getTileInfoIndex()));
			//p_tileManager->deleteTilePair(hoveredTile, false);
		}
	}
	void tryEditBases()
	{
		/*if (p_inputManager->leftMouseButtonClicked()) {
			p_basisManager->addBasis(hoveredTile, heldBasis.localOrientation, heldBasis.type);
		}
		else if (p_inputManager->rightMouseButtonClicked()) {
			p_basisManager->deleteBasis(hoveredTile);
		}*/
	}
	void tryEditEntities()
	{
		/*if (p_inputManager->leftMouseButtonClicked() && hoveredTile->entityIndices[8] == -1) {
			queuedEntities.push_back(QueuedEntity(hoveredTile->index, heldEntity->type,
												  heldEntity->getDirection(0), heldEntity->getOrientation(0)));
		}
		else if (p_inputManager->rightMouseButtonClicked()) {
			for (int i = 0; i < 9; i++) {
				if (hoveredTile->hasEntity(LocalPosition(i))) {
					p_entityManager->deleteEntity(&p_entityManager->entities[hoveredTile->entityIndices[i]]);
				}
			};
		}*/
	}

	void tryEditWorld()
	{
		if (canEditTiles) { tryEditTiles(); }
		else if (canEditBases) { tryEditBases(); }
		else if (canEditEntities) { tryEditEntities(); }
	}

	void print(RelativeTileOrientation rto)
	{
		switch (rto) {
		case RELATIVE_TILE_ORIENTATION_UP: std::cout << " RELATIVE_TILE_ORIENTATION_UP__ "; break;
		case RELATIVE_TILE_ORIENTATION_FLAT:std::cout << " RELATIVE_TILE_ORIENTATION_FLAT "; break;
		case RELATIVE_TILE_ORIENTATION_DOWN:std::cout << " RELATIVE_TILE_ORIENTATION_DOWN "; break;
		}
	}

	void update()
	{
		findHoveredTile();
		findPreviewTile();

		if (p_inputManager->keys[ROTATE_KEY].click) {
			if (canEditBases) {
				//heldBasis.localOrientation = LocalDirection((heldBasis.localOrientation + 1) % 4);
			}
			if (canEditEntities) {
				//heldEntity->setDirection(0, LocalDirection((heldEntity->getDirection(0) + 1) % 8));
				//tnav::println(heldEntity->getDirection(0));
			}
			if (canEditTiles) {
				// NOTE: IMGUI IS HANDLING THIS RN!
				//heldTileRelativeOrientation = RelativeTileOrientation((heldTileRelativeOrientation + 1) % 3);
				//print(heldTileRelativeOrientation);
			}
		}
	}
};