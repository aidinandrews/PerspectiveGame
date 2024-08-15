#include "entityManager.h"

void EntityManager::createEntity(int tileIndex, EntityType entityType, LocalDirection orientation, bool override) {

	Tile* tile = p_tileManager->tiles[tileIndex];

	if (!override && tile->isObstructed(LOCAL_POSITION_CENTER)) {
		return;
	}

	entities.push_back(Entity(entityType, LOCAL_POSITION_CENTER, LOCAL_DIRECTION_STATIC, orientation,
		0.0f, -1, -1, glm::vec4(randColor(), 1)));
	Entity* newEntity = &entities.back();
	newEntity->index = (int)entities.size() - 1;
	newEntity->tileIndices[0] = tileIndex;

	tile->entityIndices[LOCAL_POSITION_CENTER] = newEntity->index;
	tile->entityInfoIndices[LOCAL_POSITION_CENTER] = 0; // If an entity is only in one tile, info[0] corrisponds to that tile.
	tile->obstructionMask |= TileNavigator::getObstructionMask(LOCAL_POSITION_CENTER);

	if (tile->hasForce()) {
		newEntity->localDirections[0] = tile->forceLocalDirection;
	}
	else {
		newEntity->staticListIndex = (int)staticEntityIndices.size();
		staticEntityIndices.push_back(newEntity->index);
		return; // A static entity cannot be a follower or leader.
	}
	
	// Make sure to try and connect up a line if entities if needed:
	if (tryAddLeaderCenter(newEntity) == false) {
		//std::cout <<"num leaders: " << leaderInfos.size() << std::endl;
		int leaderIndex = (int)leaderInfos.size();
		leaderInfos.push_back(LeaderInfo(leaderIndex, newEntity->index, tile->forceLocalDirection, 1));
		newEntity->leaderListIndex = leaderIndex;
	}
	tryAddFollowerCenter(newEntity);
	//int followerMass = tryAddFollowerCenter(newEntity);
	//leaderInfos.back().mass += followerMass;
}

void EntityManager::removeEntityFromLeaderList(Entity* entity) {
	if (entity->hasFollower()) {
		Entity* follower = getFollower(entity);
		leaderInfos[entity->leaderListIndex].direction = follower->localDirections[0];
		if (follower->tileIndices[1] != -1) {
			leaderInfos[entity->leaderListIndex].direction = follower->localDirections[1];
		}
		leaderInfos[entity->leaderListIndex].entityIndex = follower->index;
		leaderInfos[entity->leaderListIndex].mass -= 1;
	}
	else {
		if (entity->leaderListIndex == leaderInfos.size() - 1) {
			leaderInfos.pop_back();
		}
		else {
			int leaderIndex = entity->leaderListIndex;
			std::swap(leaderInfos[leaderIndex], leaderInfos.back());
			leaderInfos[leaderIndex].leaderIndex = leaderIndex;
			entities[leaderInfos[leaderIndex].entityIndex].leaderListIndex = leaderIndex;
			leaderInfos.pop_back();
		}
	}
}

void EntityManager::removeEntityFromStaticList(Entity* entity) {
	if (entity->staticListIndex == staticEntityIndices.size() - 1) {
		staticEntityIndices.pop_back();
	}
	else {
		int staticIndex = entity->staticListIndex;
		std::swap(staticEntityIndices[staticIndex], staticEntityIndices.back());
		entities[staticEntityIndices[staticIndex]].staticListIndex = staticIndex;
		staticEntityIndices.pop_back();
	}
}

void EntityManager::removeEntityFromEntityList(Entity* entity) {
	if (entity->index == entities.size() - 1) {
		entities.pop_back();
	}
	else {
		int entityIndex = entity->index;
		std::swap(*entity, entities.back());
		entities.pop_back();
		Entity* swappedEntity = &entities[entityIndex];
		swappedEntity->index = entityIndex;

		// clean up other connections:
		if (swappedEntity->isStatic()) {
			staticEntityIndices[swappedEntity->staticListIndex] = entityIndex;
		}
		else if (swappedEntity->isLeader()) {
			leaderInfos[swappedEntity->leaderListIndex].entityIndex = entityIndex;
		}
		for (int i = 0; i < 2; i++) {
			if (swappedEntity->connectedToTile(i)) {
				getTile(swappedEntity, i)->entityIndices[swappedEntity->localPositions[i]] = entityIndex;
			}
		}
	}
}

void EntityManager::deleteEntity(Entity* entity) {
	for (int i = 0; i < 2; i++) {
		if (entity->connectedToTile(i)) {
			Tile*  tile = getTile(entity, i);
			tile->entityIndices[entity->localPositions[i]] = -1;
			obstructionMaskChanges.push_back(ObstructionMaskChange(
				tile->index, TileNavigator::getObstructionMask(entity->localPositions[i]), 
				tile->index, 0));
		}
	}

	if (entity->isLeader()) { 
		//removeEntityFromLeaderList(entity); 
		demoteLeader(entity->leaderListIndex);
	}
	if (entity->isStatic()) { removeEntityFromStaticList(entity); }
	removeEntityFromEntityList(entity);
}

int EntityManager::tryAddFollowerCenter(Entity* entity) {
	Tile* tile = getTile0(entity);
	LocalPosition followerPosition = LocalPosition((tile->forceLocalDirection + 2) % 4);

	if (tile->entityIndices[followerPosition] == NO_ENTITY_INDEX) { 
		return 0; 
	}

	Entity* follower = &entities[tile->entityIndices[followerPosition]];
	bool followerInfoIndex = tile->entityInfoIndices[followerPosition];
	LocalDirection followerDirection = follower->localDirections[followerInfoIndex];

	if (followerDirection != tile->forceLocalDirection) {
		return 0;
	}
	
	entity->followingEntityIndex = follower->index;
	follower->leadingEntityIndex = entity->index;

	if (follower->isLeader()) { demoteLeader(follower->leaderListIndex); }

	int followerCount = 1;
	while (follower->hasFollower()) {
		follower = getFollower(follower);
		followerCount++;
	}
	return followerCount;
}

bool EntityManager::tryAddLeaderCenter(Entity* entity) {
	Tile* tile = getTile(entity, 0);
	LocalPosition leaderPosition = (LocalPosition)tile->forceLocalDirection;

	if (tile->entityIndices[leaderPosition] == NO_ENTITY_INDEX) { return false; }

	Entity* leader = &entities[tile->entityIndices[leaderPosition]];
	bool infoIndex = tile->entityInfoIndices[leaderPosition];
	LocalDirection leaderDirection = leader->localDirections[infoIndex];
	
	if (leaderDirection == tile->forceLocalDirection) {
		leader->followingEntityIndex = entities.back().index;
		entity->leadingEntityIndex = leader->index;
		return true;
	}
	return false;
}

void EntityManager::manageEntityCollision(LeaderInfo* leaderInfo) {

	Entity* entity = &entities[leaderInfo->entityIndex];
	// find the opposing entity:
	Tile* tile = p_tileManager->tiles[entity->tileIndices[0]];
	Tile* opposingTile = tile;
	// opposing entities are always 2 positions away, so first step once:
	LocalPosition entityPos = entity->localPositions[0];
	LocalDirection entityDir = entity->localDirections[0];
	LocalPosition middle = LOCAL_POSITION_INVALID;
	LocalPosition opposingPos = LOCAL_POSITION_INVALID;
	
	bool inOtherTile = false;
	if (entity->inEdgePosition()) {
		inOtherTile = true;
		opposingTile = tile->neighbor(entityDir);
		middle = LocalPosition(tile->sideInfos.connectedSideIndices[entityDir] + 4); // middle indices are 4 away from edges.
		entityDir = LocalDirection((middle + 2) % 4);
	}
	else {
		middle = TileNavigator::nextLocalPosition(entityPos, entityDir);
	}

	#define middleIsEdgePos middle < 4
	if (middleIsEdgePos) {
		inOtherTile = true;
		opposingTile = tile->neighbor(entityDir);
		opposingPos = LocalPosition(tile->sideInfos.connectedSideIndices[entityDir] + 4);
	}
	else {
		opposingPos = TileNavigator::nextLocalPosition(middle, entityDir);
	}
	if (entity->index == 0) {
		std::cout << "moving 0" << std::endl;
	}

	if (opposingTile->basis.type == BASIS_TYPE_FORCE_SINK) {
		LeaderInfo* newLeaderInfo = reverseEntityLineDir(entity);
		tryMoveLeader(newLeaderInfo);
		return;
	}
	if (opposingTile->hasEntity(opposingPos) == false) {
		// This shouldnt be possible, but just in case:
		std::cout << "OPPOSING ENTITY DOES NOT EXIST!" << std::endl;
		/*reverseEntityLineDir(entity, leaderInfo->leaderIndex);
		tryMoveLeader(leaderInfo);*/
		return;
	}
	// Now we know that there is an opposing entity, we can either add this entity as a follower or inverse the
	// directions of entities in both lines depending on the collision:
	Entity* opposingEntity = getEntity(opposingTile, opposingPos);
	// if there is a collision between two lines, than the lines must not be going in the same direction,
	// as the only time that is possible is on creating of an entity, and that outlier is solved for in the
	// createEntity function already!

	// TODO: change this so that on collision, funky Newton's Cradle stuff happens:
	reverseEntityLineDir(entity);
	tryMoveLeader(leaderInfo);

	reverseEntityLineDir(opposingEntity);
}

LeaderInfo* EntityManager::reverseEntityLineDir(Entity* head) {
	int lineMass = 1;

	std::swap(head->leadingEntityIndex, head->followingEntityIndex);
	head->localDirections[0] = TileNavigator::oppositeDirection(head->localDirections[0]);
	if (head->inEdgePosition()) {
		head->localDirections[1] = TileNavigator::oppositeDirection(head->localDirections[1]);

		std::swap(getTile0(head)->entityInfoIndices[head->localPositions[0]],
			getTile1(head)->entityInfoIndices[head->localPositions[1]]);
		std::swap(head->tileIndices[0], head->tileIndices[1]);
		std::swap(head->localDirections[0], head->localDirections[1]);
		std::swap(head->localPositions[0], head->localPositions[1]);
	}
	Entity* current = head;
	while (current->hasLeader()) {
		current = getLeader(current);
		std::swap(current->leadingEntityIndex, current->followingEntityIndex);
		current->localDirections[0] = TileNavigator::oppositeDirection(current->localDirections[0]);

		// Because entity info [0] must be the arriving tile, arriving/leaving info is swapped here:
		if (current->inEdgePosition()) {
			current->localDirections[1] = TileNavigator::oppositeDirection(current->localDirections[1]);

			std::swap(getTile0(current)->entityInfoIndices[current->localPositions[0]],
				getTile1(current)->entityInfoIndices[current->localPositions[1]]);
			std::swap(current->tileIndices[0], current->tileIndices[1]);
			std::swap(current->localDirections[0], current->localDirections[1]);
			std::swap(current->localPositions[0], current->localPositions[1]);
		}
		lineMass++;
	}
	Entity* newHead = current;
	LocalDirection newHeadDir = newHead->localDirections[0];

	if (newHead == head) {
		leaderInfos[head->leaderListIndex].direction = newHeadDir;
	}
	else {
		newHead->leaderListIndex = head->leaderListIndex;
		leaderInfos[newHead->leaderListIndex] = LeaderInfo(newHead->leaderListIndex, newHead->index, newHeadDir, lineMass);
		head->leaderListIndex = -1;
	}
	return &leaderInfos[newHead->leaderListIndex];
}

void EntityManager::moveFollowers(Entity* entity) {
	std::cout << "MOVING FOLLOWERS!!!" << std::endl << std::endl;

	int lastTileIndex = -1;
	LocalPosition lastPos = LOCAL_POSITION_INVALID;

	while (entity->hasFollower()) {
		entity = getFollower(entity);
		if (entity->inEdgePosition()) {
			lastTileIndex = entity->tileIndices[1];
			lastPos = entity->localPositions[1];

			getTile(entity, 0)->entityIndices[entity->localPositions[0]] = -1;
			getTile(entity, 1)->entityIndices[entity->localPositions[1]] = -1;
			
			entity->tileIndices[1] = -1;
			
			entity->localPositions[0] = LocalPosition(entity->localPositions[0] + 4);
			entity->localPositions[1] = LOCAL_POSITION_INVALID;

			entity->localDirections[1] = LOCAL_DIRECTION_INVALID;

			getTile0(entity)->entityIndices[entity->localPositions[0]] = entity->index;
			getTile0(entity)->entityInfoIndices[entity->localPositions[0]] = 0;
		}
		else if (entity->movingToEdge()) {
			lastTileIndex = entity->tileIndices[0];
			lastPos = entity->localPositions[0];

			getTile0(entity)->entityIndices[entity->localPositions[0]] = -1;

			entity->localPositions[1] = (LocalPosition)entity->localDirections[0];
			entity->localPositions[0] = (LocalPosition)getTile0(entity)->sideInfos.connectedSideIndices[entity->localDirections[0]];

			entity->tileIndices[1] = entity->tileIndices[0];
			entity->tileIndices[0] = getTile0(entity)->neighbor(entity->localDirections[0])->index;

			entity->localDirections[1] = entity->localDirections[0];
			entity->localDirections[0] = LocalDirection((entity->localPositions[0] + 2) % 4);

			getTile0(entity)->entityIndices[entity->localPositions[0]] = entity->index;
			getTile0(entity)->entityInfoIndices[entity->localPositions[0]] = 0;
			getTile1(entity)->entityIndices[entity->localPositions[1]] = entity->index;
			getTile1(entity)->entityInfoIndices[entity->localPositions[1]] = 1;

			obstructionMaskChanges.push_back(ObstructionMaskChange(
				lastTileIndex, TileNavigator::getObstructionMask(lastPos),
				entity->tileIndices[1], TileNavigator::getObstructionMask(entity->localPositions[1])));

			return;
		}
		else { // Internal movement is simplest:
			lastTileIndex = entity->tileIndices[0];
			lastPos = entity->localPositions[0];

			getTile0(entity)->entityIndices[entity->localPositions[0]] = -1;

			entity->localPositions[0] = TileNavigator::nextLocalPosition(entity->localPositions[0], entity->localDirections[0]);

			getTile0(entity)->entityIndices[entity->localPositions[0]] = entity->index;
			getTile0(entity)->entityInfoIndices[entity->localPositions[0]] = 0;
		}
	}

	obstructionMaskChanges.push_back(ObstructionMaskChange(
		lastTileIndex, TileNavigator::getObstructionMask(lastPos),
		entity->tileIndices[0], TileNavigator::getObstructionMask(entity->localPositions[0])));
}

bool EntityManager::tryMoveLeaderInterior(LeaderInfo* leaderInfo) {
	
	Entity* entity = &entities[leaderInfo->entityIndex];
	LocalPosition newPosition = TileNavigator::nextLocalPosition(entity->localPositions[0], entity->localDirections[0]);
	uint16_t currentMask = TileNavigator::getObstructionMask(entity->localPositions[0]);
	uint16_t newMask = TileNavigator::getObstructionMask(newPosition);
	Tile* tile = p_tileManager->tiles[entity->tileIndices[0]]; // If in only 1 tile, the first index points to it.
	bool obstructed = (tile->obstructionMask & ~currentMask) & newMask;
	if (obstructed) {
		manageEntityCollision(leaderInfo);
		return false;
	}
	int oldPositionIndex = entity->localPositions[0];
	tile->entityIndices[oldPositionIndex] = -1;
	tile->entityIndices[newPosition] = entity->index;
	
	entity->localPositions[0] = newPosition;

	if (entity->hasFollower()) {
		obstructionMaskChanges.push_back(ObstructionMaskChange(tile->index, newMask, tile->index, newMask));
		moveFollowers(entity);
	}
	else {
		obstructionMaskChanges.push_back(ObstructionMaskChange(tile->index, currentMask, tile->index, newMask));
	}

	return true;
}

bool EntityManager::tryMoveLeaderToEdge(LeaderInfo* leaderInfo) {
	
	Entity* entity = &entities[leaderInfo->entityIndex];
	Tile* tile = getTile0(entity);
	Tile* neighborTile = tile->getNeighbor(entity->localDirections[0]);
	LocalPosition newPosition = (LocalPosition)entity->localDirections[0];
	LocalPosition neighborNewPosition = (LocalPosition)tile->sideInfos.connectedSideIndices[entity->localDirections[0]];

	uint16_t currentMask = TileNavigator::getObstructionMask(entity->localPositions[0]);
	uint16_t newMask = TileNavigator::getObstructionMask(newPosition);
	uint16_t neighborNewMask = TileNavigator::getObstructionMask(neighborNewPosition);

	bool obstruction = neighborTile->obstructionMask & neighborNewMask;
	if (obstruction) {
		manageEntityCollision(leaderInfo);
		return false;
	}

	// Move things around:
	tile->entityIndices[entity->localPositions[0]] = -1;
	tile->entityIndices[newPosition] = entity->index;
	tile->entityInfoIndices[newPosition] = 1;

	neighborTile->entityIndices[neighborNewPosition] = entity->index;
	neighborTile->entityInfoIndices[neighborNewPosition] = 0;

	entity->tileIndices[1] = entity->tileIndices[0];
	entity->localPositions[1] = newPosition;
	entity->localDirections[1] = entity->localDirections[0];

	// 0 always has the forward most tile infos in it:
	entity->tileIndices[0] = neighborTile->index;
	entity->localPositions[0] = neighborNewPosition;
	entity->localDirections[0] = LocalDirection((neighborNewPosition + 2) % 4);
	/*entity->localOrientations[0] = TileNavigator::orientationToOrientationMap(tile->type, neighborTile->type,
		entity->localDirections[1], entity->localOrientations[0]);*/

	if (entity->hasFollower()) {
		obstructionMaskChanges.push_back(ObstructionMaskChange(neighborTile->index, neighborNewMask, neighborTile->index, neighborNewMask));
		moveFollowers(entity);
	}
	else {
		obstructionMaskChanges.push_back(ObstructionMaskChange(tile->index, currentMask, tile->index, newMask));
		obstructionMaskChanges.push_back(ObstructionMaskChange(neighborTile->index, neighborNewMask, neighborTile->index, neighborNewMask));
	}

	return true;
}

bool EntityManager::tryMoveLeaderFromEdge(LeaderInfo* leaderInfo) {
	// figure out what tile to move fully to:
	Entity* entity = &entities[leaderInfo->entityIndex];

	Tile* arrivingTile = getTile0(entity);
	Tile* leavingTile = getTile1(entity);

	uint16_t arrivingCurrentMask = TileNavigator::getObstructionMask(entity->localPositions[0]);

	LocalPosition newPosition = LocalPosition(entity->localPositions[0] + 4);
	uint16_t newMask = TileNavigator::getObstructionMask(newPosition);

	bool obstruction = (arrivingTile->obstructionMask & ~arrivingCurrentMask) & newMask;
	if (obstruction) { 
		manageEntityCollision(leaderInfo);
		return false; 
	}

	// Edge->middle position movement has the potential for conflicts.  Conflict check here:
	// TODO: fix this it no work I think
	//if (cornerCollision(entity, arrivingTile, 0, 1)) { return false; }
	//if (cornerCollision(entity, arrivingTile, 0, 3)) { return false; }

	// Move things around:
	leavingTile->entityIndices[entity->localPositions[1]] = -1;
	
	arrivingTile->entityIndices[entity->localPositions[0]] = -1;
	arrivingTile->entityIndices[newPosition] = entity->index;
	arrivingTile->entityInfoIndices[newPosition] = 0;

	entity->localPositions[0] = newPosition;
	
	entity->tileIndices[1] = -1;
	entity->localPositions[1] = LOCAL_POSITION_INVALID;
	entity->localDirections[1] = LOCAL_DIRECTION_INVALID;

	if (entity->hasFollower()) {
		obstructionMaskChanges.push_back(ObstructionMaskChange(arrivingTile->index, newMask, arrivingTile->index, newMask));
		moveFollowers(entity);
	}
	else {
		uint16_t leavingCurrentMask = TileNavigator::getObstructionMask(entity->localPositions[1]);
		obstructionMaskChanges.push_back(ObstructionMaskChange(leavingTile->index, leavingCurrentMask, arrivingTile->index, newMask));
	}

	return true;
}

bool EntityManager::cornerCollision(Entity* entity,Tile* arrivingTile, int arrivingIndex, int edgeIndexOffset) {
	LocalPosition potentialConflictPosition = LocalPosition((entity->localPositions[arrivingIndex] + edgeIndexOffset) % 4);
	int potentialConflictEntityIndex = arrivingTile->entityIndices[potentialConflictPosition];
	bool hasEntity = potentialConflictEntityIndex != -1;
	if (!hasEntity) {
		return false;
	}

	Entity* potentialConflictEntity = &entities[potentialConflictEntityIndex];
	int i = 0;
	if (i != arrivingTile->index) { i = 1; }
	if (potentialConflictEntity->localDirections[i] == ((potentialConflictPosition + 2) % 4)) {
		std::cout << "corner conflict" << std::endl;
		return true;
	}

	return false;
}

bool EntityManager::tryMoveLeader(LeaderInfo* leaderInfo) {
	Entity* entity = &entities[leaderInfos[leaderInfo->leaderIndex].entityIndex];
	if (entity->hasDirection() == false) {
		return false;
	}
	if (entity->inEdgePosition()) { 
		return tryMoveLeaderFromEdge(leaderInfo);
	}
	if (entity->movingToEdge()) { 
		return tryMoveLeaderToEdge(leaderInfo);
	}
	return tryMoveLeaderInterior(leaderInfo);
}

bool EntityManager::tryChangeEntityDirection(LeaderInfo* leaderInfo) {
	Entity* entity = &entities[leaderInfo->entityIndex];
	Tile* tile = getTile0(entity);

	if (!tile->hasForce()) { return false; }
	if (entity->localDirections[0] == tile->forceLocalDirection) { return false; }

	// if you are reversing direciton and have followers, the whole line needs to be reversed!
#define turn180 getTile0(entity)->forceLocalDirection == (entity->localDirections[0] + 2) % 4
	if (entity->hasFollower() && turn180) {
		reverseEntityLineDir(entity);
	}
	else if (false) {// TODO: if the entity is turning 90 degrees, its followers have to be delt with somehow!

	}
	else {
		entity->localDirections[0] = getTile0(entity)->forceLocalDirection;
	}

	// if the direction is changed, it may be that there are new followers to scoop up!
	tryAddFollowerCenter(entity);
	return true;
}

void EntityManager::demoteLeader(int leaderIndex) {
	Entity* entity = &entities[leaderInfos[leaderIndex].entityIndex];
	entity->leaderListIndex = -1;
	if (entity->hasFollower()) {
		Entity* follower = getFollower(entity);
		follower->leaderListIndex = leaderIndex;
		leaderInfos[leaderIndex].entityIndex = follower->index;
		leaderInfos[leaderIndex].direction = follower->localDirections[0];
		leaderInfos[leaderIndex].mass -= 1;
		return;
	}
	if (leaderIndex == leaderInfos.size() - 1) {
		leaderInfos.pop_back();
		return;
	}
	std::swap(leaderInfos[leaderIndex], leaderInfos.back());
	entities[leaderInfos[leaderIndex].entityIndex].leaderListIndex = leaderIndex;
	leaderInfos[leaderIndex].leaderIndex = leaderIndex;
	leaderInfos.pop_back();
}
