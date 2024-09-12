#pragma once

#include <iostream>

#include "tileNavigation.h"
#include "tile.h"

namespace enav {

	struct EntityEditDirectionInfo {
		struct Center {
			int tileIndex;
			LocalPosition position;
			LocalDirection componentToEdit;
		};
		struct Side {
			int tileIndices[2];
			LocalPosition positions[2];
			int alignmentMapIndex;
			LocalDirection componentToEdit;
		};
		struct Corner {
			int tileIndices[4];
			LocalPosition positions[4];
			int alignmentMapIndices[3];
			LocalDirection componentToEdit;
		};
	};

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

	OrthogonalEntityCollisionInfo getEntityInfoCenterToDirect(Tile* tile, LocalDirection direction);
	/* USELESS FOR COLLISIONS! */ OrthogonalEntityCollisionInfo getEntityInfoCenterToOffsetA1(Tile* tile, LocalDirection direction);
	/* USELESS FOR COLLISIONS! */ OrthogonalEntityCollisionInfo getEntityInfoCenterToOffsetB1(Tile* tile, LocalDirection direction);
	/* USELESS FOR COLLISIONS! */ OrthogonalEntityCollisionInfo getEntityInfoCenterToOffsetA2(Tile* tile, LocalDirection direction);
	/* USELESS FOR COLLISIONS! */ OrthogonalEntityCollisionInfo getEntityInfoCenterToOffsetB2(Tile* tile, LocalDirection direction);
	DiagonalEntityCollisionInfo getEntityInfoCenterToCornerA(Tile* tile, LocalDirection direction);
	DiagonalEntityCollisionInfo getEntityInfoCenterToCornerB(Tile* tile, LocalDirection direction);

	// Next entity info from edge is always an 'inner' direction, so we only need to ask for the
	// position, and can infer the direction is the equivlant opposite direction of the position.
	// ,______,   ,______,  ,______,  ,______,
	// |      |   |      |  |      |  ^      |  * = initial direction
	// *>     |  <*      |  *      |  *      |  > = direction of 'next' entity search
	// |______|   |______|  v______|  |______|
	//  VALID     INVALID   INVALID   INVALID

	OrthogonalEntityCollisionInfo getEntityInfoEdgeToDirectA(Tile* tile, LocalPosition position);
	OrthogonalEntityCollisionInfo getEntityInfoEdgeToDirectB(Tile* tile, LocalPosition position);
	OrthogonalEntityCollisionInfo getEntityInfoEdgeToOffsetA1(Tile* tile, LocalPosition position);
	OrthogonalEntityCollisionInfo getEntityInfoEdgeToOffsetA2(Tile* tile, LocalPosition position);
	OrthogonalEntityCollisionInfo getEntityInfoEdgeToOffsetA3(Tile* tile, LocalPosition position);
	OrthogonalEntityCollisionInfo getEntityInfoEdgeToOffsetA4(Tile* tile, LocalPosition position);
	OrthogonalEntityCollisionInfo getEntityInfoEdgeToOffsetB1(Tile* tile, LocalPosition position);
	OrthogonalEntityCollisionInfo getEntityInfoEdgeToOffsetB2(Tile* tile, LocalPosition position);
	OrthogonalEntityCollisionInfo getEntityInfoEdgeToOffsetB3(Tile* tile, LocalPosition position);
	OrthogonalEntityCollisionInfo getEntityInfoEdgeToOffsetB4(Tile* tile, LocalPosition position);
	/* USELESS FOR COLLISIONS! */ DiagonalEntityCollisionInfo getEntityInfoEdgeToCornerA1(Tile* tile, LocalPosition position);
	/* USELESS FOR COLLISIONS! */ DiagonalEntityCollisionInfo getEntityInfoEdgeToCornerA2(Tile* tile, LocalPosition position);
	/* USELESS FOR COLLISIONS! */ DiagonalEntityCollisionInfo getEntityInfoEdgeToCornerB1(Tile* tile, LocalPosition position);
	/* USELESS FOR COLLISIONS! */ DiagonalEntityCollisionInfo getEntityInfoEdgeToCornerB2(Tile* tile, LocalPosition position);

	// Next entity info from corner is always an 'inner' direction, but there are two possible inner directions
	// so the desired direction must be specified!
	//                                ^
	// *>_____,  *______,  <*______,  *______,
	// |      |  v      |   |      |  |      |   * = initial direction
	// |      |  |      |   |      |  |      |  -> = direction of 'next' entity search
	// |______|  |______|   |______|  |______|
	// VALID     VALID      INVALID   INVALID

	OrthogonalEntityCollisionInfo getEntityInfoCornerToDirectA(Tile* tile, LocalPosition position, LocalDirection direction);
	OrthogonalEntityCollisionInfo getEntityInfoCornerToDirectB(Tile* tile, LocalPosition position, LocalDirection direction);
	OrthogonalEntityCollisionInfo getEntityInfoCornerToDirectC(Tile* tile, LocalPosition position, LocalDirection direction);
	OrthogonalEntityCollisionInfo getEntityInfoCornerToDirectD(Tile* tile, LocalPosition position, LocalDirection direction);
	OrthogonalEntityCollisionInfo getEntityInfoCornerToOffsetA1(Tile* tile, LocalPosition position, LocalDirection direction);
	OrthogonalEntityCollisionInfo getEntityInfoCornerToOffsetA2(Tile* tile, LocalPosition position, LocalDirection direction);
	OrthogonalEntityCollisionInfo getEntityInfoCornerToOffsetB1(Tile* tile, LocalPosition position, LocalDirection direction);
	OrthogonalEntityCollisionInfo getEntityInfoCornerToOffsetB2(Tile* tile, LocalPosition position, LocalDirection direction);
	DiagonalEntityCollisionInfo getEntityInfoCornerToCornerA1(Tile* tile, LocalPosition position, LocalDirection direction);
	DiagonalEntityCollisionInfo getEntityInfoCornerToCornerA2(Tile* tile, LocalPosition position, LocalDirection direction);
	DiagonalEntityCollisionInfo getEntityInfoCornerToCornerA3(Tile* tile, LocalPosition position, LocalDirection direction);
	DiagonalEntityCollisionInfo getEntityInfoCornerToCornerA4(Tile* tile, LocalPosition position, LocalDirection direction);
	DiagonalEntityCollisionInfo getEntityInfoCornerToCornerB1(Tile* tile, LocalPosition position, LocalDirection direction);
	DiagonalEntityCollisionInfo getEntityInfoCornerToCornerB2(Tile* tile, LocalPosition position, LocalDirection direction);
	DiagonalEntityCollisionInfo getEntityInfoCornerToCornerB3(Tile* tile, LocalPosition position, LocalDirection direction);
	DiagonalEntityCollisionInfo getEntityInfoCornerToCornerB4(Tile* tile, LocalPosition position, LocalDirection direction);

}