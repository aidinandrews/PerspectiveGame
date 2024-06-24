#pragma once

#include <iostream>

#include "tile.h"
#include "inputManager.h"

struct CurrentSelection {

	enum RelativeTileOrientation {
		RELATIVE_TILE_ORIENTATION_UP,
		RELATIVE_TILE_ORIENTATION_FLAT,
		RELATIVE_TILE_ORIENTATION_DOWN
	};

	InputManager* p_inputManager;

	Tile* hoveredTile;
	int	hoveredTileConnectionIndex;

	Tile::Basis heldBasis;
	Tile::Entity heldEntity;

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
	
	CurrentSelection(InputManager*im) : p_inputManager(im) {
		hoveredTile = nullptr;
		canEditEntities = false;
		canEditBases = false;
		canEditTiles = false;

		canEditEntities = true;
		heldEntity.type = Tile::Entity::Type::BUILDING_FORCE_BLOCK;
		heldBasis.type = Tile::Basis::Type::FORCE_SINK;
	}

	void update() {
		if (p_inputManager->keys[ROTATE_KEY].click) {
			if (canEditBases) {
				heldBasis.orientation = (heldBasis.orientation + 1) % 4;
			}
			if (canEditEntities) {
				heldEntity.orientation = (heldEntity.orientation + 1) % 4;
				std::cout << heldEntity.orientation << std::endl;
			}
		}
	}
};