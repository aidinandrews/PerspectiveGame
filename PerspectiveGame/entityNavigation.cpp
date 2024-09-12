#include "entityNavigation.h"

// 'out' is the direction component that leads away from the tile, not along its edge.
LocalDirection cornerOutDirection(LocalPosition position, LocalDirection direction)
{
	const LocalPosition* components = tnav::getAlignmentComponents(position);
	bool i = (components[0] == tnav::oppositeAlignment(direction)) ? 1 : 0;
	return components[i];
}

enav::OrthogonalEntityCollisionInfo getEntityInfoCenterOffset1(Tile* tile, LocalDirection direction, int offset)
{
	Tile* neighbor = tile->getNeighbor(direction);
	LocalDirection outDirection = LocalDirection((direction + offset) % 4);
	LocalDirection neighborDirection = tile->mapAlignmentTo1stDegreeNeighbor(direction, direction);
	LocalPosition nextEntityPos = tile->mapAlignmentTo1stDegreeNeighbor(direction, outDirection);

	return enav::OrthogonalEntityCollisionInfo(neighbor->entityIndices[nextEntityPos],
								neighbor->entityInfoIndices[nextEntityPos],
								neighborDirection);
}
enav::OrthogonalEntityCollisionInfo getEntityInfoCenterOffset2(Tile* tile, LocalDirection direction, int offset)
{
	LocalDirection outDirection = LocalDirection((direction + offset) % 4);

	Tile* neighbor = tile->getNeighbor(outDirection);
	LocalDirection neighborDirection = tile->mapAlignmentTo1stDegreeNeighbor(outDirection, direction);

	Tile* neighborNeighbor = neighbor->getNeighbor(neighborDirection);
	LocalDirection neighborNeighborDirection = neighbor->mapAlignmentTo1stDegreeNeighbor(neighborDirection, neighborDirection);

	LocalPosition nextEntityPos = tnav::oppositeAlignment(LocalPosition(outDirection));
	nextEntityPos = tile->mapAlignmentTo1stDegreeNeighbor(outDirection, nextEntityPos);
	nextEntityPos = neighbor->mapAlignmentTo1stDegreeNeighbor(neighborDirection, nextEntityPos);

	return enav::OrthogonalEntityCollisionInfo(neighborNeighbor->entityIndices[nextEntityPos],
								neighborNeighbor->entityInfoIndices[nextEntityPos],
								neighborNeighborDirection);
}
enav::DiagonalEntityCollisionInfo getEntityInfoCenterCorner(Tile* tile, LocalDirection direction, int offset)
{
	LocalDirection outDirection = LocalDirection((direction + offset) % 4);

	Tile* neighbor = tile->getNeighbor(outDirection);
	LocalDirection neighborDirection = tile->mapAlignmentTo1stDegreeNeighbor(outDirection, direction);
	
	Tile* neighborNeighbor = neighbor->getNeighbor(neighborDirection);
	LocalDirection neighborNeighborDirection = neighbor->mapAlignmentTo1stDegreeNeighbor(neighborDirection, neighborDirection);

	LocalDirection collisionDirection = tnav::oppositeAlignment(outDirection);
	collisionDirection = tile->mapAlignmentTo1stDegreeNeighbor(outDirection, collisionDirection);
	collisionDirection = neighbor->mapAlignmentTo1stDegreeNeighbor(neighborDirection, collisionDirection);

	return enav::DiagonalEntityCollisionInfo(neighborNeighbor->entityIndices[LOCAL_POSITION_CENTER],
											 neighborNeighbor->entityInfoIndices[LOCAL_POSITION_CENTER],
											 neighborNeighborDirection, collisionDirection);
}

enav::OrthogonalEntityCollisionInfo getEntityInfoEdgeOffset1(Tile* tile, LocalPosition position, int offset)
{
	LocalDirection direction = (LocalDirection)tnav::oppositeAlignment(position);
	LocalDirection outDirection = LocalDirection((direction + offset) % 4);
	LocalPosition nextEntityPos = tnav::combineAlignments(LocalPosition(direction), LocalPosition(outDirection));

	return enav::OrthogonalEntityCollisionInfo(tile->entityIndices[nextEntityPos],
								tile->entityInfoIndices[nextEntityPos],
								direction);
}
enav::OrthogonalEntityCollisionInfo getEntityInfoEdgeOffset2(Tile* tile, LocalPosition position, int offset)
{
	LocalDirection direction = (LocalDirection)tnav::oppositeAlignment(position);
	LocalDirection outDirection = LocalDirection((direction + offset) % 4);

	Tile* neighbor = tile->getNeighbor(direction);
	LocalDirection neighborDirection = tile->mapAlignmentTo1stDegreeNeighbor(direction, direction);

	LocalPosition nextEntityPos = tnav::combineAlignments(position, LocalPosition(outDirection));
	nextEntityPos = tile->mapAlignmentTo1stDegreeNeighbor(direction, nextEntityPos);

	return enav::OrthogonalEntityCollisionInfo(neighbor->entityIndices[nextEntityPos],
								neighbor->entityInfoIndices[nextEntityPos],
								neighborDirection);
}
enav::OrthogonalEntityCollisionInfo getEntityInfoEdgeOffset3(Tile* tile, LocalPosition position, int offset)
{
	LocalDirection direction = (LocalDirection)tnav::oppositeAlignment(position);
	LocalDirection outDirection = LocalDirection((direction + offset) % 4);

	Tile* neighbor = tile->getNeighbor(outDirection);
	LocalDirection neighborDirection = tile->mapAlignmentTo1stDegreeNeighbor(outDirection, direction);

	Tile* neighborNeighbor = neighbor->getNeighbor(neighborDirection);
	LocalDirection neighborNeighborDirection = neighbor->mapAlignmentTo1stDegreeNeighbor(neighborDirection, neighborDirection);

	LocalPosition nextEntityPos = tnav::combineAlignments(position, LocalPosition(tnav::oppositeAlignment(outDirection)));
	nextEntityPos = tile->mapAlignmentTo1stDegreeNeighbor(outDirection, nextEntityPos);
	nextEntityPos = neighbor->mapAlignmentTo1stDegreeNeighbor(neighborDirection, nextEntityPos);

	return enav::OrthogonalEntityCollisionInfo(neighborNeighbor->entityIndices[nextEntityPos],
								neighborNeighbor->entityInfoIndices[nextEntityPos],
								neighborNeighborDirection);
}
enav::OrthogonalEntityCollisionInfo getEntityInfoEdgeOffset4(Tile* tile, LocalPosition position, int offset)
{
	LocalDirection direction = (LocalDirection)tnav::oppositeAlignment(position);
	LocalDirection outDirection = LocalDirection((direction + offset) % 4);
	Tile* neighbor = tile->getNeighbor(outDirection);
	LocalDirection neighborDirection = tile->mapAlignmentTo1stDegreeNeighbor(outDirection, direction);
	LocalPosition nextEntityPos = tnav::combineAlignments(LocalPosition(direction), LocalPosition(tnav::oppositeAlignment(outDirection)));
	nextEntityPos = tile->mapAlignmentTo1stDegreeNeighbor(outDirection, nextEntityPos);

	return enav::OrthogonalEntityCollisionInfo(neighbor->entityIndices[nextEntityPos],
								neighbor->entityInfoIndices[nextEntityPos],
								neighborDirection);
}
enav::DiagonalEntityCollisionInfo getEntityInfoEdgeCorner1(Tile* tile, LocalPosition position, int offset)
{
	LocalDirection direction = (LocalDirection)tnav::oppositeAlignment(position);
	LocalDirection outDirection = LocalDirection((direction + offset) % 4);
	
	Tile* neighbor = tile->getNeighbor(outDirection);
	LocalDirection neighborDirection = tile->mapAlignmentTo1stDegreeNeighbor(outDirection, direction);
	LocalPosition nextEntityPos = tile->mapAlignmentTo1stDegreeNeighbor(outDirection, LocalPosition(direction));

	LocalDirection collisionDirection = tnav::oppositeAlignment(outDirection);
	collisionDirection = tile->mapAlignmentTo1stDegreeNeighbor(outDirection, collisionDirection);

	return enav::DiagonalEntityCollisionInfo(neighbor->entityIndices[nextEntityPos],
											 neighbor->entityInfoIndices[nextEntityPos],
											 neighborDirection, collisionDirection);
}
enav::DiagonalEntityCollisionInfo getEntityInfoEdgeCorner2(Tile* tile, LocalPosition position, int offset)
{
	LocalDirection direction = (LocalDirection)tnav::oppositeAlignment(position);
	LocalDirection outDirection = LocalDirection((direction + offset) % 4);

	Tile* neighbor = tile->getNeighbor(outDirection);
	LocalDirection neighborDirection = tile->mapAlignmentTo1stDegreeNeighbor(outDirection, direction);

	Tile* neighborNeighbor = neighbor->getNeighbor(neighborDirection);
	LocalDirection neighborNeighborDirection = neighbor->mapAlignmentTo1stDegreeNeighbor(neighborDirection, neighborDirection);

	LocalPosition nextEntityPos = tile->mapAlignmentTo1stDegreeNeighbor(outDirection, position);
	nextEntityPos = neighbor->mapAlignmentTo1stDegreeNeighbor(neighborDirection, nextEntityPos);

	LocalDirection collisionDirection = tnav::oppositeAlignment(outDirection);
	collisionDirection = tile->mapAlignmentTo1stDegreeNeighbor(outDirection, collisionDirection);
	collisionDirection = neighbor->mapAlignmentTo1stDegreeNeighbor(neighborDirection, collisionDirection);

	return enav::DiagonalEntityCollisionInfo(neighbor->entityIndices[nextEntityPos],
								neighbor->entityInfoIndices[nextEntityPos],
								neighborNeighborDirection,
											   collisionDirection);
}

enav::OrthogonalEntityCollisionInfo enav::getEntityInfoCenterToDirect(Tile* tile, LocalDirection direction)
{
	Tile* neighbor = tile->getNeighbor(direction);
	return enav::OrthogonalEntityCollisionInfo(neighbor->entityIndices[LOCAL_POSITION_CENTER],
								neighbor->entityInfoIndices[LOCAL_POSITION_CENTER],
								tile->mapAlignmentTo1stDegreeNeighbor(direction, direction));
}
enav::OrthogonalEntityCollisionInfo enav::getEntityInfoCenterToOffsetA1(Tile* tile, LocalDirection direction)
{
	return getEntityInfoCenterOffset1(tile, direction, 1);
}
enav::OrthogonalEntityCollisionInfo enav::getEntityInfoCenterToOffsetB1(Tile* tile, LocalDirection direction)
{
	return getEntityInfoCenterOffset1(tile, direction, 3);
}
enav::OrthogonalEntityCollisionInfo enav::getEntityInfoCenterToOffsetA2(Tile* tile, LocalDirection direction)
{
	return getEntityInfoCenterOffset2(tile, direction, 1);
}
enav::OrthogonalEntityCollisionInfo enav::getEntityInfoCenterToOffsetB2(Tile* tile, LocalDirection direction)
{
	return getEntityInfoCenterOffset2(tile, direction, 3);
}
enav::DiagonalEntityCollisionInfo enav::getEntityInfoCenterToCornerA(Tile* tile, LocalDirection direction)
{
	return getEntityInfoCenterCorner(tile, direction, 1);
}
enav::DiagonalEntityCollisionInfo enav::getEntityInfoCenterToCornerB(Tile* tile, LocalDirection direction)
{
	return getEntityInfoCenterCorner(tile, direction, 3);
}

enav::OrthogonalEntityCollisionInfo enav::getEntityInfoEdgeToDirectA(Tile* tile, LocalPosition position)
{
	LocalPosition oppositePos = tnav::oppositeAlignment(position);
	return enav::OrthogonalEntityCollisionInfo(tile->entityIndices[oppositePos],
								tile->entityInfoIndices[oppositePos],
								LocalDirection(oppositePos));
}
enav::OrthogonalEntityCollisionInfo enav::getEntityInfoEdgeToDirectB(Tile* tile, LocalPosition position)
{
	LocalDirection direction = (LocalDirection)tnav::oppositeAlignment(position);
	Tile* neighbor = tile->getNeighbor(direction);
	LocalDirection neighborDirection = tile->mapAlignmentTo1stDegreeNeighbor(direction, direction);
	LocalDirection nextEntityPos = tile->mapAlignmentTo1stDegreeNeighbor(direction, position);

	return enav::OrthogonalEntityCollisionInfo(neighbor->entityIndices[nextEntityPos],
								neighbor->entityInfoIndices[nextEntityPos],
								neighborDirection);
}
enav::OrthogonalEntityCollisionInfo enav::getEntityInfoEdgeToOffsetA1(Tile* tile, LocalPosition position)
{
	return getEntityInfoEdgeOffset1(tile, position, 1);
}
enav::OrthogonalEntityCollisionInfo enav::getEntityInfoEdgeToOffsetA2(Tile* tile, LocalPosition position)
{
	return getEntityInfoEdgeOffset2(tile, position, 1);
}
enav::OrthogonalEntityCollisionInfo enav::getEntityInfoEdgeToOffsetA3(Tile* tile, LocalPosition position)
{
	return getEntityInfoEdgeOffset3(tile, position, 1);
}
enav::OrthogonalEntityCollisionInfo enav::getEntityInfoEdgeToOffsetA4(Tile* tile, LocalPosition position)
{
	return getEntityInfoEdgeOffset4(tile, position, 1);
}
enav::OrthogonalEntityCollisionInfo enav::getEntityInfoEdgeToOffsetB1(Tile* tile, LocalPosition position)
{
	return getEntityInfoEdgeOffset1(tile, position, 3);
}
enav::OrthogonalEntityCollisionInfo enav::getEntityInfoEdgeToOffsetB2(Tile* tile, LocalPosition position)
{
	return getEntityInfoEdgeOffset2(tile, position, 3);
}
enav::OrthogonalEntityCollisionInfo enav::getEntityInfoEdgeToOffsetB3(Tile* tile, LocalPosition position)
{
	return getEntityInfoEdgeOffset3(tile, position, 3);
}
enav::OrthogonalEntityCollisionInfo enav::getEntityInfoEdgeToOffsetB4(Tile* tile, LocalPosition position)
{
	return getEntityInfoEdgeOffset4(tile, position, 3);
}
enav::DiagonalEntityCollisionInfo  enav::getEntityInfoEdgeToCornerA1(Tile* tile, LocalPosition position)
{
	return getEntityInfoEdgeCorner1(tile, position, 1);
}
enav::DiagonalEntityCollisionInfo  enav::getEntityInfoEdgeToCornerA2(Tile* tile, LocalPosition position)
{
	return getEntityInfoEdgeCorner2(tile, position, 1);
}
enav::DiagonalEntityCollisionInfo  enav::getEntityInfoEdgeToCornerB1(Tile* tile, LocalPosition position)
{
	return getEntityInfoEdgeCorner1(tile, position, 3);
}
enav::DiagonalEntityCollisionInfo  enav::getEntityInfoEdgeToCornerB2(Tile* tile, LocalPosition position)
{
	return getEntityInfoEdgeCorner2(tile, position, 3);
}

enav::OrthogonalEntityCollisionInfo enav::getEntityInfoCornerToDirectA(Tile* tile, LocalPosition position, LocalDirection direction)
{
	LocalPosition nextEntityPos = tnav::nextPosition(tnav::nextPosition(position, direction), direction);
	return OrthogonalEntityCollisionInfo(tile->entityIndices[nextEntityPos],
						  tile->entityInfoIndices[nextEntityPos],
						  direction);
}
enav::OrthogonalEntityCollisionInfo enav::getEntityInfoCornerToDirectB(Tile* tile, LocalPosition position, LocalDirection direction)
{
	Tile* neighbor = tile->getNeighbor(direction);
	LocalDirection neighborDirection = tile->mapAlignmentTo1stDegreeNeighbor(direction, direction);

	LocalPosition nextEntityPos = tile->mapAlignmentTo1stDegreeNeighbor(direction, position);

	return OrthogonalEntityCollisionInfo(neighbor->entityIndices[nextEntityPos],
						  neighbor->entityInfoIndices[nextEntityPos],
						  neighborDirection);
}
enav::OrthogonalEntityCollisionInfo enav::getEntityInfoCornerToDirectC(Tile* tile, LocalPosition position, LocalDirection direction)
{
	const LocalPosition* posComps = tnav::getAlignmentComponents(position);
	bool matchPosCompI = ((LocalDirection)posComps[0] != direction) ? 0 : 1;
	LocalDirection outDirection = tnav::oppositeAlignment(LocalDirection(posComps[!matchPosCompI]));

	Tile* neighbor = tile->getNeighbor(outDirection);
	LocalDirection neighborDirection = tile->mapAlignmentTo1stDegreeNeighbor(outDirection, direction);

	LocalPosition nextEntityPos = tile->mapAlignmentTo1stDegreeNeighbor(direction, tnav::oppositeAlignment(position));

	return OrthogonalEntityCollisionInfo(neighbor->entityIndices[nextEntityPos],
						  neighbor->entityInfoIndices[nextEntityPos],
						  neighborDirection);
}
enav::OrthogonalEntityCollisionInfo enav::getEntityInfoCornerToDirectD(Tile* tile, LocalPosition position, LocalDirection direction)
{
	LocalDirection outDirection = cornerOutDirection(position, direction);

	Tile* neighbor = tile->getNeighbor(outDirection);
	LocalDirection neighborDirection = tile->mapAlignmentTo1stDegreeNeighbor(outDirection, direction);

	Tile* neighborNeighbor = neighbor->getNeighbor(neighborDirection);
	LocalDirection neighborNeighborDirection = neighbor->mapAlignmentTo1stDegreeNeighbor(neighborDirection, neighborDirection);

	LocalPosition nextEntityPos = tnav::combineAlignments((LocalPosition)tnav::oppositeAlignment(direction), (LocalPosition)tnav::oppositeAlignment(outDirection));
	nextEntityPos = tile->mapAlignmentTo1stDegreeNeighbor(outDirection, nextEntityPos);
	nextEntityPos = neighbor->mapAlignmentTo1stDegreeNeighbor(neighborDirection, nextEntityPos);

	return OrthogonalEntityCollisionInfo(neighborNeighbor->entityIndices[nextEntityPos],
						  neighborNeighbor->entityInfoIndices[nextEntityPos],
						  neighborNeighborDirection);
}
enav::OrthogonalEntityCollisionInfo enav::getEntityInfoCornerToOffsetA1(Tile* tile, LocalPosition position, LocalDirection direction)
{
	LocalDirection outDirection = cornerOutDirection(position, direction);

	Tile* neighbor = tile->getNeighbor(outDirection);
	LocalDirection neighborDirection = tile->mapAlignmentTo1stDegreeNeighbor(outDirection, direction);

	LocalPosition nextEntityPos = tile->mapAlignmentTo1stDegreeNeighbor(outDirection, LocalPosition(direction));

	return OrthogonalEntityCollisionInfo(neighbor->entityIndices[nextEntityPos],
						  neighbor->entityInfoIndices[nextEntityPos],
						  neighborDirection);
}
enav::OrthogonalEntityCollisionInfo enav::getEntityInfoCornerToOffsetA2(Tile* tile, LocalPosition position, LocalDirection direction)
{
	LocalDirection outDirection = cornerOutDirection(position, direction);

	Tile* neighbor = tile->getNeighbor(outDirection);
	LocalDirection neighborDirection = tile->mapAlignmentTo1stDegreeNeighbor(outDirection, direction);

	Tile* neighborNeighbor = neighbor->getNeighbor(neighborDirection);
	LocalDirection neighborNeighborDirection = neighbor->mapAlignmentTo1stDegreeNeighbor(neighborDirection, neighborDirection);

	LocalPosition nextEntityPos = tile->mapAlignmentTo1stDegreeNeighbor(outDirection, tnav::oppositeAlignment(LocalPosition(direction)));
	nextEntityPos = neighbor->mapAlignmentTo1stDegreeNeighbor(neighborDirection, nextEntityPos);

	return OrthogonalEntityCollisionInfo(neighborNeighbor->entityIndices[nextEntityPos],
						  neighborNeighbor->entityInfoIndices[nextEntityPos],
						  neighborNeighborDirection);
}
enav::OrthogonalEntityCollisionInfo enav::getEntityInfoCornerToOffsetB1(Tile* tile, LocalPosition position, LocalDirection direction)
{
	return OrthogonalEntityCollisionInfo(tile->entityIndices[LocalPosition(direction)],
						  tile->entityInfoIndices[LocalPosition(direction)],
						  direction);
}
enav::OrthogonalEntityCollisionInfo enav::getEntityInfoCornerToOffsetB2(Tile* tile, LocalPosition position, LocalDirection direction)
{
	Tile* neighbor = tile->getNeighbor(direction);
	LocalDirection neighborDirection = tile->mapAlignmentTo1stDegreeNeighbor(direction, direction);

	LocalPosition nextEntityPos = tnav::oppositeAlignment(LocalPosition(direction));
	nextEntityPos = tile->mapAlignmentTo1stDegreeNeighbor(direction, nextEntityPos);

	return OrthogonalEntityCollisionInfo(neighbor->entityIndices[nextEntityPos],
						  neighbor->entityInfoIndices[nextEntityPos],
						  neighborDirection);
}
enav::DiagonalEntityCollisionInfo enav::getEntityInfoCornerToCornerA1(Tile* tile, LocalPosition position, LocalDirection direction)
{
	LocalDirection outDirection = cornerOutDirection(position, direction);

	Tile* neighbor = tile->getNeighbor(outDirection);
	LocalDirection neighborDirection = tile->mapAlignmentTo1stDegreeNeighbor(outDirection, direction);

	LocalPosition nextEntityPos = tnav::nextPosition(tnav::nextPosition(position, direction), direction);
	nextEntityPos = tile->mapAlignmentTo1stDegreeNeighbor(outDirection, LocalPosition(direction));

	LocalDirection collisionDirection = tnav::oppositeAlignment(outDirection);
	collisionDirection = tile->mapAlignmentTo1stDegreeNeighbor(outDirection, collisionDirection);

	return DiagonalEntityCollisionInfo(neighbor->entityIndices[nextEntityPos],
						  neighbor->entityInfoIndices[nextEntityPos],
						  neighborDirection, collisionDirection);
}
enav::DiagonalEntityCollisionInfo enav::getEntityInfoCornerToCornerA2(Tile* tile, LocalPosition position, LocalDirection direction)
{
	LocalDirection outDirection = cornerOutDirection(position, direction);

	Tile* neighbor = tile->getNeighbor(outDirection);
	LocalDirection neighborDirection = tile->mapAlignmentTo1stDegreeNeighbor(outDirection, direction);

	Tile* neighborNeighbor = neighbor->getNeighbor(neighborDirection);
	LocalDirection neighborNeighborDirection = neighbor->mapAlignmentTo1stDegreeNeighbor(neighborDirection, neighborDirection);

	LocalPosition nextEntityPos = tile->mapAlignmentTo1stDegreeNeighbor(outDirection, position);
	nextEntityPos = neighbor->mapAlignmentTo1stDegreeNeighbor(neighborDirection, nextEntityPos);

	LocalDirection collisionDirection = tnav::oppositeAlignment(outDirection);
	collisionDirection = tile->mapAlignmentTo1stDegreeNeighbor(outDirection, collisionDirection);
	collisionDirection = neighbor->mapAlignmentTo1stDegreeNeighbor(neighborDirection, collisionDirection);

	return DiagonalEntityCollisionInfo(neighborNeighbor->entityIndices[nextEntityPos],
						  neighborNeighbor->entityInfoIndices[nextEntityPos],
						  neighborNeighborDirection, collisionDirection);
}
enav::DiagonalEntityCollisionInfo enav::getEntityInfoCornerToCornerA3(Tile* tile, LocalPosition position, LocalDirection direction)
{
	LocalDirection outDirection = cornerOutDirection(position, direction);

	Tile* neighbor = tile->getNeighbor(outDirection);
	LocalDirection neighborDirection = tile->mapAlignmentTo1stDegreeNeighbor(outDirection, direction);
	LocalDirection neighborOutDirection = tile->mapAlignmentTo1stDegreeNeighbor(outDirection, outDirection);

	Tile* neighborNeighbor = neighbor->getNeighbor(neighborOutDirection);
	LocalDirection neighborNeighborDirection = neighbor->mapAlignmentTo1stDegreeNeighbor(neighborOutDirection, neighborDirection);

	LocalPosition nextEntityPos = tnav::oppositeAlignment(position);
	nextEntityPos = tile->mapAlignmentTo1stDegreeNeighbor(outDirection, nextEntityPos);
	nextEntityPos = neighbor->mapAlignmentTo1stDegreeNeighbor(neighborOutDirection, nextEntityPos);

	LocalDirection collisionDirection = tnav::oppositeAlignment(outDirection);
	collisionDirection = tile->mapAlignmentTo1stDegreeNeighbor(outDirection, collisionDirection);
	collisionDirection = neighbor->mapAlignmentTo1stDegreeNeighbor(neighborDirection, collisionDirection);

	return DiagonalEntityCollisionInfo(neighborNeighbor->entityIndices[nextEntityPos],
						  neighborNeighbor->entityInfoIndices[nextEntityPos],
						  neighborNeighborDirection, collisionDirection);
}
enav::DiagonalEntityCollisionInfo enav::getEntityInfoCornerToCornerA4(Tile* tile, LocalPosition position, LocalDirection direction)
{
	LocalDirection outDirection = cornerOutDirection(position, direction);

	Tile* neighbor = tile->getNeighbor(outDirection);
	LocalDirection neighborDirection = tile->mapAlignmentTo1stDegreeNeighbor(outDirection, direction);
	LocalDirection neighborOutDirection = tile->mapAlignmentTo1stDegreeNeighbor(outDirection, outDirection);

	Tile* neighborNeighbor = neighbor->getNeighbor(neighborOutDirection);
	LocalDirection neighborNeighborDirection = neighbor->mapAlignmentTo1stDegreeNeighbor(neighborOutDirection, neighborDirection);

	Tile* neighborNeighborNeighbor = neighborNeighbor->getNeighbor(neighborNeighborDirection);
	LocalDirection neighborNeighborNeighborDirection = neighborNeighbor->mapAlignmentTo1stDegreeNeighbor(neighborNeighborDirection, neighborNeighborDirection);

	LocalPosition nextEntityPos = tnav::combineAlignments(tnav::oppositeAlignment(LocalPosition(direction)), tnav::oppositeAlignment(LocalPosition(outDirection)));
	nextEntityPos = tile->mapAlignmentTo1stDegreeNeighbor(outDirection, nextEntityPos);
	nextEntityPos = neighbor->mapAlignmentTo1stDegreeNeighbor(neighborOutDirection, nextEntityPos);
	nextEntityPos = neighborNeighbor->mapAlignmentTo1stDegreeNeighbor(neighborNeighborNeighborDirection, nextEntityPos);

	LocalDirection collisionDirection = tnav::oppositeAlignment(outDirection);
	collisionDirection = tile->mapAlignmentTo1stDegreeNeighbor(outDirection, collisionDirection);
	collisionDirection = neighbor->mapAlignmentTo1stDegreeNeighbor(neighborDirection, collisionDirection);
	collisionDirection = neighborNeighbor->mapAlignmentTo1stDegreeNeighbor(neighborNeighborDirection, collisionDirection);

	return DiagonalEntityCollisionInfo(neighborNeighbor->entityIndices[nextEntityPos],
						  neighborNeighbor->entityInfoIndices[nextEntityPos],
						  neighborNeighborDirection, collisionDirection);
}
enav::DiagonalEntityCollisionInfo enav::getEntityInfoCornerToCornerB1(Tile* tile, LocalPosition position, LocalDirection direction)
{
	LocalDirection outDirection = cornerOutDirection(position, direction);
	LocalPosition nextEntityPos = tnav::combineAlignments(LocalPosition(direction), tnav::oppositeAlignment(LocalPosition(outDirection)));
	LocalDirection collisionDirection = outDirection;
	
	return DiagonalEntityCollisionInfo(tile->entityIndices[nextEntityPos],
						  tile->entityInfoIndices[nextEntityPos],
						  direction, collisionDirection);
}
enav::DiagonalEntityCollisionInfo enav::getEntityInfoCornerToCornerB2(Tile* tile, LocalPosition position, LocalDirection direction)
{
	LocalDirection outDirection = cornerOutDirection(position, direction);
	LocalDirection inverseOutDirection = tnav::oppositeAlignment(LocalPosition(outDirection));

	Tile* neighbor = tile->getNeighbor(direction);
	LocalDirection neighborDirection = tile->mapAlignmentTo1stDegreeNeighbor(direction, direction);

	LocalPosition nextEntityPos = tnav::combineAlignments(tnav::oppositeAlignment(LocalPosition(direction)), LocalPosition(inverseOutDirection));
	nextEntityPos = tile->mapAlignmentTo1stDegreeNeighbor(inverseOutDirection, nextEntityPos);

	LocalDirection collisionDirection = tnav::oppositeAlignment(outDirection);
	collisionDirection = tile->mapAlignmentTo1stDegreeNeighbor(outDirection, collisionDirection);

	return DiagonalEntityCollisionInfo(neighbor->entityIndices[nextEntityPos],
						  neighbor->entityInfoIndices[nextEntityPos],
						  neighborDirection, collisionDirection);
}
enav::DiagonalEntityCollisionInfo enav::getEntityInfoCornerToCornerB3(Tile* tile, LocalPosition position, LocalDirection direction)
{
	LocalDirection outDirection = cornerOutDirection(position, direction);
	LocalDirection inverseOutDirection = tnav::oppositeAlignment(LocalPosition(outDirection));

	Tile* neighbor = tile->getNeighbor(inverseOutDirection);
	LocalDirection neighborDirection = tile->mapAlignmentTo1stDegreeNeighbor(inverseOutDirection, direction);

	LocalPosition nextEntityPos = tnav::combineAlignments(LocalPosition(direction), LocalPosition(outDirection));
	nextEntityPos = tile->mapAlignmentTo1stDegreeNeighbor(inverseOutDirection, nextEntityPos);

	LocalDirection collisionDirection = tnav::oppositeAlignment(outDirection);
	collisionDirection = tile->mapAlignmentTo1stDegreeNeighbor(outDirection, collisionDirection);

	return DiagonalEntityCollisionInfo(neighbor->entityIndices[nextEntityPos],
						  neighbor->entityInfoIndices[nextEntityPos],
						  neighborDirection, collisionDirection);
}
enav::DiagonalEntityCollisionInfo enav::getEntityInfoCornerToCornerB4(Tile* tile, LocalPosition position, LocalDirection direction)
{
	LocalDirection outDirection = cornerOutDirection(position, direction);
	LocalDirection inverseOutDirection = tnav::oppositeAlignment(LocalPosition(outDirection));

	Tile* neighbor = tile->getNeighbor(inverseOutDirection);
	LocalDirection neighborDirection = tile->mapAlignmentTo1stDegreeNeighbor(inverseOutDirection, direction);

	Tile* neighborNeighbor = neighbor->getNeighbor(neighborDirection);
	LocalDirection neighborNeighborDirection = neighbor->mapAlignmentTo1stDegreeNeighbor(neighborDirection, neighborDirection);

	LocalPosition nextEntityPos = position;
	nextEntityPos = tile->mapAlignmentTo1stDegreeNeighbor(inverseOutDirection, nextEntityPos);
	nextEntityPos = tile->mapAlignmentTo1stDegreeNeighbor(neighborNeighborDirection, nextEntityPos);

	LocalDirection collisionDirection = tnav::oppositeAlignment(outDirection);
	collisionDirection = tile->mapAlignmentTo1stDegreeNeighbor(outDirection, collisionDirection);
	collisionDirection = neighbor->mapAlignmentTo1stDegreeNeighbor(neighborNeighborDirection, collisionDirection);

	return DiagonalEntityCollisionInfo(neighborNeighbor->entityIndices[nextEntityPos],
						  neighborNeighbor->entityInfoIndices[nextEntityPos],
						  neighborNeighborDirection, collisionDirection);
}