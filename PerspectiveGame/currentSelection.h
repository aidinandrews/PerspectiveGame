#pragma once

#include <iostream>

#include "inputManager.h"
#include "tileManager.h"
#include "entityManager.h"
#include "basisManager.h"
#include "buttonManager.h"
#include "cameraManager.h"

struct CurrentSelection {

	enum RelativeTileOrientation {
		RELATIVE_TILE_ORIENTATION_UP,
		RELATIVE_TILE_ORIENTATION_FLAT,
		RELATIVE_TILE_ORIENTATION_DOWN
	};

	InputManager* p_inputManager;
	TileManager* p_tileManager;
	EntityManager* p_entityManager;
	ButtonManager* p_buttonManager;
	BasisManager* p_basisManager;
	Camera* p_camera;

	Tile* hoveredTile;
	int	hoveredTileConnectionIndex;

	Tile::Basis heldBasis;
	Entity heldEntity;

	bool canEditEntities;
	bool canEditBases;
	bool canEditTiles;

	bool tryingToAddTile;
	Tile addTile;
	glm::vec3 addTileColor;
	RelativeTileOrientation addTileRelativeOrientation;

	// Tile before the preview tile we want to add.  tile 'connecting' preview tile to scene:
	TileTarget addTileParentTarget; 
	int addTileParentSideConnectionIndex;
	
	CurrentSelection(InputManager*im,TileManager*tm,EntityManager*em,ButtonManager*bm,Camera*(cam),BasisManager*bam) : p_inputManager(im),p_tileManager(tm),p_entityManager(em),p_buttonManager(bm),p_camera(cam),p_basisManager(bam) {
		hoveredTile = nullptr;
		hoveredTileConnectionIndex = 0;

		canEditEntities = false;
		canEditBases = false;
		canEditTiles = false;

		canEditEntities = true;
		heldEntity.type = Entity::Type::BUILDING_FORCE_BLOCK;
		heldBasis.type = BasisType::FORCE_SINK;

		tryingToAddTile = false;
		tryingToAddTile = true; // TESTING, TEMP!

		addTileRelativeOrientation = CurrentSelection::RELATIVE_TILE_ORIENTATION_DOWN;
		addTileColor = glm::vec3(1, 0, 0);
	}

	void findHoveredTile() {
		// find what tile the cursor is hovering over:
	// This algorithm mirrors the on in 2D3rdPersonPOV frag shader, which is documented better.
		Button* sceneView = &p_buttonManager->buttons[ButtonManager::pov2d3rdPersonViewButtonIndex]; // CHANGE TO BE MORE GENERIC
		glm::mat4 inWindowToWorldSpace = glm::inverse(p_camera->getProjectionMatrix((float)sceneView->pixelWidth(),
			(float)sceneView->pixelHeight()));
		glm::vec2 cursorWorldPos = glm::vec2(inWindowToWorldSpace * glm::vec4(-CursorScreenPos.x, CursorScreenPos.y, 0, 1));

		// If the cursor is inside the povTile, then we dont have to do the move complex steps:
		if (cursorWorldPos.x > 0 && cursorWorldPos.x < 1 && cursorWorldPos.y > 0 && cursorWorldPos.y < 1) {
			hoveredTile = p_tileManager->povTile.tile;
			addTileParentTarget = p_tileManager->povTile;
			return;
		}

		// Welp, here goes the more complex steps:
		glm::vec2 povPos = (glm::vec2)p_camera->viewPlanePos;
		glm::vec2 povPosToPixelPos = cursorWorldPos - povPos;
		float totalDist = glm::length(povPosToPixelPos);
		glm::vec2 stepDist = totalDist / abs(povPosToPixelPos);

		bool goingRight = povPosToPixelPos.x > 0.0f;
		bool goingUp = povPosToPixelPos.y > 0.0f;
		glm::vec2 runningDist;

		if (goingRight) { runningDist.x = (1.0f - povPos.x) * stepDist.x; }
		else { runningDist.x = povPos.x * stepDist.x; }

		if (goingUp) { runningDist.y = (1.0f - povPos.y) * stepDist.y; }
		else { runningDist.y = povPos.y * stepDist.y; }

		const int MAX_STEPS = 1000; int stepCount = 0;
		float currentDist = 0;
		TileTarget target = p_tileManager->povTile;
		hoveredTileConnectionIndex = 0;
		int drawSideIndex = 0;
		glm::vec2 drawTileOffset(0, 0);

		while (stepCount < MAX_STEPS) {

			if (runningDist.x < runningDist.y) {

				currentDist = runningDist.x;
				if (currentDist > totalDist) {
					break; // We have arrived!
				}
				runningDist.x += stepDist.x;

				// Shift tile target left/right depending on goingRight:
				if (goingRight) {
					hoveredTileConnectionIndex = target.initialSideIndex;
					drawSideIndex = 0;
					drawTileOffset += glm::vec2(1, 0);
				}
				else { // Going left:
					hoveredTileConnectionIndex = (target.initialSideIndex + target.sideInfosOffset * 2) % 4;
					drawSideIndex = 2;
					drawTileOffset += glm::vec2(-1, 0);
				}
			}
			else { // runningDist.x > runningDist.y

				currentDist = runningDist.y;
				if (currentDist > totalDist) {
					break; // We have arrived!
				}
				runningDist.y += stepDist.y;

				// Shift tile target up/down depending on goingUp:
				if (goingUp) {
					hoveredTileConnectionIndex = (target.initialSideIndex + target.sideInfosOffset * 3) % 4;
					drawSideIndex = 3;
					drawTileOffset += glm::vec2(0, 1);
				}
				else { // Going down:
					hoveredTileConnectionIndex = (target.initialSideIndex + target.sideInfosOffset * 1) % 4;
					drawSideIndex = 1;
					drawTileOffset += glm::vec2(0, -1);
				}
			}
			addTileParentTarget = target;
			target = TileManager::adjustTileTarget(&target, drawSideIndex);
			stepCount++;
		}

		hoveredTile = target.tile;

		addTileParentSideConnectionIndex = (addTileParentTarget.initialVertIndex
			+ drawSideIndex * addTileParentTarget.sideInfosOffset)
			% 4;
	}
	void findPreviewTile() {
		// Figure out the preview tile's type:
		int infosOffset = hoveredTileConnectionIndex - addTileParentTarget.initialSideIndex;
		if (infosOffset < 0) {
			infosOffset = abs(infosOffset) * 3 % 4;
		}
		int v1Index = (addTileParentTarget.initialVertIndex + infosOffset) % 4;
		glm::ivec3 v1 = addTileParentTarget.tile->getVertPos(v1Index);
		glm::ivec3 v2 = addTileParentTarget.tile->getVertPos((addTileParentTarget.initialVertIndex + infosOffset + addTileParentTarget.sideInfosOffset) % 4);
		glm::ivec3 v3, v4;

		switch (addTileRelativeOrientation) {
		case CurrentSelection::RELATIVE_TILE_ORIENTATION_DOWN:
			v3 = v1 - addTileParentTarget.tile->normal();
			v4 = v2 - addTileParentTarget.tile->normal();
			break;
		case CurrentSelection::RELATIVE_TILE_ORIENTATION_FLAT:
			glm::ivec3 offset = v1 - addTileParentTarget.tile->getVertPos((addTileParentTarget.initialVertIndex + infosOffset + addTileParentTarget.sideInfosOffset * 3) % 4);
			v3 = v1 + offset;
			v4 = v2 + offset;
			break;
		default: /*case RELATIVE_TILE_ORIENTATION_UP:*/
			v3 = v1 + addTileParentTarget.tile->normal();
			v4 = v2 + addTileParentTarget.tile->normal();
			break;
		}
		Tile::Type tileType = Tile::getTileType(v1, v2, v3, v4);
		TileSubType tileSubype = Tile::tileSubType(tileType, true);
		glm::ivec3 maxVert = Tile::getMaxVert(v1, v2, v3, v4);
		addTile = Tile(tileSubype, maxVert);
	}

	void tryEditTiles() {
		if (p_inputManager->leftMouseButtonClicked()) {
			p_tileManager->createTilePair(
				Tile::superTileType(addTile.type), addTile.maxVert, addTileColor, addTileColor * 0.5f);
		}
		else if (p_inputManager->rightMouseButtonClicked()) {
			p_tileManager->deleteTile(hoveredTile);
		}
	}
	void tryEditBases() {
		if (p_inputManager->leftMouseButtonClicked()) {
			p_basisManager->addBasis(hoveredTile, heldBasis.localOrientation, heldBasis.type);
		}
		else if (p_inputManager->rightMouseButtonClicked()) {
			p_basisManager->deleteBasis(hoveredTile);
		}
	}
	void tryEditEntities() {
		if (p_inputManager->leftMouseButtonClicked() && hoveredTile->entityIndices[8] == -1) {

			p_entityManager->createEntity(hoveredTile->index, heldEntity.type, heldEntity.localOrientation, true);

			std::cout << "new num entities after add: "<<p_entityManager->entities.size() << std::endl;
		}
		else if (p_inputManager->rightMouseButtonClicked()) {
			for (int i = 0; i < 9; i++) {
				if (hoveredTile->hasEntity(LocalPosition(i))) {
					p_entityManager->deleteEntity(&p_entityManager->entities[hoveredTile->entityIndices[i]]);
					std::cout << "entities left after deletion: "<<p_entityManager->entities.size() << std::endl;
				}
			}
		}
	}

	void tryEditWorld() {
		     if (canEditTiles)    { tryEditTiles();    }
		else if (canEditBases)    { tryEditBases();    }
		else if (canEditEntities) { tryEditEntities(); }
	}

	void update() {
		findHoveredTile();
		findPreviewTile();

		if (p_inputManager->keys[ROTATE_KEY].click) {
			if (canEditBases) {
				heldBasis.localOrientation = LocalDirection((heldBasis.localOrientation + 1) % 4);
				std::cout << heldBasis.localOrientation << std::endl;
			}
			if (canEditEntities) {
				heldEntity.localOrientation = LocalDirection((heldEntity.localOrientation + 1) % 4);
			}
		}

		tryEditWorld();
	}

	void cyclePreviewTileOrientation() {
		switch (addTileRelativeOrientation) {
		case CurrentSelection::RELATIVE_TILE_ORIENTATION_DOWN:
			addTileRelativeOrientation = CurrentSelection::RELATIVE_TILE_ORIENTATION_FLAT;
			return;
		case CurrentSelection::RELATIVE_TILE_ORIENTATION_FLAT:
			addTileRelativeOrientation = CurrentSelection::RELATIVE_TILE_ORIENTATION_UP;
			return;
		default: /*RELATIVE_TILE_ORIENTATION_UP*/
			addTileRelativeOrientation = CurrentSelection::RELATIVE_TILE_ORIENTATION_DOWN;
			return;
		}
	}
};