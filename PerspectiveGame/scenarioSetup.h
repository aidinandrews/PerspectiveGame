#pragma once

#include <iostream>

#include"currentSelection.h"
#include"tileManager.h"
#include"entityManager.h"

void create4x4TileGrid(TileManager* tm)
{
	for (int w = 1; w < 5; w++) {
		for (int h = 1; h < 5; h++) {
			tm->createTilePair(TILE_TYPE_XY, glm::ivec3(w, h, 0), glm::vec3(0, 0, 1), glm::vec3(0, 0, 0.5));
		}
	}
	tm->povTile.node = tm->tiles[0];
}

void clearWorld(TileManager* tm, EntityManager* em)
{
	int s = (int)em->entities.size();
	for (int i = 0; i < s; i++) {
		em->deleteEntity(&em->entities[0]);
	}
	s = (int)tm->tiles.size() / 2;
	for (int i = 0; i < s; i++) {
		tm->deleteTilePair(tm->tiles[0], true);
	}
	std::cout <<"num tiles after delete: " << tm->tiles.size() << std::endl;
}

void fullContactDirectCollision(TileManager* tm, EntityManager* em, CurrentSelection* cs)
{
	clearWorld(tm, em);
	create4x4TileGrid(tm);
	em->createEntity(0, ENTITY_TYPE_OMNI, LOCAL_DIRECTION_0, LOCAL_ORIENTATION_0);
	em->createEntity(24, ENTITY_TYPE_OMNI, LOCAL_DIRECTION_2, LOCAL_ORIENTATION_0);
	std::cout << "Scenario setup: Direct orthogonal collision from center positions." << std::endl;
}

void fullContactDirectCollisionOnStatic(TileManager* tm,EntityManager* em, CurrentSelection* cs)
{
	clearWorld(tm, em);
	create4x4TileGrid(tm);
	em->createEntity(0, ENTITY_TYPE_OMNI, LOCAL_DIRECTION_0, LOCAL_ORIENTATION_0);
	em->createEntity(16, ENTITY_TYPE_OMNI, LOCAL_DIRECTION_STATIC, LOCAL_ORIENTATION_0);
	std::cout << "Scenario setup: Direct orthogonal collision with static entity from center positions." << std::endl;
}

void fullContactTCollision(TileManager* tm, EntityManager* em, CurrentSelection* cs)
{
	clearWorld(tm, em);
	create4x4TileGrid(tm);
	em->createEntity(0, ENTITY_TYPE_OMNI, LOCAL_DIRECTION_0, LOCAL_ORIENTATION_0);
	em->createEntity(12, ENTITY_TYPE_OMNI, LOCAL_DIRECTION_1, LOCAL_ORIENTATION_0);
	std::cout << "Scenario setup: Side orthogonal collision from center positions." << std::endl;
}

void setupScenarioCornerCollisionFromCenter(TileManager* tm, EntityManager* em, CurrentSelection* cs)
{
	clearWorld(tm, em);
	create4x4TileGrid(tm);
	em->createEntity(0, ENTITY_TYPE_OMNI, LOCAL_DIRECTION_0, LOCAL_ORIENTATION_0);
	em->createEntity(20, ENTITY_TYPE_OMNI, LOCAL_DIRECTION_1, LOCAL_ORIENTATION_0);
	std::cout << "Scenario setup: Corner orthogonal collision from center positions." << std::endl;
}

void setupScenarioDirectCollisionFromEdge(TileManager* tm, EntityManager* em, CurrentSelection* cs)
{
	clearWorld(tm, em);
	create4x4TileGrid(tm);
	em->createEntity(0, ENTITY_TYPE_OMNI, LOCAL_DIRECTION_0, LOCAL_ORIENTATION_0);
	em->createEntity(16, ENTITY_TYPE_OMNI, LOCAL_DIRECTION_2, LOCAL_ORIENTATION_0);
	std::cout << "Scenario setup: Direct orthogonal collision from edge positions." << std::endl;
}

void setupCollisionScenario4(TileManager* tm, EntityManager* em, CurrentSelection* cs)
{
	clearWorld(tm, em);
	create4x4TileGrid(tm);
	em->createEntity(0, ENTITY_TYPE_OMNI, LOCAL_DIRECTION_3_0, LOCAL_ORIENTATION_0);
	em->createEntity(24, ENTITY_TYPE_OMNI, LOCAL_DIRECTION_2_3, LOCAL_ORIENTATION_0);
	std::cout << "Scenario setup: Direct orthogonal collision from edge positions." << std::endl;
}

void setupCollisionScenario5(TileManager* tm, EntityManager* em, CurrentSelection* cs)
{
	clearWorld(tm, em);
	create4x4TileGrid(tm);
	em->createEntity(0, ENTITY_TYPE_OMNI, LOCAL_DIRECTION_3_0, LOCAL_ORIENTATION_0);
	em->createEntity(16, ENTITY_TYPE_OMNI, LOCAL_DIRECTION_2, LOCAL_ORIENTATION_0);
	std::cout << "Scenario setup: Direct orthogonal collision from edge positions." << std::endl;
}

void setupCollisionScenario6(TileManager* tm, EntityManager* em, CurrentSelection* cs)
{
	clearWorld(tm, em);
	create4x4TileGrid(tm);
	//em->createEntity(0, ENTITY_TYPE_OMNI, LOCAL_DIRECTION_3_0, LOCAL_ORIENTATION_0);
	em->createEntity(6, ENTITY_TYPE_OMNI, LOCAL_DIRECTION_0_1, LOCAL_ORIENTATION_0);
	em->createEntity(24, ENTITY_TYPE_OMNI, LOCAL_DIRECTION_2_3, LOCAL_ORIENTATION_0);
	em->createEntity(30, ENTITY_TYPE_OMNI, LOCAL_DIRECTION_1_2, LOCAL_ORIENTATION_0);
	//em->createEntity(0, ENTITY_TYPE_OMNI, LOCAL_DIRECTION_0, LOCAL_ORIENTATION_0);
	//em->createEntity(8, ENTITY_TYPE_OMNI, LOCAL_DIRECTION_0_1, LOCAL_ORIENTATION_0);
	std::cout << "Scenario setup: Direct orthogonal collision from edge positions." << std::endl;
}

void setupTestScenario(int scenarioID, TileManager* tm, EntityManager* em, CurrentSelection* cs)
{
	switch (scenarioID) {
	case 0: 
		setupCollisionScenario4(tm, em, cs);
		break;
	case 1: 
		fullContactDirectCollisionOnStatic(tm, em, cs); 
		break;
	case 2: 
		fullContactTCollision(tm, em, cs); 
		break;
	case 3: 
		setupScenarioCornerCollisionFromCenter(tm, em, cs); 
		break;
	case 4: 
		fullContactDirectCollision(tm, em, cs);
		break;
	default: 
		clearWorld(tm, em); create4x4TileGrid(tm); 
		std::cout << "Default Scenario: Empty 4x4 world." << std::endl;
		break;
	}
}

int ticksPerScenario(int scenarioID)
{
	switch (scenarioID) {
	case 0:	return 4;
	case 1:	return 4;
	case 2:	return 4;
	case 3:	return 4;
	case 4:	return 4;
	default: return 4;
	}
}
