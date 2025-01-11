#include "tileNavigation.h"

const LocalAlignment tnav::LOCAL_ALIGNMENT_SET[] = {
		LOCAL_ALIGNMENT_0, LOCAL_ALIGNMENT_1, LOCAL_ALIGNMENT_2, LOCAL_ALIGNMENT_3,
		LOCAL_ALIGNMENT_0_1,LOCAL_ALIGNMENT_1_2,LOCAL_ALIGNMENT_2_3,LOCAL_ALIGNMENT_3_0,
		LOCAL_ALIGNMENT_NONE,
};

const LocalDirection tnav::DIRECTION_SET[8] = {
	LOCAL_ALIGNMENT_0, LOCAL_ALIGNMENT_1, LOCAL_ALIGNMENT_2, LOCAL_ALIGNMENT_3,
	LOCAL_ALIGNMENT_0_1,LOCAL_ALIGNMENT_1_2,LOCAL_ALIGNMENT_2_3,LOCAL_ALIGNMENT_3_0
};

const LocalDirection tnav::ORTHOGONAL_DIRECTION_SET[4] = {
	LOCAL_ALIGNMENT_0, LOCAL_ALIGNMENT_1, LOCAL_ALIGNMENT_2, LOCAL_ALIGNMENT_3
};

const LocalDirection tnav::DIAGONAL_DIRECTION_SET[4] = {
	LOCAL_ALIGNMENT_0_1, LOCAL_ALIGNMENT_1_2, LOCAL_ALIGNMENT_2_3, LOCAL_ALIGNMENT_3_0
};

void tnav::checkOrthogonal(LocalAlignment alignment)
{
	if (alignment > 3) std::cout << "ORTHOGONAL ALIGNMENT EXPECTED BUT NOT RESPECTED!" << std::endl;
}

// Given a [tile type] and a [meta alignment], will return the matching local alignment:
const LocalAlignment META_TO_LOCAL_ALIGNMENT[6][19] = {
	// Define some macros to make the below array more legible:
	#define __0__ LOCAL_ALIGNMENT_0
	#define __1__ LOCAL_ALIGNMENT_1
	#define __2__ LOCAL_ALIGNMENT_2
	#define __3__ LOCAL_ALIGNMENT_3
	#define _0_1_ LOCAL_ALIGNMENT_0_1
	#define _1_2_ LOCAL_ALIGNMENT_1_2
	#define _2_3_ LOCAL_ALIGNMENT_2_3
	#define _3_0_ LOCAL_ALIGNMENT_3_0
	#define none LOCAL_ALIGNMENT_NONE
	#define error LOCAL_ALIGNMENT_ERROR

	// Meta Alignment:   |  a1  |  a2  |  b1  |  b2  |  c1  |  c2  | a1b1 | a1b2 | a2b1 | a2b2 | a1c1 | a1c2 | a2c1 | a2c2 | b1c1 | b1c2 | b2c1 | b2c2 | none |
	/* Tile Type XYF: */ { error, error, __0__, __2__, __3__, __1__, error, error, error, error, error, error, error, error, _3_0_, _0_1_, _2_3_, _1_2_, none },
	/* Tile Type XYB: */ { error, error, __2__, __0__, __1__, __3__, error, error, error, error, error, error, error, error, _1_2_, _2_3_, _0_1_, _3_0_, none },
	/* Tile Type XZF: */ { __1__, __3__, error, error, __2__, __0__, error, error, error, error, _1_2_, _0_1_, _2_3_, _3_0_, error, error, error, error, none },
	/* Tile Type XZB: */ { __3__, __1__, error, error, __0__, __2__, error, error, error, error, _3_0_, _2_3_, _0_1_, _1_2_, error, error, error, error, none },
	/* Tile Type YZF: */ { __0__, __2__, __1__, __3__, error, error, _0_1_, _3_0_, _1_2_, _2_3_, error, error, error, error, error, error, error, error, none },
	/* Tile Type YZB: */ { __2__, __0__, __3__, __1__, error, error, _2_3_, _1_2_, _3_0_, _0_1_, error, error, error, error, error, error, error, error, none }
	
	// undefine macros to avoid errors:
	#undef __0__
	#undef __1__
	#undef __2__
	#undef __3__
	#undef _0_1_
	#undef _1_2_
	#undef _2_3_
	#undef _3_0_
	#undef none 
	#undef error
};

// Breaks down diagonal directions into their components.
// Orthogonal components are just duplicated.
const LocalDirection ALIGNMENT_COMPONENTS[8][2] = {
	// Orthogonal:
	{ LOCAL_ALIGNMENT_0, LOCAL_ALIGNMENT_0 }, // LOCAL_DIRECTION_0
	{ LOCAL_ALIGNMENT_1, LOCAL_ALIGNMENT_1 }, // LOCAL_DIRECTION_1
	{ LOCAL_ALIGNMENT_2, LOCAL_ALIGNMENT_2 }, // LOCAL_DIRECTION_2
	{ LOCAL_ALIGNMENT_3, LOCAL_ALIGNMENT_3 }, // LOCAL_DIRECTION_3
	// Diagonal:
	{ LOCAL_ALIGNMENT_0, LOCAL_ALIGNMENT_1 }, // LOCAL_DIRECTION_0_1
	{ LOCAL_ALIGNMENT_1, LOCAL_ALIGNMENT_2 }, // LOCAL_DIRECTION_1_2
	{ LOCAL_ALIGNMENT_2, LOCAL_ALIGNMENT_3 }, // LOCAL_DIRECTION_2_3
	{ LOCAL_ALIGNMENT_3, LOCAL_ALIGNMENT_0 }  // LOCAL_DIRECTION_3_0
};

const LocalAlignment* tnav::getAlignmentComponents(LocalAlignment alignment)
{
	return ALIGNMENT_COMPONENTS[alignment];
}

// Given two orthogonal LocalDirections, combines them into an orthogonal or diagonal direction.
// Directions facing opposite ways are combined into the static enum.
const LocalDirection COMBINED_ALIGNMENTS[4][4] = {
	{ LOCAL_ALIGNMENT_0, LOCAL_ALIGNMENT_0_1, LOCAL_ALIGNMENT_NONE, LOCAL_ALIGNMENT_3_0 },
	{ LOCAL_ALIGNMENT_0_1, LOCAL_ALIGNMENT_1, LOCAL_ALIGNMENT_1_2, LOCAL_ALIGNMENT_NONE },
	{ LOCAL_ALIGNMENT_NONE, LOCAL_ALIGNMENT_1_2, LOCAL_ALIGNMENT_2, LOCAL_ALIGNMENT_2_3 },
	{ LOCAL_ALIGNMENT_3_0, LOCAL_ALIGNMENT_NONE, LOCAL_ALIGNMENT_2_3, LOCAL_ALIGNMENT_3 }
};

const LocalAlignment tnav::combine(LocalAlignment a, LocalAlignment b)
{
	return COMBINED_ALIGNMENTS[a][b];
}

// [local Position][local direction]
const LocalPosition NEXT_LOCAL_POSITIONS[9][8] = {
#define _0_ LOCAL_POSITION_0
#define _1_ LOCAL_POSITION_1
#define _2_ LOCAL_POSITION_2
#define _3_ LOCAL_POSITION_3
#define _01 LOCAL_POSITION_0_1
#define _12 LOCAL_POSITION_1_2
#define _23 LOCAL_POSITION_2_3
#define _30 LOCAL_POSITION_3_0
#define CEN LOCAL_POSITION_CENTER
#define XXX LOCAL_DIRECTION_ERROR
	{ XXX, _01, CEN, _30, XXX, _1_, _3_, XXX },
	{ _01, XXX, _12, CEN, XXX, XXX, _2_, _0_ },
	{ CEN, _12, XXX, _23, _1_, XXX, XXX, _3_ },
	{ _30, CEN, _23, XXX, _0_, _2_, XXX, XXX },
	{ XXX, XXX, _1_, _0_, XXX, XXX, CEN, XXX },
	{ _1_, XXX, XXX, _2_, XXX, XXX, XXX, CEN },
	{ _3_, _2_, XXX, XXX, CEN, XXX, XXX, XXX },
	{ XXX, _0_, _3_, XXX, XXX, CEN, XXX, XXX },
	{ _0_, _1_, _2_, _3_, _01, _12, _23, _30 }
#undef _0_
#undef _1_
#undef _2_
#undef _3_
#undef _01
#undef _12
#undef _23
#undef _30
#undef CEN
#undef XXX
};

LocalPosition tnav::nextPosition(LocalPosition position, LocalDirection direction)
{
	return NEXT_LOCAL_POSITIONS[position][direction];
}

// [local Position][local direction]
const LocalPosition NEXT_NEXT_LOCAL_POSITIONS[9][8] = {
#define _0_ LOCAL_POSITION_0
#define _1_ LOCAL_POSITION_1
#define _2_ LOCAL_POSITION_2
#define _3_ LOCAL_POSITION_3
#define _01 LOCAL_POSITION_0_1
#define _12 LOCAL_POSITION_1_2
#define _23 LOCAL_POSITION_2_3
#define _30 LOCAL_POSITION_3_0
#define CEN LOCAL_POSITION_CENTER
#define XXX LOCAL_DIRECTION_ERROR
	{ XXX, XXX, _2_, XXX, XXX, XXX, XXX, XXX },
	{ XXX, XXX, XXX, _3_, XXX, XXX, XXX, XXX },
	{ _0_, XXX, XXX, XXX, XXX, XXX, XXX, XXX },
	{ XXX, _1_, XXX, XXX, XXX, XXX, XXX, XXX },
	{ XXX, XXX, _12, _30, XXX, XXX, _23, XXX },
	{ _01, XXX, XXX, _23, XXX, XXX, XXX, _30 },
	{ _30, _12, XXX, XXX, _01, XXX, XXX, XXX },
	{ XXX, _01, _23, XXX, XXX, _12, XXX, XXX },
	{ XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX }
#undef _0_
#undef _1_
#undef _2_
#undef _3_
#undef _01
#undef _12
#undef _23
#undef _30
#undef CEN
#undef XXX
};

const LocalPosition tnav::getNextNextPosition(LocalPosition position, LocalDirection direction)
{
	return NEXT_NEXT_LOCAL_POSITIONS[position][direction];
}

/*
* BREIF: Given a local position, this array will return the 1 to 4 sub-tile area indicess surrounding that position.
*
* WARNING: a -1 return denotes no information/position outside of tile (for positions on the edge of the tile).
*
* Key: LocalPositionToSubTileIndex[LocalPosition][Sub-Tile Index]
*/ 
const int LOCAL_POSITION_TO_SUB_TILE_AREA_INDICES[25][4] = {
#define X -1
	{ X,  X, X,  0 }, {  X,  X,  0,  1 }, {  X,  X,  1,  2 }, {  X,  X,  2,  3 }, {  X, X,  3, X },
	{ X,  0, X,  4 }, {  0,  1,  4,  5 }, {  1,  2,  5,  6 }, {  2,  3,  6,  7 }, {  3, X,  7, X },
	{ X,  4, X,  8 }, {  4,  5,  8,  9 }, {  5,  6,  9, 10 }, {  6,  7, 10, 11 }, {  7, X, 11, X },
	{ X,  8, X, 12 }, {  8,  9, 12, 13 }, {  9, 10, 13, 14 }, { 10, 11, 14, 15 }, { 11, X, 15, X },
	{ X, 12, X,  X }, { 12, 13,  X,  X }, { 13, 14,  X,  X }, { 14, 15,  X,  X }, { 15, X,  X, X }
#undef X
};

/*
*
*/
const int NEXT_SUB_TILE_AREA_INDICES[16][8] = {
#define XX -1
	{  1,  4, XX, XX,  5, XX, XX, XX },
	{  2,  5,  0, XX,  6,  4, XX, XX },
	{  3,  6,  1, XX,  7,  5, XX, XX },
	{ XX,  7,  2, XX, XX,  6, XX, XX },
	{  5,  8, XX,  0,  9, XX, XX,  1 },
	{  6,  9,  4,  1, 10,  8,  0,  2 },
	{  7, 10,  5,  2, 11,  9,  1,  3 },
	{ XX, 11,  6,  3, XX, 10,  2, XX },
	{  9, 12, XX,  4, 13, XX, XX,  5 },
	{ 10, 13,  8,  5, 14, 12,  4,  6 },
	{ 11, 14,  9,  6, 15, 13,  5,  7 },
	{ XX, 15, 10,  7, XX, 14,  6, XX },
	{ 13, XX, XX,  8, XX, XX, XX,  9 },
	{ 14, XX, 12,  9, XX, XX,  8, 10 },
	{ 15, XX, 13, 10, XX, XX,  9, 11 },
	{ XX, XX, 14, 11, XX, XX, 10, XX },
#undef XX
};

/*
* BREIF: Given a sub-tile area index and a direction from that area, This array stores the number of bits needed
*        to shift for the movementInfo of the next area in the given direction to have no leading 0s.
*
* EXAMPLE: If the sub-tile area index == 1 and direction == LOCAL_DIRECTION_2, movementInfo will need to be shifted
*          4 bits to the right to go from ...AA][XXXX][BBBB] to ...AA[XXXX], where 'XXXX' is the movement info
*          associated with sub-tile area 1.
*
* EXAMPLE: If the sub-tile area index == 1 and direction == LOCAL_DIRECTION_1_2, movementInfo will need to be shifted
*          16 bits to the right as the diagonal next position is 'down' 1 and 'left' 1 in local directions.
*
* WARNING: If the next sub-tile area is not within the current tile (i.e. going left from positions 0, 4, 8, or 12),
*          this array will return -1, denoting an INVALID area index!
*/ const int NEXT_SUB_TILE_AREA_SHIFTS[16][8] = {
#define XX -1
	{  4, 16, XX, XX, 20, XX, XX, XX },
	{  8, 20,  0, XX, 24, 16, XX, XX },
	{ 12, 24,  4, XX, 28, 20, XX, XX },
	{ XX, 28,  8, XX, XX, 24, XX, XX },
	{ 20, 32, XX,  0, 36, XX, XX,  4 },
	{ 24, 36, 16,  4, 40, 32,  0,  8 },
	{ 28, 40, 20,  8, 44, 36,  4, 12 },
	{ XX, 44, 24, 12, XX, 40,  8, XX },
	{ 36, 48, XX, 16, 52, XX, XX, 20 },
	{ 40, 52, 32, 20, 56, 48, 16, 24 },
	{ 44, 56, 36, 24, 60, 52, 20, 28 },
	{ XX, 60, 40, 28, XX, 56, 24, XX },
	{ 52, XX, XX, 32, XX, XX, XX, 36 },
	{ 56, XX, 48, 36, XX, XX, 32, 40 },
	{ 60, XX, 52, 40, XX, XX, 36, 44 },
	{ XX, XX, 56, 44, XX, XX, 40, XX }
#undef XX
};

// Has no error return! Will give 0x0 if outside of tile.
const uint16_t NEXT_SUB_TILE_OBSTRUCTION_MASKS[16][8] = {
	#define XXX 0x0
	{ 0x1, 0x4, XXX, XXX, 0x5, XXX, XXX, XXX },
	{ 0x2, 0x5, 0x0, XXX, 0x6, 0x4, XXX, XXX },
	{ 0x3, 0x6, 0x1, XXX, 0x7, 0x5, XXX, XXX },
	{ XXX, 0x7, 0x2, XXX, XXX, 0x6, XXX, XXX },
	{ 0x5, 0x8, XXX, 0x0, 0x9, XXX, XXX, 0x1 },
	{ 0x6, 0x9, 0x4, 0x1, 0xa, 0x8, 0x0, 0x2 },
	{ 0x7, 0xa, 0x5, 0x2, 0xb, 0x9, 0x1, 0x3 },
	{ XXX, 0xb, 0x6, 0x3, XXX, 0xa, 0x2, XXX },
	{ 0x9, 0xc, XXX, 0x4, 0xd, XXX, XXX, 0x5 },
	{ 0xa, 0xd, 0x8, 0x5, 0xe, 0xc, 0x4, 0x6 },
	{ 0xb, 0xe, 0x9, 0x6, 0xf, 0xd, 0x5, 0x7 },
	{ XXX, 0xf, 0xa, 0x7, XXX, 0xe, 0x6, XXX },
	{ 0xd, XXX, XXX, 0x8, XXX, XXX, XXX, 0x9 },
	{ 0xe, XXX, 0xc, 0x9, XXX, XXX, 0x8, 0xa },
	{ 0xf, XXX, 0xd, 0xa, XXX, XXX, 0x9, 0xb },
	{ XXX, XXX, 0xe, 0xb, XXX, XXX, 0xa, XXX }
#undef XXX
};

// Returns the adjusted local orientation of an entity/basis when going from one tile to another.
// Input Key: ALIGNMENT_TO_ALIGNMENT_MAP[current tile-type][neighbor tile-type][ORTHOGONAL exiting direction][alignment]
const LocalAlignment ALIGNMENT_TO_ALIGNMENT_MAP[6][6][4][8] = {
#define XXX LOCAL_DIRECTION_ERROR
#define __0 LOCAL_DIRECTION_0
#define __1 LOCAL_DIRECTION_1
#define __2 LOCAL_DIRECTION_2
#define __3 LOCAL_DIRECTION_3
#define _01 LOCAL_DIRECTION_0_1
#define _12 LOCAL_DIRECTION_1_2
#define _23 LOCAL_DIRECTION_2_3
#define _30 LOCAL_DIRECTION_3_0
	{ // Current Tile XYF:
		/* Destination Tile XYF: */ { { __0, __1, __2, __3, _01, _12, _23, _30 }, { __0, __1, __2, __3, _01, _12, _23, _30 }, { __0, __1, __2, __3, _01, _12, _23, _30 }, { __0, __1, __2, __3, _01, _12, _23, _30 }, /* exiting Side 0, 1, 2, 3 */ },
		/* Destination Tile XYB: */ { { __2, __1, __0, __3, _12, _01, _30, _23 }, { __0, __3, __2, __1, _30, _23, _12, _01 }, { __2, __1, __0, __3, _12, _01, _30, _23 }, { __0, __3, __2, __1, _30, _23, _12, _01 }, /* exiting Side 0, 1, 2, 3 */ },
		/* Destination Tile XZF: */ { { XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX }, { __3, __0, __1, __2, _30, _01, _12, _23 }, { XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX }, { __3, __0, __1, __2, _30, _01, _12, _23 }, /* exiting Side 0, 1, 2, 3 */ },
		/* Destination Tile XZB: */ { { XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX }, { __3, __2, __1, __0, _23, _12, _01, _30 }, { XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX }, { __3, __2, __1, __0, _23, _12, _01, _30 }, /* exiting Side 0, 1, 2, 3 */ },
		/* Destination Tile YZF: */ { { __1, __2, __3, __0, _12, _23, _30, _01 }, { XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX }, { __1, __2, __3, __0, _12, _23, _30, _01 }, { XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX }, /* exiting Side 0, 1, 2, 3 */ },
		/* Destination Tile YZB: */ { { __3, __2, __1, __0, _23, _12, _01, _30 }, { XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX }, { __3, __2, __1, __0, _23, _12, _01, _30 }, { XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX }, /* exiting Side 0, 1, 2, 3 */ },
	},
	{ // Current Tile XYB:
		/* Destination Tile XYF: */ { { __2, __1, __0, __3, _12, _01, _30, _23 }, { __0, __3, __2, __1, _30, _23, _12, _01 }, { __2, __1, __0, __3, _12, _01, _30, _23 }, { __0, __3, __2, __1, _30, _23, _12, _01 }, /* exiting Side 0, 1, 2, 3 */ },
		/* Destination Tile XYB: */ { { __0, __1, __2, __3, _01, _12, _23, _30 }, { __0, __1, __2, __3, _01, _12, _23, _30 }, { __0, __1, __2, __3, _01, _12, _23, _30 }, { __0, __1, __2, __3, _01, _12, _23, _30 }, /* exiting Side 0, 1, 2, 3 */ },
		/* Destination Tile XZF: */ { { XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX }, { __3, __2, __1, __0, _23, _12, _01, _30 }, { XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX }, { __3, __2, __1, __0, _23, _12, _01, _30 }, /* exiting Side 0, 1, 2, 3 */ },
		/* Destination Tile XZB: */ { { XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX }, { __3, __0, __1, __2, _30, _01, _12, _23 }, { XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX }, { __3, __0, __1, __2, _30, _01, _12, _23 }, /* exiting Side 0, 1, 2, 3 */ },
		/* Destination Tile YZF: */ { { __3, __2, __1, __0, _23, _12, _01, _30 }, { XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX }, { __3, __2, __1, __0, _23, _12, _01, _30 }, { XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX }, /* exiting Side 0, 1, 2, 3 */ },
		/* Destination Tile YZB: */ { { __1, __2, __3, __0, _12, _23, _30, _01 }, { XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX }, { __1, __2, __3, __0, _12, _23, _30, _01 }, { XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX }, /* exiting Side 0, 1, 2, 3 */ },
	},
	{ // Current Tile XZF:
		/* Destination Tile XYF: */ { { __1, __2, __3, __0, _12, _23, _30, _01 }, { XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX }, { __1, __2, __3, __0, _12, _23, _30, _01 }, { XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX }, /* exiting Side 0, 1, 2, 3 */ },
		/* Destination Tile XYB: */ { { __3, __2, __1, __0, _23, _12, _01, _30 }, { XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX }, { __3, __2, __1, __0, _23, _12, _01, _30 }, { XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX }, /* exiting Side 0, 1, 2, 3 */ },
		/* Destination Tile XZF: */ { { __0, __1, __2, __3, _01, _12, _23, _30 }, { __0, __1, __2, __3, _01, _12, _23, _30 }, { __0, __1, __2, __3, _01, _12, _23, _30 }, { __0, __1, __2, __3, _01, _12, _23, _30 }, /* exiting Side 0, 1, 2, 3 */ },
		/* Destination Tile XZB: */ { { __2, __1, __0, __3, _12, _01, _30, _23 }, { __0, __3, __2, __1, _30, _23, _12, _01 }, { __2, __1, __0, __3, _12, _01, _30, _23 }, { __0, __3, __2, __1, _30, _23, _12, _01 }, /* exiting Side 0, 1, 2, 3 */ },
		/* Destination Tile YZF: */ { { XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX }, { __3, __0, __1, __2, _30, _01, _12, _23 }, { XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX }, { __3, __0, __1, __2, _30, _01, _12, _23 }, /* exiting Side 0, 1, 2, 3 */ },
		/* Destination Tile YZB: */ { { XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX }, { __3, __2, __1, __0, _23, _12, _01, _30 }, { XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX }, { __3, __2, __1, __0, _23, _12, _01, _30 }, /* exiting Side 0, 1, 2, 3 */ },
	},
	{ // Current Tile XZB:
		/* Destination Tile XYF: */ { { __3, __2, __1, __0, _23, _12, _01, _30 }, { XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX }, { __3, __2, __1, __0, _23, _12, _01, _30 }, { XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX }, /* exiting Side 0, 1, 2, 3 */ },
		/* Destination Tile XYB: */ { { __1, __2, __3, __0, _12, _23, _30, _01 }, { XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX }, { __1, __2, __3, __0, _12, _23, _30, _01 }, { XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX }, /* exiting Side 0, 1, 2, 3 */ },
		/* Destination Tile XZF: */ { { __2, __1, __0, __3, _12, _01, _30, _23 }, { __0, __3, __2, __1, _30, _23, _12, _01 }, { __2, __1, __0, __3, _12, _01, _30, _23 }, { __0, __3, __2, __1, _30, _23, _12, _01 }, /* exiting Side 0, 1, 2, 3 */ },
		/* Destination Tile XZB: */ { { __0, __1, __2, __3, _01, _12, _23, _30 }, { __0, __1, __2, __3, _01, _12, _23, _30 }, { __0, __1, __2, __3, _01, _12, _23, _30 }, { __0, __1, __2, __3, _01, _12, _23, _30 }, /* exiting Side 0, 1, 2, 3 */ },
		/* Destination Tile YZF: */ { { XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX }, { __3, __2, __1, __0, _23, _12, _01, _30 }, { XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX }, { __3, __2, __1, __0, _23, _12, _01, _30 }, /* exiting Side 0, 1, 2, 3 */ },
		/* Destination Tile YZB: */ { { XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX }, { __3, __0, __1, __2, _30, _01, _12, _23 }, { XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX }, { __3, __0, __1, __2, _30, _01, _12, _23 }, /* exiting Side 0, 1, 2, 3 */ },
	},
	{ // Current Tile YZF:
		/* Destination Tile XYF: */ { { XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX }, { __3, __0, __1, __2, _30, _01, _12, _23 }, { XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX }, { __3, __0, __1, __2, _30, _01, _12, _23 }, /* exiting Side 0, 1, 2, 3 */ },
		/* Destination Tile XYB: */ { { XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX }, { __3, __2, __1, __0, _23, _12, _01, _30 }, { XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX }, { __3, __2, __1, __0, _23, _12, _01, _30 }, /* exiting Side 0, 1, 2, 3 */ },
		/* Destination Tile XZF: */ { { __1, __2, __3, __0, _12, _23, _30, _01 }, { XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX }, { __1, __2, __3, __0, _12, _23, _30, _01 }, { XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX }, /* exiting Side 0, 1, 2, 3 */ },
		/* Destination Tile XZB: */ { { __3, __2, __1, __0, _23, _12, _01, _30 }, { XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX }, { __3, __2, __1, __0, _23, _12, _01, _30 }, { XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX }, /* exiting Side 0, 1, 2, 3 */ },
		/* Destination Tile YZF: */ { { __0, __1, __2, __3, _01, _12, _23, _30 }, { __0, __1, __2, __3, _01, _12, _23, _30 }, { __0, __1, __2, __3, _01, _12, _23, _30 }, { __0, __1, __2, __3, _01, _12, _23, _30 }, /* exiting Side 0, 1, 2, 3 */ },
		/* Destination Tile YZB: */ { { __2, __1, __0, __3, _12, _01, _30, _23 }, { __0, __3, __2, __1, _30, _23, _12, _01 }, { __2, __1, __0, __3, _12, _01, _30, _23 }, { __0, __3, __2, __1, _30, _23, _12, _01 }, /* exiting Side 0, 1, 2, 3 */ },
	},
	{ // Current Tile YZB:
		/* Destination Tile XYF: */ { { XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX }, { __3, __2, __1, __0, _23, _12, _01, _30 }, { XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX }, { __3, __2, __1, __0, _23, _12, _01, _30 }, /* exiting Side 0, 1, 2, 3 */ },
		/* Destination Tile XYB: */ { { XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX }, { __3, __0, __1, __2, _30, _01, _12, _23 }, { XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX }, { __3, __0, __1, __2, _30, _01, _12, _23 }, /* exiting Side 0, 1, 2, 3 */ },
		/* Destination Tile XZF: */ { { __3, __2, __1, __0, _23, _12, _01, _30 }, { XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX }, { __3, __2, __1, __0, _23, _12, _01, _30 }, { XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX }, /* exiting Side 0, 1, 2, 3 */ },
		/* Destination Tile XZB: */ { { __1, __2, __3, __0, _12, _23, _30, _01 }, { XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX }, { __1, __2, __3, __0, _12, _23, _30, _01 }, { XXX, XXX, XXX, XXX, XXX, XXX, XXX, XXX }, /* exiting Side 0, 1, 2, 3 */ },
		/* Destination Tile YZF: */ { { __2, __1, __0, __3, _12, _01, _30, _23 }, { __0, __3, __2, __1, _30, _23, _12, _01 }, { __2, __1, __0, __3, _12, _01, _30, _23 }, { __0, __3, __2, __1, _30, _23, _12, _01 }, /* exiting Side 0, 1, 2, 3 */ },
		/* Destination Tile YZB: */ { { __0, __1, __2, __3, _01, _12, _23, _30 }, { __0, __1, __2, __3, _01, _12, _23, _30 }, { __0, __1, __2, __3, _01, _12, _23, _30 }, { __0, __1, __2, __3, _01, _12, _23, _30 }, /* exiting Side 0, 1, 2, 3 */ },
	}
#undef XXX
#undef __0
#undef __1
#undef __2
#undef __3
#undef _01
#undef _12
#undef _23
#undef _30
};

// Given a [local direction] and a [tile type], will return the global euclidean direction equivelant.
const GlobalAlignment LOCAL_DIR_TO_GLOBAL_DIR[6][4] = {
	{ GLOBAL_ALIGNMENT_pX, GLOBAL_ALIGNMENT_nY, GLOBAL_ALIGNMENT_nX, GLOBAL_ALIGNMENT_pY },
	{ GLOBAL_ALIGNMENT_pX, GLOBAL_ALIGNMENT_nY, GLOBAL_ALIGNMENT_nX, GLOBAL_ALIGNMENT_pY },
	{ GLOBAL_ALIGNMENT_pZ, GLOBAL_ALIGNMENT_nY, GLOBAL_ALIGNMENT_nZ, GLOBAL_ALIGNMENT_pY },
	{ GLOBAL_ALIGNMENT_pZ, GLOBAL_ALIGNMENT_nY, GLOBAL_ALIGNMENT_nZ, GLOBAL_ALIGNMENT_pY },
	{ GLOBAL_ALIGNMENT_pY, GLOBAL_ALIGNMENT_nZ, GLOBAL_ALIGNMENT_nY, GLOBAL_ALIGNMENT_pZ },
	{ GLOBAL_ALIGNMENT_pY, GLOBAL_ALIGNMENT_nZ, GLOBAL_ALIGNMENT_nY, GLOBAL_ALIGNMENT_pZ },
};

const bool HAS_COMPONENT[10][10] = {
	{ 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 0, 1, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 1, 0, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 1, 0, 0, 0, 0, 0, 0 },
	{ 1, 1, 0, 0, 1, 0, 0, 0, 0, 0 },
	{ 0, 1, 1, 0, 0, 1, 0, 0, 0, 0 },
	{ 0, 0, 1, 1, 0, 0, 1, 0, 0, 0 },
	{ 1, 0, 0, 1, 0, 0, 0, 1, 0, 0 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 1, 0 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 }
};

// TODO: change local directions to always use flags instead of enums.  I think this will help?
const uint8_t LOCAL_DIRECTION_TO_LOCAL_DIRECTION_FLAG[10] = {
	0b0001, 0b0010, 0b0100, 0b1000, 0b0011, 0b0110, 0b1100, 0b1001, 0b0000, 0b1111
};

//LocalAlignment tnav::alignmentToAlignmentMap(TileType currentTileType, TileType neighborTileType, LocalDirection exitingSide, LocalAlignment alignment)
//{
//	return ALIGNMENT_TO_ALIGNMENT_MAP[currentTileType][neighborTileType][exitingSide][alignment];
//}

// Given a local direction and a tile type, will return the global euclidean direction equivelant.
GlobalAlignment tnav::localToGlobalDir(TileType type, LocalDirection direction)
{
	return LOCAL_DIR_TO_GLOBAL_DIR[type][direction];
}

glm::vec3 tnav::globalDirToVec3(GlobalAlignment g)
{
	switch (g) {
		case GLOBAL_ALIGNMENT_pX: return glm::vec3( 1,  0,  0);
		case GLOBAL_ALIGNMENT_nX: return glm::vec3(-1,  0,  0);
		case GLOBAL_ALIGNMENT_pY: return glm::vec3( 0,  1,  0);
		case GLOBAL_ALIGNMENT_nY: return glm::vec3( 0, -1,  0);
		case GLOBAL_ALIGNMENT_pZ: return glm::vec3( 0,  0,  1);
		case GLOBAL_ALIGNMENT_nZ: return glm::vec3( 0,  0, -1);
		default: return glm::vec3(0, 0, 0);
	}
}

bool tnav::isOrthogonal(LocalDirection direction) { return direction < 4; }

bool tnav::isDiagonal(LocalDirection direction) { return 3 < direction && direction < 8; }

LocalDirection tnav::inverse(LocalAlignment alignment)
{
	int diagonalOffset = (alignment > 3) * 4;
	return LocalDirection((alignment + 2) % 4 + diagonalOffset);
}

const bool tnav::alignmentHasComponent(LocalAlignment alignment, LocalAlignment component)
{
	return HAS_COMPONENT[alignment][component];
}

const uint8_t tnav::getDirectionFlag(LocalDirection direction)
{
	return LOCAL_DIRECTION_TO_LOCAL_DIRECTION_FLAG[direction];
}

const LocalDirection tnav::getDirection(uint8_t directionFlag)
{
	switch (directionFlag) {
	case 0b0001: return LOCAL_DIRECTION_0;
	case 0b0010: return LOCAL_DIRECTION_1;
	case 0b0100: return LOCAL_DIRECTION_2;
	case 0b1000: return LOCAL_DIRECTION_3;
	case 0b0011: return LOCAL_DIRECTION_0_1;
	case 0b0110: return LOCAL_DIRECTION_1_2;
	case 0b1100: return LOCAL_DIRECTION_2_3;
	case 0b1001: return LOCAL_DIRECTION_3_0;
	case 0b0000: return LOCAL_DIRECTION_STATIC;
	case 0b0101: return LOCAL_DIRECTION_STATIC;
	case 0b1010: return LOCAL_DIRECTION_STATIC;
	default: return LOCAL_DIRECTION_ERROR;
	}
}

// ALIGNMENT_TRANSLATION_MAPS[map index][alignment to map]
// Allows for a quick way to convert direction enums from one local tile space to a neighboring one.
// Because there are only 2 axes in a 2D tile, there are only 8 ways to map x+, x-, y+, y- to each other
// with the restriction that the 'next' direction has to be +1 or -1 'away' from the previous where
// (x+, x-, y+, y-) -> (0, 2, 3, 1) (this ordering makes more sense in other contexts).
const LocalAlignment ALIGNMENT_TRANSLATION_MAPS[8][10] = {
#define _0 LOCAL_ALIGNMENT_0
#define _1 LOCAL_ALIGNMENT_1
#define _2 LOCAL_ALIGNMENT_2
#define _3 LOCAL_ALIGNMENT_3
#define _0_1 LOCAL_ALIGNMENT_0_1
#define _1_2 LOCAL_ALIGNMENT_1_2
#define _2_3 LOCAL_ALIGNMENT_2_3
#define _3_0 LOCAL_ALIGNMENT_3_0
#define NONE LOCAL_ALIGNMENT_NONE
#define ALIGNMENT_ERROR LOCAL_ALIGNMENT_ERROR
	{ _0, _1, _2, _3, _0_1, _1_2, _2_3, _3_0, NONE, ALIGNMENT_ERROR },
	{ _1, _2, _3, _0, _1_2, _2_3, _3_0, _0_1, NONE, ALIGNMENT_ERROR },
	{ _2, _3, _0, _1, _2_3, _3_0, _0_1, _1_2, NONE, ALIGNMENT_ERROR },
	{ _3, _0, _1, _2, _3_0, _0_1, _1_2, _2_3, NONE, ALIGNMENT_ERROR },
	{ _0, _3, _2, _1, _3_0, _2_3, _1_2, _0_1, NONE, ALIGNMENT_ERROR },
	{ _1, _0, _3, _2, _0_1, _3_0, _2_3, _1_2, NONE, ALIGNMENT_ERROR },
	{ _2, _1, _0, _3, _1_2, _0_1, _3_0, _2_3, NONE, ALIGNMENT_ERROR },
	{ _3, _2, _1, _0, _2_3, _1_2, _0_1, _3_0, NONE, ALIGNMENT_ERROR }
#undef a_0
#undef a_1
#undef a_2
#undef a_3
#undef a01
#undef a12
#undef a23
#undef a30
#undef NONE
#undef ERROR
};

// Given an index to a local alignment to local alignment map and an alignment, will convert that
// alignment to its mapped alignment and return it.  Map indices should be stored inside tiles as
// current tile alignments -> neighbor tile alignments maps as well as other places.
const LocalAlignment tnav::map(MapType mapType, LocalAlignment currentAlignment)
{
#ifdef RUNNING_DEBUG
	if (mapType > 7) std::cout << "INVALID ALIGNMENT TRANSLATION MAP INDEX" << std::endl;
#endif
	return ALIGNMENT_TRANSLATION_MAPS[mapType][currentAlignment];
}

// NEIGHBOR_ALIGNMENT_MAP_INDICES[currentTileEdgeIndex][connectedNeighborEdgeIndex]
const MapType NEIGHBOR_ALIGNMENT_MAP_TYPE[4][4] = {
	{ MAP_TYPE_6, MAP_TYPE_7, MAP_TYPE_0, MAP_TYPE_1 },
	{ MAP_TYPE_7, MAP_TYPE_4, MAP_TYPE_3, MAP_TYPE_0 },
	{ MAP_TYPE_0, MAP_TYPE_1, MAP_TYPE_6, MAP_TYPE_7 },
	{ MAP_TYPE_3, MAP_TYPE_0, MAP_TYPE_7, MAP_TYPE_4 }
};

// I have no idea why you dont have to account for the different starting tile types.  
// They all come out to the same map indices somehow!  
// 6x size decreas maybe due to the layout of edge indices on different tile types being a nice pattern?
const MapType tnav::getNeighborMap(LocalDirection currentToNeighbor, LocalDirection neighborToCurrent)
{
#ifdef RUNNING_DEBUG
	checkOrthogonal(connectedCurrentTileEdgeIndex);
	checkOrthogonal(connectedNeighborEdgeIndex);
#endif
	return NEIGHBOR_ALIGNMENT_MAP_TYPE[currentToNeighbor][neighborToCurrent];
}

// ALIGNMENT_MAP_COMBINATIONS[map index 0][map index 1]
const MapType COMBINE_MAP_INDICES[8][8] = {
	{ MAP_TYPE_0, MAP_TYPE_1, MAP_TYPE_2, MAP_TYPE_3, MAP_TYPE_4, MAP_TYPE_5, MAP_TYPE_6, MAP_TYPE_7 },
	{ MAP_TYPE_1, MAP_TYPE_2, MAP_TYPE_3, MAP_TYPE_0, MAP_TYPE_7, MAP_TYPE_4, MAP_TYPE_5, MAP_TYPE_6 },
	{ MAP_TYPE_2, MAP_TYPE_3, MAP_TYPE_0, MAP_TYPE_1, MAP_TYPE_6, MAP_TYPE_7, MAP_TYPE_4, MAP_TYPE_5 },
	{ MAP_TYPE_3, MAP_TYPE_0, MAP_TYPE_1, MAP_TYPE_2, MAP_TYPE_5, MAP_TYPE_6, MAP_TYPE_7, MAP_TYPE_4 },
	{ MAP_TYPE_4, MAP_TYPE_5, MAP_TYPE_6, MAP_TYPE_7, MAP_TYPE_0, MAP_TYPE_1, MAP_TYPE_2, MAP_TYPE_3 },
	{ MAP_TYPE_5, MAP_TYPE_6, MAP_TYPE_7, MAP_TYPE_4, MAP_TYPE_3, MAP_TYPE_0, MAP_TYPE_1, MAP_TYPE_2 },
	{ MAP_TYPE_6, MAP_TYPE_7, MAP_TYPE_4, MAP_TYPE_5, MAP_TYPE_2, MAP_TYPE_3, MAP_TYPE_0, MAP_TYPE_1 },
	{ MAP_TYPE_7, MAP_TYPE_4, MAP_TYPE_5, MAP_TYPE_6, MAP_TYPE_1, MAP_TYPE_2, MAP_TYPE_3, MAP_TYPE_0 }
};

const MapType tnav::combine(MapType map1, MapType map2)
{
	return COMBINE_MAP_INDICES[map1][map2];
}

const MapType tnav::inverse(MapType map)
{
	switch (map) {
	case 0: return MAP_TYPE_0;
	case 1: return MAP_TYPE_3;
	case 2: return MAP_TYPE_2;
	case 3: return MAP_TYPE_1;
	case 4: return MAP_TYPE_4;
	case 5: return MAP_TYPE_5;
	case 6: return MAP_TYPE_6;
	case 7: return MAP_TYPE_7;
	default: throw std::runtime_error("INVALID ALIGNMENT MAP INDEX IN GIVEN TO inverseAlignmentMapIndex()");
	}
}

const SuperTileType tnav::getSuperTileType(glm::ivec3 tileVert1, glm::ivec3 tileVert2, glm::ivec3 tileVert3)
{
	if (tileVert1.z == tileVert2.z && tileVert1.z == tileVert3.z) { return TILE_TYPE_XY; }
	else if (tileVert1.y == tileVert2.y && tileVert1.y == tileVert3.y) { return TILE_TYPE_XZ; }
	else /* tileVert1.x == tileVert2.x == tileVert3.x*/ { return TILE_TYPE_YZ; }
}

const SuperTileType tnav::getSuperTileType(TileType type)
{
	return SuperTileType(type / 2);
}

const TileType tnav::getTileType(SuperTileType tileMapType, bool isFront)
{
	return TileType(int(tileMapType) * 2 + int(!isFront));
}

const TileType tnav::getFrontTileType(SuperTileType tileMapType)
{
	return TileType(int(tileMapType) * 2);
}

const TileType tnav::getBackTileType(SuperTileType tileMapType)
{
	return TileType(int(tileMapType) * 2 + 1);
}

const TileType tnav::inverseTileType(TileType type)
{
	switch (type) {
	case TILE_TYPE_XYF: return TILE_TYPE_XYB;
	case TILE_TYPE_XYB: return TILE_TYPE_XYF;
	case TILE_TYPE_XZF: return TILE_TYPE_XZB;
	case TILE_TYPE_XZB: return TILE_TYPE_XZF;
	case TILE_TYPE_YZF: return TILE_TYPE_YZB;
	default: return TILE_TYPE_YZF; // This is either TILE_TYPE_YZ_BACK or you and/or I fucked up.	
	}
}

// [tile type][side index][connectable tile type (result)]
// Follows same type, N type, M type, inverse type for the last dimension!
const TileType CONNECTABLE_TILE_TYPES[6][4][4] = {
#define XYF TILE_TYPE_XYF
#define XYB TILE_TYPE_XYB
#define XZF TILE_TYPE_XZF
#define XZB TILE_TYPE_XZB
#define YZF TILE_TYPE_YZF
#define YZB TILE_TYPE_YZB
	{ { XYF, YZF, YZB, XYB }, { XYF, XZF, XZB, XYB }, { XYF, YZF, YZB, XYB }, { XYF, XZF, XZB, XYB } },
	{ { XYB, YZF, YZB, XYF }, { XYB, XZF, XZB, XYF }, { XYB, YZF, YZB, XYF }, { XYB, XZF, XZB, XYF } },
	{ { XZF, XYF, XYB, XZB }, { XZF, YZF, YZB, XZB }, { XZF, XYF, XYB, XZB }, { XZF, YZF, YZB, XZB } },
	{ { XZB, XYF, XYB, XZF }, { XZB, YZF, YZB, XZF }, { XZB, XYF, XYB, XZF }, { XZB, YZF, YZB, XZF } },
	{ { YZF, XZF, XZB, YZB }, { YZF, XYF, XYB, YZB }, { YZF, XZF, XZB, YZB }, { YZF, XYF, XYB, YZB } },
	{ { YZB, XZF, XZB, YZF }, { YZB, XYF, XYB, YZF }, { YZB, XZF, XZB, YZF }, { YZB, XYF, XYB, YZF } }
#undef XYF
#undef XYB
#undef XZF
#undef XZB
#undef YZF
#undef YZB
};

const TileType tnav::getConnectableTileType(TileType type, int orthogonalSide, int i)
{
	return CONNECTABLE_TILE_TYPES[type][orthogonalSide][i];
}



// [tile type][side index][offset to connectable tile type (result)]
// Follows { same type, N type, M type, inverse type } for the last dimension!
const glm::ivec3 CONNECTABLE_TILE_OFFSET[6][4][4] = {
#define v glm::ivec3
{	{ v(+1, +0, +0), v(+0, +0, +0), v(+0, +0, +1), v(0) }, // XYF
	{ v(+0, -1, +0), v(+0, -1, +1), v(+0, -1, +0), v(0) },
	{ v(-1, +0, +0), v(-1, +0, +1), v(-1, +0, +0), v(0) },
	{ v(+0, +1, +0), v(+0, +0, +0), v(+0, +0, +1), v(0) },
},{ { v(+1, +0, +0), v(+0, +0, +1), v(+0, +0, +0), v(0) }, // XYB
	{ v(+0, -1, +0), v(+0, -1, +0), v(+0, -1, +1), v(0) },
	{ v(-1, +0, +0), v(-1, +0, +0), v(-1, +0, +1), v(0) },
	{ v(+0, +1, +0), v(+0, +0, +1), v(+0, +0, +0), v(0) },
},{ { v(+0, +0, +1), v(+0, +0, +0), v(+0, +1, +0), v(0) }, // XZF
	{ v(-1, +0, +0), v(-1, +1, +0), v(-1, +0, +0), v(0) },
	{ v(+0, +0, -1), v(+0, +1, -1), v(+0, +0, -1), v(0) },
	{ v(+1, +0, +0), v(+0, +0, +0), v(+0, +1, +0), v(0) },
},{ { v(+0, +0, +1), v(-1, +0, +0), v(-1, +0, +0), v(0) }, // XZB
	{ v(-1, +1, +0), v(+0, +0, -1), v(+0, +0, -1), v(0) },
	{ v(+0, +1, -1), v(+1, +0, +0), v(+0, +1, +0), v(0) },
	{ v(+0, +0, +0), v(+0, +1, +0), v(+0, +0, +0), v(0) },
},{ { v(+1, +0, +0), v(+0, +0, -1), v(+1, +0, -1), v(0) }, // YZF
	{ v(+0, +0, -1), v(+0, +1, +0), v(+0, +0, +0), v(0) },
	{ v(+0, -1, +0), v(+1, -1, +0), v(+0, -1, +0), v(0) },
	{ v(+0, +0, +1), v(+0, +0, +0), v(+1, +0, +0), v(0) },
},{ { v(+0, +1, +0), v(+1, +0, +0), v(+0, +0, +0), v(0) }, // YZB
	{ v(+0, +0, -1), v(+0, +0, -1), v(+1, +0, -1), v(0) },
	{ v(+0, -1, +0), v(+0, -1, +0), v(+1, -1, +0), v(0) },
	{ v(+0, +0, +1), v(+1, +0, +0), v(+0, +0, +0), v(0) }, 
}
#undef v
};

const glm::ivec3 tnav::getConnectableTileOffset(TileType type, int orthogonalSide, int i)
{
	return CONNECTABLE_TILE_OFFSET[type][orthogonalSide][i];
}

// Returns an integer from 0 (never visible) to 4 (most visible) when comparing two tile subtypes 
// from the perspective of a certain side.  Make sure to input into the array as follows:
// TILE_VISIBILITY[SUBJECT TILE SUB TYPE][SUBJECT TILE SIDE INDEX][COMPARE TILE SUB TYPE]
const int TILE_VISIBILITY[6][4][6] = {
#define X 0
	{ { 3, 1, X, X, 2, 4 }, { 3, 1, 4, 2, X, X }, { 3, 1, X, X, 4, 2 }, { 3, 1, 2, 4, X, X } }, // XYF
	{ { 1, 3, X, X, 2, 4 }, { 1, 3, 4, 2, X, X }, { 1, 3, X, X, 4, 2 }, { 1, 3, 2, 4, X, X } }, // XYB
	{ { 2, 4, 3, 1, X, X }, { X, X, 3, 1, 4, 2 }, { 4, 2, 3, 1, X, X }, { X, X, 3, 1, 2, 4 } }, // XZF
	{ { 2, 4, 1, 3, X, X }, { X, X, 1, 3, 4, 2 }, { 4, 2, 1, 3, X, X }, { X, X, 1, 3, 2, 4 } }, // XZB
	{ { X, X, 2, 4, 3, 1 }, { 4, 2, X, X, 3, 1 }, { X, X, 4, 2, 3, 1 }, { 2, 4, X, X, 3, 1 } }, // YZF
	{ { X, X, 2, 4, 1, 3 }, { 4, 2, X, X, 1, 3 }, { X, X, 4, 2, 1, 3 }, { 2, 4, X, X, 1, 3 } }  // YZB
#undef X
};

const int tnav::getTileVisibility(TileType subjetTileType, LocalDirection orthoSide, TileType otherTileType)
{
	return TILE_VISIBILITY[subjetTileType][orthoSide][otherTileType];
}

const glm::vec3 tnav::getNormal(TileType type)
{
	switch (type) {
	case TILE_TYPE_XYF: return glm::vec3(0, 0, 1);
	case TILE_TYPE_XYB: return glm::vec3(0, 0, -1);
	case TILE_TYPE_XZF: return glm::vec3(0, 1, 0);
	case TILE_TYPE_XZB: return glm::vec3(0, -1, 0);
	case TILE_TYPE_YZF: return glm::vec3(1, 0, 0);
	case TILE_TYPE_YZB: return glm::vec3(-1, 0, 0);
	default: throw std::runtime_error("Tile sub type is ERROR on getNormal()!");
	}
}

const TileType tnav::getTileType(glm::vec3 normal)
{
	if (normal == glm::vec3(0, 0, 1)) { return TILE_TYPE_XYF; }
	else if (normal == glm::vec3(0, 0, -1)) { return TILE_TYPE_XYB; }
	else if (normal == glm::vec3(0, 1, 0)) { return TILE_TYPE_XZF; }
	else if (normal == glm::vec3(0, -1, 0)) { return TILE_TYPE_XZB; }
	else if (normal == glm::vec3(1, 0, 0)) { return TILE_TYPE_YZF; }
	else if (normal == glm::vec3(-1, 0, 0)) { return TILE_TYPE_YZB; }
	throw std::runtime_error("Given normal does not corrispond to a tile type!");
}


const LocalAlignment tnav::getOtherComponent(LocalAlignment diagonal, LocalAlignment component)
{
	switch (diagonal) {
	case LOCAL_ALIGNMENT_0_1:
		switch (component) {
		case LOCAL_ALIGNMENT_0: return LOCAL_ALIGNMENT_1;
		case LOCAL_ALIGNMENT_1: return LOCAL_ALIGNMENT_0;
		default: return LOCAL_ALIGNMENT_ERROR;
		}
	case LOCAL_ALIGNMENT_1_2:
		switch (component) {
		case LOCAL_ALIGNMENT_1: return LOCAL_ALIGNMENT_2;
		case LOCAL_ALIGNMENT_2: return LOCAL_ALIGNMENT_1;
		default: return LOCAL_ALIGNMENT_ERROR;
		}
	case LOCAL_ALIGNMENT_2_3:
		switch (component) {
		case LOCAL_ALIGNMENT_2: return LOCAL_ALIGNMENT_3;
		case LOCAL_ALIGNMENT_3: return LOCAL_ALIGNMENT_2;
		default: return LOCAL_ALIGNMENT_ERROR;
		}
	case LOCAL_ALIGNMENT_3_0:
		switch (component) {
		case LOCAL_ALIGNMENT_3: return LOCAL_ALIGNMENT_0;
		case LOCAL_ALIGNMENT_0: return LOCAL_ALIGNMENT_3;
		default: return LOCAL_ALIGNMENT_ERROR;
		}
	default: return LOCAL_ALIGNMENT_ERROR;
	}
}

const glm::vec3 tnav::TO_NODE_OFFSETS[3][8] = {
	{ 
		/* sides:   */ glm::vec3(0.5f, 0.0f, 0.0f), glm::vec3(0.0f, -0.5f, 0.0f), glm::vec3(-0.5f, 0.0f, 0.0f), glm::vec3(0.0f, 0.5f, 0.0f),
		/* corners: */ glm::vec3(0.5f, 0.5f, 0.0f), glm::vec3(0.5f, -0.5f, 0.0f), glm::vec3(-0.5f, -0.5f, 0.0f), glm::vec3(-0.5f, 0.5f, 0.0f)
	},
	{
		/* sides:   */ glm::vec3(0.0f, 0.0f, 0.5f), glm::vec3(-0.5f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -0.5f), glm::vec3(0.5f, 0.0f, 0.0f),
		/* corners: */ glm::vec3(0.5f, 0.0f, 0.5f), glm::vec3(-0.5f, 0.0f, 0.5f), glm::vec3(-0.5f, 0.0f, -0.5f), glm::vec3(0.5f, 0.0f, -0.5f)
	},
	{
		/* sides:   */ glm::vec3(0.0f, 0.5f, 0.0f), glm::vec3(0.0f, 0.0f, -0.5f), glm::vec3(0.0f, -0.5f, 0.0f), glm::vec3(0.0f, 0.0f, 0.5f),
		/* corners: */ glm::vec3(0.0f, 0.5f, 0.5f), glm::vec3(0.0f, 0.5f, -0.5f), glm::vec3(0.0f, -0.5f, -0.5f), glm::vec3(0.0f, -0.5f, 0.5f)
	},
};

const glm::vec3* tnav::getNodePositionOffsets(SuperTileType type)
{
	return TO_NODE_OFFSETS[type];
}


const glm::vec3 tnav::getCenterToNeighborVec(TileType type, LocalDirection dir)
{
	static const glm::vec3 TILE_CENTER_TO_SIDE[6][8] = {
		{ // XYF:
			glm::vec3(1, 0, 0),
			glm::vec3(0, -1, 0),
			glm::vec3(-1, 0, 0),
			glm::vec3(0, 1, 0),
			glm::vec3(1, -1, 0),
			glm::vec3(-1, -1, 0),
			glm::vec3(-1, 1, 0),
			glm::vec3(1, 1, 0),
		}, { // XYB:
			glm::vec3(1, 0, 0),
			glm::vec3(0, -1, 0),
			glm::vec3(-1, 0, 0),
			glm::vec3(0, 1, 0),
			glm::vec3(1, -1, 0),
			glm::vec3(-1, -1, 0),
			glm::vec3(-1, 1, 0),
			glm::vec3(1, 1, 0) 
		}, { // XZF:
			glm::vec3(0, 0, 1),
			glm::vec3(-1, 0, 0),
			glm::vec3(0, 0, -1),
			glm::vec3(1, 0, 0) ,
			glm::vec3(-1, 0, 1) ,
			glm::vec3(-1, 0, -1) ,
			glm::vec3(1, 0, -1) ,
			glm::vec3(1, 0, 1) ,
		}, { // XZB:
			glm::vec3(0, 0, 1),
			glm::vec3(-1, 0, 0),
			glm::vec3(0, 0, -1),
			glm::vec3(1, 0, 0),
			glm::vec3(-1, 0, 1) ,
			glm::vec3(-1, 0, -1) ,
			glm::vec3(1, 0, -1) ,
			glm::vec3(1, 0, 1) ,
		}, { // YZF:
			glm::vec3(0, 1, 0),
			glm::vec3(0, 0, -1),
			glm::vec3(0, -1, 0),
			glm::vec3(0, 0, 1),
			glm::vec3(0, 1, -1),
			glm::vec3(0, -1, -1),
			glm::vec3(0, -1, 1),
			glm::vec3(0, 1, 1),
		}, { // YZB:
			glm::vec3(0, 1, 0),
			glm::vec3(0, 0, -1),
			glm::vec3(0, -1, 0),
			glm::vec3(0, 0, 1),
			glm::vec3(0, 1, -1),
			glm::vec3(0, -1, -1),
			glm::vec3(0, -1, 1),
			glm::vec3(0, 1, 1),
		},
	};

	return TILE_CENTER_TO_SIDE[type][dir];
}
