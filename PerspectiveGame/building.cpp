//#include "building.h"
//#include "tile.h"
//
//Building::Building(Building::Side orientation, Building::Type buildingType, Tile *parentTile) :
//	orientation(orientation), buildingType(buildingType), parentTile(parentTile) {
//	if (parentTile->building.buildingType != Building::Type::NONE) {
//		std::cout << "TRYING TO OVERWRITE BUILDING WITH OTHER BULIDING!" << std::endl;
//		return;
//	}
//}
//
//void Producer::update() {
//	if (cooldown > 0.0f) {
//		cooldown -= 1.0f * DeltaTime;
//		return;
//	}
//	if (entity.type == ENTITY_TYPE_NONE) {
//		entity.type = producedEntityType;
//		entity.offset = 0.0f;
//		return;
//	}
//	if (connectedBuilding != nullptr && connectedBuilding->entity.type == ENTITY_TYPE_NONE) {
//
//		connectedBuilding->entity.type = entity.type;
//		connectedBuilding->entity.offset = 1.0f;
//		connectedBuilding->entity.offsetSide = parentTile->sideInfos[orientation].connection.sideIndex;
//
//		entity.type = ENTITY_TYPE_NONE;
//		cooldown = 1.0f;
//	}
//}
//
//void Consumer::updateConnections() {
//	for (int i = 0; i < 4; i++) {
//		Building *buildingPtr = &(parentTile->sideInfos[i].connection.tile->building);
//		EntityBuilding *cast = dynamic_cast<EntityBuilding *>(buildingPtr);
//		if (cast != nullptr) {
//			connectedBuilding[i] = cast;
//		} else {
//			connectedBuilding[i] = nullptr;
//		}
//	}
//}