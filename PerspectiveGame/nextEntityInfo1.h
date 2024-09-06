#pragma once

#include <iostream>

#include "tileNavigation.h"
#include "tile.h"

struct NextEntityInfoManager {
private:

	//struct NextEntityInfoFromCenter {
	//	NextEntityInfo direct;
	//	NextEntityInfo offset1[2];
	//	NextEntityInfo offset2[2];
	//	NextEntityInfo corner1[2];
	//	NextEntityInfo corner2[2];
	//};

	//struct NextEntityInfoFromEdge {
	//	NextEntityInfo direct[2];
	//	NextEntityInfo offset1[4];
	//	NextEntityInfo offset2[4];
	//	NextEntityInfo corner1[2];
	//	NextEntityInfo corner2[2];
	//};

	//struct NextEntityInfoFromCorner {
	//	NextEntityInfo directA[4];
	//	NextEntityInfo offsetA1[2];
	//	NextEntityInfo offsetA2[2];
	//	NextEntityInfo cornerA[4];

	//	NextEntityInfo directB[4];
	//	NextEntityInfo offsetB1[2];
	//	NextEntityInfo offsetB2[2];
	//	NextEntityInfo cornerB[4];

	//	NextEntityInfo cornerC[4];
	//};

	//struct CornerEquivelentInfoMap {
	//	int tileIndex[3];
	//	LocalPosition position[3];
	//	LocalDirection direction[3];
	//};

	//struct AllNextEntityInfos {
	//	NextEntityInfoFromCenter fromCenterInfos[4];
	//	NextEntityInfoFromEdge fromEdgeInfos[4];
	//	NextEntityInfoFromCorner fromCornerInfos[4];
	//	CornerEquivelentInfoMap cornerEquivelentInfos[4];
	//};

public:

};

namespace enav {

	struct OrthogonalEntityCollisionInfo {
		int entityIndex;
		int entityInfoIndex;
		LocalDirection equivelantDirection;
		LocalDirection cornerDirectionOfCollision;

		OrthogonalEntityCollisionInfo()
		{
			entityIndex = -1;
			entityInfoIndex = -1;
			equivelantDirection = LOCAL_DIRECTION_ERROR;
		}
		OrthogonalEntityCollisionInfo(int entityIndex, int entityInfoIndex, LocalDirection equivelantDirection) :
			entityIndex(entityIndex), entityInfoIndex(entityInfoIndex), equivelantDirection(equivelantDirection)
		{}
	};
	struct DiagonalEntityCollisionInfo {
		int entityIndex;
		int entityInfoIndex;
		LocalDirection equivelantDirection;
		LocalDirection cornerDirectionOfCollision;

		DiagonalEntityCollisionInfo()
		{
			entityIndex = -1;
			entityInfoIndex = -1;
			equivelantDirection = LOCAL_DIRECTION_ERROR;
			cornerDirectionOfCollision = LOCAL_DIRECTION_ERROR;
		}
		DiagonalEntityCollisionInfo(int entityIndex, int entityInfoIndex, LocalDirection equivelantDirection, LocalDirection cornerDirectionOfCollision) :
			entityIndex(entityIndex), entityInfoIndex(entityInfoIndex), equivelantDirection(equivelantDirection), cornerDirectionOfCollision(cornerDirectionOfCollision)
		{}
	};

	// 'A' designates an offset direction of (n + 1) % 4 of the chosen direction.
	// 'B' designates an offset direction of (n + 3) % 4 of the chosen direction.
	// A'n' / B'n' designate equivelent tile positions/directions for the next entity.
	//     NOTE: Smaller n is easier to calculate.

	OrthogonalEntityCollisionInfo getNextEntityInfoFromCenterToDirect(Tile* tile, LocalDirection direction);
	/* USELESS FOR COLLISIONS! */ OrthogonalEntityCollisionInfo getNextEntityInfoFromCenterToOffsetA1(Tile* tile, LocalDirection direction);
	/* USELESS FOR COLLISIONS! */ OrthogonalEntityCollisionInfo getNextEntityInfoFromCenterToOffsetB1(Tile* tile, LocalDirection direction);
	/* USELESS FOR COLLISIONS! */ OrthogonalEntityCollisionInfo getNextEntityInfoFromCenterToOffsetA2(Tile* tile, LocalDirection direction);
	/* USELESS FOR COLLISIONS! */ OrthogonalEntityCollisionInfo getNextEntityInfoFromCenterToOffsetB2(Tile* tile, LocalDirection direction);
	DiagonalEntityCollisionInfo getNextEntityInfoFromCenterToCornerA(Tile* tile, LocalDirection direction);
	DiagonalEntityCollisionInfo getNextEntityInfoFromCenterToCornerB(Tile* tile, LocalDirection direction);

	// Next entity info from edge is always an 'inner' direction, so we only need to ask for the
	// position, and can infer the direction is the equivlant opposite direction of the position.
	// ,______,   ,______,  ,______,  ,______,
	// |      |   |      |  |      |  ^      |  * = initial direction
	// *>     |  <*      |  *      |  *      |  > = direction of 'next' entity search
	// |______|   |______|  v______|  |______|
	//  VALID     INVALID   INVALID   INVALID

	OrthogonalEntityCollisionInfo getNextEntityInfoFromEdgeToDirectA(Tile* tile, LocalPosition position);
	OrthogonalEntityCollisionInfo getNextEntityInfoFromEdgeToDirectB(Tile* tile, LocalPosition position);
	OrthogonalEntityCollisionInfo getNextEntityInfoFromEdgeToOffsetA1(Tile* tile, LocalPosition position);
	OrthogonalEntityCollisionInfo getNextEntityInfoFromEdgeToOffsetA2(Tile* tile, LocalPosition position);
	OrthogonalEntityCollisionInfo getNextEntityInfoFromEdgeToOffsetA3(Tile* tile, LocalPosition position);
	OrthogonalEntityCollisionInfo getNextEntityInfoFromEdgeToOffsetA4(Tile* tile, LocalPosition position);
	OrthogonalEntityCollisionInfo getNextEntityInfoFromEdgeToOffsetB1(Tile* tile, LocalPosition position);
	OrthogonalEntityCollisionInfo getNextEntityInfoFromEdgeToOffsetB2(Tile* tile, LocalPosition position);
	OrthogonalEntityCollisionInfo getNextEntityInfoFromEdgeToOffsetB3(Tile* tile, LocalPosition position);
	OrthogonalEntityCollisionInfo getNextEntityInfoFromEdgeToOffsetB4(Tile* tile, LocalPosition position);
	/* USELESS FOR COLLISIONS! */ DiagonalEntityCollisionInfo getNextEntityInfoFromEdgeToCornerA1(Tile* tile, LocalPosition position);
	/* USELESS FOR COLLISIONS! */ DiagonalEntityCollisionInfo getNextEntityInfoFromEdgeToCornerA2(Tile* tile, LocalPosition position);
	/* USELESS FOR COLLISIONS! */ DiagonalEntityCollisionInfo getNextEntityInfoFromEdgeToCornerB1(Tile* tile, LocalPosition position);
	/* USELESS FOR COLLISIONS! */ DiagonalEntityCollisionInfo getNextEntityInfoFromEdgeToCornerB2(Tile* tile, LocalPosition position);

	// Next entity info from corner is always an 'inner' direction, but there are two possible inner directions
	// so the desired direction must be specified!
	//                                ^
	// *>_____,  *______,  <*______,  *______,
	// |      |  v      |   |      |  |      |   * = initial direction
	// |      |  |      |   |      |  |      |  -> = direction of 'next' entity search
	// |______|  |______|   |______|  |______|
	// VALID     VALID      INVALID   INVALID

	OrthogonalEntityCollisionInfo getNextEntityInfoFromCornerToDirectA(Tile* tile, LocalPosition position, LocalDirection direction);
	OrthogonalEntityCollisionInfo getNextEntityInfoFromCornerToDirectB(Tile* tile, LocalPosition position, LocalDirection direction);
	OrthogonalEntityCollisionInfo getNextEntityInfoFromCornerToDirectC(Tile* tile, LocalPosition position, LocalDirection direction);
	OrthogonalEntityCollisionInfo getNextEntityInfoFromCornerToDirectD(Tile* tile, LocalPosition position, LocalDirection direction);
	OrthogonalEntityCollisionInfo getNextEntityInfoFromCornerToOffsetA1(Tile* tile, LocalPosition position, LocalDirection direction);
	OrthogonalEntityCollisionInfo getNextEntityInfoFromCornerToOffsetA2(Tile* tile, LocalPosition position, LocalDirection direction);
	OrthogonalEntityCollisionInfo getNextEntityInfoFromCornerToOffsetB1(Tile* tile, LocalPosition position, LocalDirection direction);
	OrthogonalEntityCollisionInfo getNextEntityInfoFromCornerToOffsetB2(Tile* tile, LocalPosition position, LocalDirection direction);
	DiagonalEntityCollisionInfo getNextEntityInfoFromCornerToCornerA1(Tile* tile, LocalPosition position, LocalDirection direction);
	DiagonalEntityCollisionInfo getNextEntityInfoFromCornerToCornerA2(Tile* tile, LocalPosition position, LocalDirection direction);
	DiagonalEntityCollisionInfo getNextEntityInfoFromCornerToCornerA3(Tile* tile, LocalPosition position, LocalDirection direction);
	DiagonalEntityCollisionInfo getNextEntityInfoFromCornerToCornerA4(Tile* tile, LocalPosition position, LocalDirection direction);
	DiagonalEntityCollisionInfo getNextEntityInfoFromCornerToCornerB1(Tile* tile, LocalPosition position, LocalDirection direction);
	DiagonalEntityCollisionInfo getNextEntityInfoFromCornerToCornerB2(Tile* tile, LocalPosition position, LocalDirection direction);
	DiagonalEntityCollisionInfo getNextEntityInfoFromCornerToCornerB3(Tile* tile, LocalPosition position, LocalDirection direction);
	DiagonalEntityCollisionInfo getNextEntityInfoFromCornerToCornerB4(Tile* tile, LocalPosition position, LocalDirection direction);

}