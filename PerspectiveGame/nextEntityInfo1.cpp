#include "nextEntityInfo1.h"

// 'out' is the direction component that leads away from the tile, not along its edge.
LocalDirection cornerOutDirection(LocalPosition position, LocalDirection direction)
{
	const LocalPosition* components = tnav::getAlignmentComponents(position);
	bool i = ((LocalDirection)components[0] != direction) ? 0 : 1;
	return tnav::oppositeAlignment(LocalDirection(components[!i]));
}

enav::OrthogonalEntityCollisionInfo getNextEntityInfoFromCenterOffset1(Tile* tile, LocalDirection direction, int offset)
{
	Tile* neighbor = tile->getNeighbor(direction);
	LocalDirection outDirection = LocalDirection((direction + offset) % 4);
	LocalDirection neighborDirection = tile->getMappedNeighborAlignment(direction, direction);
	LocalPosition nextEntityPos = tile->getMappedNeighborAlignment(direction, outDirection);

	return enav::OrthogonalEntityCollisionInfo(neighbor->entityIndices[nextEntityPos],
								neighbor->entityInfoIndices[nextEntityPos],
								neighborDirection);
}
enav::OrthogonalEntityCollisionInfo getNextEntityInfoFromCenterOffset2(Tile* tile, LocalDirection direction, int offset)
{
	LocalDirection outDirection = LocalDirection((direction + offset) % 4);

	Tile* neighbor = tile->getNeighbor(outDirection);
	LocalDirection neighborDirection = tile->getMappedNeighborAlignment(outDirection, direction);

	Tile* neighborNeighbor = neighbor->getNeighbor(neighborDirection);
	LocalDirection neighborNeighborDirection = neighbor->getMappedNeighborAlignment(neighborDirection, neighborDirection);

	LocalPosition nextEntityPos = tnav::oppositeAlignment(LocalPosition(outDirection));
	nextEntityPos = tile->getMappedNeighborAlignment(outDirection, nextEntityPos);
	nextEntityPos = neighbor->getMappedNeighborAlignment(neighborDirection, nextEntityPos);

	return enav::OrthogonalEntityCollisionInfo(neighborNeighbor->entityIndices[nextEntityPos],
								neighborNeighbor->entityInfoIndices[nextEntityPos],
								neighborNeighborDirection);
}
enav::DiagonalEntityCollisionInfo getNextEntityInfoFromCenterCorner(Tile* tile, LocalDirection direction, int offset)
{
	LocalDirection outDirection = LocalDirection((direction + offset) % 4);

	Tile* neighbor = tile->getNeighbor(outDirection);
	LocalDirection neighborDirection = tile->getMappedNeighborAlignment(outDirection, direction);
	
	Tile* neighborNeighbor = neighbor->getNeighbor(neighborDirection);
	LocalDirection neighborNeighborDirection = neighbor->getMappedNeighborAlignment(neighborDirection, neighborDirection);

	LocalDirection collisionDirection = tnav::oppositeAlignment(outDirection);
	collisionDirection = tile->getMappedNeighborAlignment(outDirection, collisionDirection);
	collisionDirection = neighbor->getMappedNeighborAlignment(neighborDirection, collisionDirection);

	return enav::DiagonalEntityCollisionInfo(neighborNeighbor->entityIndices[LOCAL_POSITION_CENTER],
											 neighborNeighbor->entityInfoIndices[LOCAL_POSITION_CENTER],
											 neighborNeighborDirection, collisionDirection);
}

enav::OrthogonalEntityCollisionInfo getNextEntityInfoFromEdgeOffset1(Tile* tile, LocalPosition position, int offset)
{
	LocalDirection direction = (LocalDirection)tnav::oppositeAlignment(position);
	LocalDirection outDirection = LocalDirection((direction + offset) % 4);
	LocalPosition nextEntityPos = tnav::combineAlignments(LocalPosition(direction), LocalPosition(outDirection));

	return enav::OrthogonalEntityCollisionInfo(tile->entityIndices[nextEntityPos],
								tile->entityInfoIndices[nextEntityPos],
								direction);
}
enav::OrthogonalEntityCollisionInfo getNextEntityInfoFromEdgeOffset2(Tile* tile, LocalPosition position, int offset)
{
	LocalDirection direction = (LocalDirection)tnav::oppositeAlignment(position);
	LocalDirection outDirection = LocalDirection((direction + offset) % 4);

	Tile* neighbor = tile->getNeighbor(direction);
	LocalDirection neighborDirection = tile->getMappedNeighborAlignment(direction, direction);

	LocalPosition nextEntityPos = tnav::combineAlignments(position, LocalPosition(outDirection));
	nextEntityPos = tile->getMappedNeighborAlignment(direction, nextEntityPos);

	return enav::OrthogonalEntityCollisionInfo(neighbor->entityIndices[nextEntityPos],
								neighbor->entityInfoIndices[nextEntityPos],
								neighborDirection);
}
enav::OrthogonalEntityCollisionInfo getNextEntityInfoFromEdgeOffset3(Tile* tile, LocalPosition position, int offset)
{
	LocalDirection direction = (LocalDirection)tnav::oppositeAlignment(position);
	LocalDirection outDirection = LocalDirection((direction + offset) % 4);

	Tile* neighbor = tile->getNeighbor(outDirection);
	LocalDirection neighborDirection = tile->getMappedNeighborAlignment(outDirection, direction);

	Tile* neighborNeighbor = neighbor->getNeighbor(neighborDirection);
	LocalDirection neighborNeighborDirection = neighbor->getMappedNeighborAlignment(neighborDirection, neighborDirection);

	LocalPosition nextEntityPos = tnav::combineAlignments(position, LocalPosition(tnav::oppositeAlignment(outDirection)));
	nextEntityPos = tile->getMappedNeighborAlignment(outDirection, nextEntityPos);
	nextEntityPos = neighbor->getMappedNeighborAlignment(neighborDirection, nextEntityPos);

	return enav::OrthogonalEntityCollisionInfo(neighborNeighbor->entityIndices[nextEntityPos],
								neighborNeighbor->entityInfoIndices[nextEntityPos],
								neighborNeighborDirection);
}
enav::OrthogonalEntityCollisionInfo getNextEntityInfoFromEdgeOffset4(Tile* tile, LocalPosition position, int offset)
{
	LocalDirection direction = (LocalDirection)tnav::oppositeAlignment(position);
	LocalDirection outDirection = LocalDirection((direction + offset) % 4);
	Tile* neighbor = tile->getNeighbor(outDirection);
	LocalDirection neighborDirection = tile->getMappedNeighborAlignment(outDirection, direction);
	LocalPosition nextEntityPos = tnav::combineAlignments(LocalPosition(direction), LocalPosition(tnav::oppositeAlignment(outDirection)));
	nextEntityPos = tile->getMappedNeighborAlignment(outDirection, nextEntityPos);

	return enav::OrthogonalEntityCollisionInfo(neighbor->entityIndices[nextEntityPos],
								neighbor->entityInfoIndices[nextEntityPos],
								neighborDirection);
}
enav::DiagonalEntityCollisionInfo getNextEntityInfoFromEdgeCorner1(Tile* tile, LocalPosition position, int offset)
{
	LocalDirection direction = (LocalDirection)tnav::oppositeAlignment(position);
	LocalDirection outDirection = LocalDirection((direction + offset) % 4);
	
	Tile* neighbor = tile->getNeighbor(outDirection);
	LocalDirection neighborDirection = tile->getMappedNeighborAlignment(outDirection, direction);
	LocalPosition nextEntityPos = tile->getMappedNeighborAlignment(outDirection, LocalPosition(direction));

	LocalDirection collisionDirection = tnav::oppositeAlignment(outDirection);
	collisionDirection = tile->getMappedNeighborAlignment(outDirection, collisionDirection);

	return enav::DiagonalEntityCollisionInfo(neighbor->entityIndices[nextEntityPos],
											 neighbor->entityInfoIndices[nextEntityPos],
											 neighborDirection, collisionDirection);
}
enav::DiagonalEntityCollisionInfo getNextEntityInfoFromEdgeCorner2(Tile* tile, LocalPosition position, int offset)
{
	LocalDirection direction = (LocalDirection)tnav::oppositeAlignment(position);
	LocalDirection outDirection = LocalDirection((direction + offset) % 4);

	Tile* neighbor = tile->getNeighbor(outDirection);
	LocalDirection neighborDirection = tile->getMappedNeighborAlignment(outDirection, direction);

	Tile* neighborNeighbor = neighbor->getNeighbor(neighborDirection);
	LocalDirection neighborNeighborDirection = neighbor->getMappedNeighborAlignment(neighborDirection, neighborDirection);

	LocalPosition nextEntityPos = tile->getMappedNeighborAlignment(outDirection, position);
	nextEntityPos = neighbor->getMappedNeighborAlignment(neighborDirection, nextEntityPos);

	LocalDirection collisionDirection = tnav::oppositeAlignment(outDirection);
	collisionDirection = tile->getMappedNeighborAlignment(outDirection, collisionDirection);
	collisionDirection = neighbor->getMappedNeighborAlignment(neighborDirection, collisionDirection);

	return enav::DiagonalEntityCollisionInfo(neighbor->entityIndices[nextEntityPos],
								neighbor->entityInfoIndices[nextEntityPos],
								neighborNeighborDirection,
											   collisionDirection);
}

enav::OrthogonalEntityCollisionInfo enav::getNextEntityInfoFromCenterToDirect(Tile* tile, LocalDirection direction)
{
	Tile* neighbor = tile->getNeighbor(direction);
	return enav::OrthogonalEntityCollisionInfo(neighbor->entityIndices[LOCAL_POSITION_CENTER],
								neighbor->entityInfoIndices[LOCAL_POSITION_CENTER],
								tile->getMappedNeighborAlignment(direction, direction));
}
enav::OrthogonalEntityCollisionInfo enav::getNextEntityInfoFromCenterToOffsetA1(Tile* tile, LocalDirection direction)
{
	return getNextEntityInfoFromCenterOffset1(tile, direction, 1);
}
enav::OrthogonalEntityCollisionInfo enav::getNextEntityInfoFromCenterToOffsetB1(Tile* tile, LocalDirection direction)
{
	return getNextEntityInfoFromCenterOffset1(tile, direction, 3);
}
enav::OrthogonalEntityCollisionInfo enav::getNextEntityInfoFromCenterToOffsetA2(Tile* tile, LocalDirection direction)
{
	return getNextEntityInfoFromCenterOffset2(tile, direction, 1);
}
enav::OrthogonalEntityCollisionInfo enav::getNextEntityInfoFromCenterToOffsetB2(Tile* tile, LocalDirection direction)
{
	return getNextEntityInfoFromCenterOffset2(tile, direction, 3);
}
enav::DiagonalEntityCollisionInfo enav::getNextEntityInfoFromCenterToCornerA(Tile* tile, LocalDirection direction)
{
	return getNextEntityInfoFromCenterCorner(tile, direction, 1);
}
enav::DiagonalEntityCollisionInfo enav::getNextEntityInfoFromCenterToCornerB(Tile* tile, LocalDirection direction)
{
	return getNextEntityInfoFromCenterCorner(tile, direction, 3);
}

enav::OrthogonalEntityCollisionInfo enav::getNextEntityInfoFromEdgeToDirectA(Tile* tile, LocalPosition position)
{
	LocalPosition oppositePos = tnav::oppositeAlignment(position);
	return enav::OrthogonalEntityCollisionInfo(tile->entityIndices[oppositePos],
								tile->entityInfoIndices[oppositePos],
								LocalDirection(oppositePos));
}
enav::OrthogonalEntityCollisionInfo enav::getNextEntityInfoFromEdgeToDirectB(Tile* tile, LocalPosition position)
{
	LocalDirection direction = (LocalDirection)tnav::oppositeAlignment(position);
	Tile* neighbor = tile->getNeighbor(direction);
	LocalDirection neighborDirection = tile->getMappedNeighborAlignment(direction, direction);
	LocalDirection nextEntityPos = tile->getMappedNeighborAlignment(direction, position);

	return enav::OrthogonalEntityCollisionInfo(neighbor->entityIndices[nextEntityPos],
								neighbor->entityInfoIndices[nextEntityPos],
								neighborDirection);
}
enav::OrthogonalEntityCollisionInfo enav::getNextEntityInfoFromEdgeToOffsetA1(Tile* tile, LocalPosition position)
{
	return getNextEntityInfoFromEdgeOffset1(tile, position, 1);
}
enav::OrthogonalEntityCollisionInfo enav::getNextEntityInfoFromEdgeToOffsetA2(Tile* tile, LocalPosition position)
{
	return getNextEntityInfoFromEdgeOffset2(tile, position, 1);
}
enav::OrthogonalEntityCollisionInfo enav::getNextEntityInfoFromEdgeToOffsetA3(Tile* tile, LocalPosition position)
{
	return getNextEntityInfoFromEdgeOffset3(tile, position, 1);
}
enav::OrthogonalEntityCollisionInfo enav::getNextEntityInfoFromEdgeToOffsetA4(Tile* tile, LocalPosition position)
{
	return getNextEntityInfoFromEdgeOffset4(tile, position, 1);
}
enav::OrthogonalEntityCollisionInfo enav::getNextEntityInfoFromEdgeToOffsetB1(Tile* tile, LocalPosition position)
{
	return getNextEntityInfoFromEdgeOffset1(tile, position, 3);
}
enav::OrthogonalEntityCollisionInfo enav::getNextEntityInfoFromEdgeToOffsetB2(Tile* tile, LocalPosition position)
{
	return getNextEntityInfoFromEdgeOffset2(tile, position, 3);
}
enav::OrthogonalEntityCollisionInfo enav::getNextEntityInfoFromEdgeToOffsetB3(Tile* tile, LocalPosition position)
{
	return getNextEntityInfoFromEdgeOffset3(tile, position, 3);
}
enav::OrthogonalEntityCollisionInfo enav::getNextEntityInfoFromEdgeToOffsetB4(Tile* tile, LocalPosition position)
{
	return getNextEntityInfoFromEdgeOffset4(tile, position, 3);
}
enav::DiagonalEntityCollisionInfo  enav::getNextEntityInfoFromEdgeToCornerA1(Tile* tile, LocalPosition position)
{
	return getNextEntityInfoFromEdgeCorner1(tile, position, 1);
}
enav::DiagonalEntityCollisionInfo  enav::getNextEntityInfoFromEdgeToCornerA2(Tile* tile, LocalPosition position)
{
	return getNextEntityInfoFromEdgeCorner2(tile, position, 1);
}
enav::DiagonalEntityCollisionInfo  enav::getNextEntityInfoFromEdgeToCornerB1(Tile* tile, LocalPosition position)
{
	return getNextEntityInfoFromEdgeCorner1(tile, position, 3);
}
enav::DiagonalEntityCollisionInfo  enav::getNextEntityInfoFromEdgeToCornerB2(Tile* tile, LocalPosition position)
{
	return getNextEntityInfoFromEdgeCorner2(tile, position, 3);
}

enav::OrthogonalEntityCollisionInfo enav::getNextEntityInfoFromCornerToDirectA(Tile* tile, LocalPosition position, LocalDirection direction)
{
	LocalPosition nextEntityPos = tnav::nextPosition(tnav::nextPosition(position, direction), direction);
	return OrthogonalEntityCollisionInfo(tile->entityIndices[nextEntityPos],
						  tile->entityInfoIndices[nextEntityPos],
						  direction);
}
enav::OrthogonalEntityCollisionInfo enav::getNextEntityInfoFromCornerToDirectB(Tile* tile, LocalPosition position, LocalDirection direction)
{
	Tile* neighbor = tile->getNeighbor(direction);
	LocalDirection neighborDirection = tile->getMappedNeighborAlignment(direction, direction);

	LocalPosition nextEntityPos = tile->getMappedNeighborAlignment(direction, position);

	return OrthogonalEntityCollisionInfo(neighbor->entityIndices[nextEntityPos],
						  neighbor->entityInfoIndices[nextEntityPos],
						  neighborDirection);
}
enav::OrthogonalEntityCollisionInfo enav::getNextEntityInfoFromCornerToDirectC(Tile* tile, LocalPosition position, LocalDirection direction)
{
	const LocalPosition* posComps = tnav::getAlignmentComponents(position);
	bool matchPosCompI = ((LocalDirection)posComps[0] != direction) ? 0 : 1;
	LocalDirection outDirection = tnav::oppositeAlignment(LocalDirection(posComps[!matchPosCompI]));

	Tile* neighbor = tile->getNeighbor(outDirection);
	LocalDirection neighborDirection = tile->getMappedNeighborAlignment(outDirection, direction);

	LocalPosition nextEntityPos = tile->getMappedNeighborAlignment(direction, tnav::oppositeAlignment(position));

	return OrthogonalEntityCollisionInfo(neighbor->entityIndices[nextEntityPos],
						  neighbor->entityInfoIndices[nextEntityPos],
						  neighborDirection);
}
enav::OrthogonalEntityCollisionInfo enav::getNextEntityInfoFromCornerToDirectD(Tile* tile, LocalPosition position, LocalDirection direction)
{
	LocalDirection outDirection = cornerOutDirection(position, direction);

	Tile* neighbor = tile->getNeighbor(outDirection);
	LocalDirection neighborDirection = tile->getMappedNeighborAlignment(outDirection, direction);

	Tile* neighborNeighbor = neighbor->getNeighbor(neighborDirection);
	LocalDirection neighborNeighborDirection = neighbor->getMappedNeighborAlignment(neighborDirection, neighborDirection);

	LocalPosition nextEntityPos = tnav::combineAlignments((LocalPosition)tnav::oppositeAlignment(direction), (LocalPosition)tnav::oppositeAlignment(outDirection));
	nextEntityPos = tile->getMappedNeighborAlignment(outDirection, nextEntityPos);
	nextEntityPos = neighbor->getMappedNeighborAlignment(neighborDirection, nextEntityPos);

	return OrthogonalEntityCollisionInfo(neighborNeighbor->entityIndices[nextEntityPos],
						  neighborNeighbor->entityInfoIndices[nextEntityPos],
						  neighborNeighborDirection);
}
enav::OrthogonalEntityCollisionInfo enav::getNextEntityInfoFromCornerToOffsetA1(Tile* tile, LocalPosition position, LocalDirection direction)
{
	LocalDirection outDirection = cornerOutDirection(position, direction);

	Tile* neighbor = tile->getNeighbor(outDirection);
	LocalDirection neighborDirection = tile->getMappedNeighborAlignment(outDirection, direction);

	LocalPosition nextEntityPos = tile->getMappedNeighborAlignment(outDirection, LocalPosition(direction));

	return OrthogonalEntityCollisionInfo(neighbor->entityIndices[nextEntityPos],
						  neighbor->entityInfoIndices[nextEntityPos],
						  neighborDirection);
}
enav::OrthogonalEntityCollisionInfo enav::getNextEntityInfoFromCornerToOffsetA2(Tile* tile, LocalPosition position, LocalDirection direction)
{
	LocalDirection outDirection = cornerOutDirection(position, direction);

	Tile* neighbor = tile->getNeighbor(outDirection);
	LocalDirection neighborDirection = tile->getMappedNeighborAlignment(outDirection, direction);

	Tile* neighborNeighbor = neighbor->getNeighbor(neighborDirection);
	LocalDirection neighborNeighborDirection = neighbor->getMappedNeighborAlignment(neighborDirection, neighborDirection);

	LocalPosition nextEntityPos = tile->getMappedNeighborAlignment(outDirection, tnav::oppositeAlignment(LocalPosition(direction)));
	nextEntityPos = neighbor->getMappedNeighborAlignment(neighborDirection, nextEntityPos);

	return OrthogonalEntityCollisionInfo(neighborNeighbor->entityIndices[nextEntityPos],
						  neighborNeighbor->entityInfoIndices[nextEntityPos],
						  neighborNeighborDirection);
}
enav::OrthogonalEntityCollisionInfo enav::getNextEntityInfoFromCornerToOffsetB1(Tile* tile, LocalPosition position, LocalDirection direction)
{
	return OrthogonalEntityCollisionInfo(tile->entityIndices[LocalPosition(direction)],
						  tile->entityInfoIndices[LocalPosition(direction)],
						  direction);
}
enav::OrthogonalEntityCollisionInfo enav::getNextEntityInfoFromCornerToOffsetB2(Tile* tile, LocalPosition position, LocalDirection direction)
{
	Tile* neighbor = tile->getNeighbor(direction);
	LocalDirection neighborDirection = tile->getMappedNeighborAlignment(direction, direction);

	LocalPosition nextEntityPos = tnav::oppositeAlignment(LocalPosition(direction));
	nextEntityPos = tile->getMappedNeighborAlignment(direction, nextEntityPos);

	return OrthogonalEntityCollisionInfo(neighbor->entityIndices[nextEntityPos],
						  neighbor->entityInfoIndices[nextEntityPos],
						  neighborDirection);
}
enav::DiagonalEntityCollisionInfo enav::getNextEntityInfoFromCornerToCornerA1(Tile* tile, LocalPosition position, LocalDirection direction)
{
	LocalDirection outDirection = cornerOutDirection(position, direction);

	Tile* neighbor = tile->getNeighbor(outDirection);
	LocalDirection neighborDirection = tile->getMappedNeighborAlignment(outDirection, direction);

	LocalPosition nextEntityPos = tnav::nextPosition(tnav::nextPosition(position, direction), direction);
	nextEntityPos = tile->getMappedNeighborAlignment(outDirection, LocalPosition(direction));

	LocalDirection collisionDirection = tnav::oppositeAlignment(outDirection);
	collisionDirection = tile->getMappedNeighborAlignment(outDirection, collisionDirection);

	return DiagonalEntityCollisionInfo(neighbor->entityIndices[nextEntityPos],
						  neighbor->entityInfoIndices[nextEntityPos],
						  neighborDirection, collisionDirection);
}
enav::DiagonalEntityCollisionInfo enav::getNextEntityInfoFromCornerToCornerA2(Tile* tile, LocalPosition position, LocalDirection direction)
{
	LocalDirection outDirection = cornerOutDirection(position, direction);

	Tile* neighbor = tile->getNeighbor(outDirection);
	LocalDirection neighborDirection = tile->getMappedNeighborAlignment(outDirection, direction);

	Tile* neighborNeighbor = neighbor->getNeighbor(neighborDirection);
	LocalDirection neighborNeighborDirection = neighbor->getMappedNeighborAlignment(neighborDirection, neighborDirection);

	LocalPosition nextEntityPos = tile->getMappedNeighborAlignment(outDirection, position);
	nextEntityPos = neighbor->getMappedNeighborAlignment(neighborDirection, nextEntityPos);

	LocalDirection collisionDirection = tnav::oppositeAlignment(outDirection);
	collisionDirection = tile->getMappedNeighborAlignment(outDirection, collisionDirection);
	collisionDirection = neighbor->getMappedNeighborAlignment(neighborDirection, collisionDirection);

	return DiagonalEntityCollisionInfo(neighborNeighbor->entityIndices[nextEntityPos],
						  neighborNeighbor->entityInfoIndices[nextEntityPos],
						  neighborNeighborDirection, collisionDirection);
}
enav::DiagonalEntityCollisionInfo enav::getNextEntityInfoFromCornerToCornerA3(Tile* tile, LocalPosition position, LocalDirection direction)
{
	LocalDirection outDirection = cornerOutDirection(position, direction);

	Tile* neighbor = tile->getNeighbor(outDirection);
	LocalDirection neighborDirection = tile->getMappedNeighborAlignment(outDirection, direction);
	LocalDirection neighborOutDirection = tile->getMappedNeighborAlignment(outDirection, outDirection);

	Tile* neighborNeighbor = neighbor->getNeighbor(neighborOutDirection);
	LocalDirection neighborNeighborDirection = neighbor->getMappedNeighborAlignment(neighborOutDirection, neighborDirection);

	LocalPosition nextEntityPos = tnav::oppositeAlignment(position);
	nextEntityPos = tile->getMappedNeighborAlignment(outDirection, nextEntityPos);
	nextEntityPos = neighbor->getMappedNeighborAlignment(neighborOutDirection, nextEntityPos);

	LocalDirection collisionDirection = tnav::oppositeAlignment(outDirection);
	collisionDirection = tile->getMappedNeighborAlignment(outDirection, collisionDirection);
	collisionDirection = neighbor->getMappedNeighborAlignment(neighborDirection, collisionDirection);

	return DiagonalEntityCollisionInfo(neighborNeighbor->entityIndices[nextEntityPos],
						  neighborNeighbor->entityInfoIndices[nextEntityPos],
						  neighborNeighborDirection, collisionDirection);
}
enav::DiagonalEntityCollisionInfo enav::getNextEntityInfoFromCornerToCornerA4(Tile* tile, LocalPosition position, LocalDirection direction)
{
	LocalDirection outDirection = cornerOutDirection(position, direction);

	Tile* neighbor = tile->getNeighbor(outDirection);
	LocalDirection neighborDirection = tile->getMappedNeighborAlignment(outDirection, direction);
	LocalDirection neighborOutDirection = tile->getMappedNeighborAlignment(outDirection, outDirection);

	Tile* neighborNeighbor = neighbor->getNeighbor(neighborOutDirection);
	LocalDirection neighborNeighborDirection = neighbor->getMappedNeighborAlignment(neighborOutDirection, neighborDirection);

	Tile* neighborNeighborNeighbor = neighborNeighbor->getNeighbor(neighborNeighborDirection);
	LocalDirection neighborNeighborNeighborDirection = neighborNeighbor->getMappedNeighborAlignment(neighborNeighborDirection, neighborNeighborDirection);

	LocalPosition nextEntityPos = tnav::combineAlignments(tnav::oppositeAlignment(LocalPosition(direction)), tnav::oppositeAlignment(LocalPosition(outDirection)));
	nextEntityPos = tile->getMappedNeighborAlignment(outDirection, nextEntityPos);
	nextEntityPos = neighbor->getMappedNeighborAlignment(neighborOutDirection, nextEntityPos);
	nextEntityPos = neighborNeighbor->getMappedNeighborAlignment(neighborNeighborNeighborDirection, nextEntityPos);

	LocalDirection collisionDirection = tnav::oppositeAlignment(outDirection);
	collisionDirection = tile->getMappedNeighborAlignment(outDirection, collisionDirection);
	collisionDirection = neighbor->getMappedNeighborAlignment(neighborDirection, collisionDirection);
	collisionDirection = neighborNeighbor->getMappedNeighborAlignment(neighborNeighborDirection, collisionDirection);

	return DiagonalEntityCollisionInfo(neighborNeighbor->entityIndices[nextEntityPos],
						  neighborNeighbor->entityInfoIndices[nextEntityPos],
						  neighborNeighborDirection, collisionDirection);
}
enav::DiagonalEntityCollisionInfo enav::getNextEntityInfoFromCornerToCornerB1(Tile* tile, LocalPosition position, LocalDirection direction)
{
	LocalDirection outDirection = cornerOutDirection(position, direction);
	LocalPosition nextEntityPos = tnav::combineAlignments(LocalPosition(direction), tnav::oppositeAlignment(LocalPosition(outDirection)));
	LocalDirection collisionDirection = outDirection;
	
	return DiagonalEntityCollisionInfo(tile->entityIndices[nextEntityPos],
						  tile->entityInfoIndices[nextEntityPos],
						  direction, collisionDirection);
}
enav::DiagonalEntityCollisionInfo enav::getNextEntityInfoFromCornerToCornerB2(Tile* tile, LocalPosition position, LocalDirection direction)
{
	LocalDirection outDirection = cornerOutDirection(position, direction);
	LocalDirection inverseOutDirection = tnav::oppositeAlignment(LocalPosition(outDirection));

	Tile* neighbor = tile->getNeighbor(direction);
	LocalDirection neighborDirection = tile->getMappedNeighborAlignment(direction, direction);

	LocalPosition nextEntityPos = tnav::combineAlignments(tnav::oppositeAlignment(LocalPosition(direction)), LocalPosition(inverseOutDirection));
	nextEntityPos = tile->getMappedNeighborAlignment(inverseOutDirection, nextEntityPos);

	LocalDirection collisionDirection = tnav::oppositeAlignment(outDirection);
	collisionDirection = tile->getMappedNeighborAlignment(outDirection, collisionDirection);

	return DiagonalEntityCollisionInfo(neighbor->entityIndices[nextEntityPos],
						  neighbor->entityInfoIndices[nextEntityPos],
						  neighborDirection, collisionDirection);
}
enav::DiagonalEntityCollisionInfo enav::getNextEntityInfoFromCornerToCornerB3(Tile* tile, LocalPosition position, LocalDirection direction)
{
	LocalDirection outDirection = cornerOutDirection(position, direction);
	LocalDirection inverseOutDirection = tnav::oppositeAlignment(LocalPosition(outDirection));

	Tile* neighbor = tile->getNeighbor(inverseOutDirection);
	LocalDirection neighborDirection = tile->getMappedNeighborAlignment(inverseOutDirection, direction);

	LocalPosition nextEntityPos = tnav::combineAlignments(LocalPosition(direction), LocalPosition(outDirection));
	nextEntityPos = tile->getMappedNeighborAlignment(inverseOutDirection, nextEntityPos);

	LocalDirection collisionDirection = tnav::oppositeAlignment(outDirection);
	collisionDirection = tile->getMappedNeighborAlignment(outDirection, collisionDirection);

	return DiagonalEntityCollisionInfo(neighbor->entityIndices[nextEntityPos],
						  neighbor->entityInfoIndices[nextEntityPos],
						  neighborDirection, collisionDirection);
}
enav::DiagonalEntityCollisionInfo enav::getNextEntityInfoFromCornerToCornerB4(Tile* tile, LocalPosition position, LocalDirection direction)
{
	LocalDirection outDirection = cornerOutDirection(position, direction);
	LocalDirection inverseOutDirection = tnav::oppositeAlignment(LocalPosition(outDirection));

	Tile* neighbor = tile->getNeighbor(inverseOutDirection);
	LocalDirection neighborDirection = tile->getMappedNeighborAlignment(inverseOutDirection, direction);

	Tile* neighborNeighbor = neighbor->getNeighbor(neighborDirection);
	LocalDirection neighborNeighborDirection = neighbor->getMappedNeighborAlignment(neighborDirection, neighborDirection);

	LocalPosition nextEntityPos = position;
	nextEntityPos = tile->getMappedNeighborAlignment(inverseOutDirection, nextEntityPos);
	nextEntityPos = tile->getMappedNeighborAlignment(neighborNeighborDirection, nextEntityPos);

	LocalDirection collisionDirection = tnav::oppositeAlignment(outDirection);
	collisionDirection = tile->getMappedNeighborAlignment(outDirection, collisionDirection);
	collisionDirection = neighbor->getMappedNeighborAlignment(neighborNeighborDirection, collisionDirection);

	return DiagonalEntityCollisionInfo(neighborNeighbor->entityIndices[nextEntityPos],
						  neighborNeighbor->entityInfoIndices[nextEntityPos],
						  neighborNeighborDirection, collisionDirection);
}