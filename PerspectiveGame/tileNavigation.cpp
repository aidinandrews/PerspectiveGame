#include "tileNavigation.h"

// Given a [tile type] and a [meta alignment], will return the matching local alignment:
const LocalAlignment META_TO_LOCAL_ALIGNMENT[6][19] = {
	// Define some macros to make the below array more legible:
	#define __0__ LOCAL_ALIGNMENT_0
	#define __1__ LOCAL_ALIGNMENT_1
	#define __2__ LOCAL_ALIGNMENT_2
	#define __3__ LOCAL_ALIGNMENT_3
	#define _0_1_ LOCAL_ALIGNMENT_01
	#define _1_2_ LOCAL_ALIGNMENT_12
	#define _2_3_ LOCAL_ALIGNMENT_23
	#define _3_0_ LOCAL_ALIGNMENT_30
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
const LocalDirection DIRECTION_COMPONENTS[8][2] = {
	// Orthogonal:
	{ LOCAL_DIRECTION_0, LOCAL_DIRECTION_0 }, // LOCAL_DIRECTION_0
	{ LOCAL_DIRECTION_1, LOCAL_DIRECTION_1 }, // LOCAL_DIRECTION_1
	{ LOCAL_DIRECTION_2, LOCAL_DIRECTION_2 }, // LOCAL_DIRECTION_2
	{ LOCAL_DIRECTION_3, LOCAL_DIRECTION_3 }, // LOCAL_DIRECTION_3
	// Diagonal:
	{ LOCAL_DIRECTION_0, LOCAL_DIRECTION_1 }, // LOCAL_DIRECTION_01
	{ LOCAL_DIRECTION_1, LOCAL_DIRECTION_2 }, // LOCAL_DIRECTION_12
	{ LOCAL_DIRECTION_2, LOCAL_DIRECTION_3 }, // LOCAL_DIRECTION_23
	{ LOCAL_DIRECTION_3, LOCAL_DIRECTION_0 }  // LOCAL_DIRECTION_30
};

const LocalDirection* tnav::localDirectionComponents(LocalDirection direction)
{
	return DIRECTION_COMPONENTS[direction];
}

// Given two orthogonal LocalDirections, combines them into an orthogonal or diagonal direction.
// Directions facing opposite ways are combined into the static enum.
const LocalDirection COMBINED_DIRECTIONS[4][4] = {
	{ LOCAL_DIRECTION_0, LOCAL_DIRECTION_01, LOCAL_DIRECTION_STATIC, LOCAL_DIRECTION_30 },
	{ LOCAL_DIRECTION_01, LOCAL_DIRECTION_1, LOCAL_DIRECTION_12, LOCAL_DIRECTION_STATIC },
	{ LOCAL_DIRECTION_STATIC, LOCAL_DIRECTION_12, LOCAL_DIRECTION_2, LOCAL_DIRECTION_23 },
	{ LOCAL_DIRECTION_30, LOCAL_DIRECTION_STATIC, LOCAL_DIRECTION_23, LOCAL_DIRECTION_3 }
};

const LocalDirection tnav::combineLocalDirections(LocalDirection direction1, LocalDirection direction2)
{
	return COMBINED_DIRECTIONS[direction1][direction2];
}

const LocalPosition NEXT_LOCAL_POSITIONS[9][8] = {
#define _0__ LOCAL_POSITION_0
#define _1__ LOCAL_POSITION_1
#define _2__ LOCAL_POSITION_2
#define _3__ LOCAL_POSITION_3
#define _01_ LOCAL_POSITION_01
#define _12_ LOCAL_POSITION_12
#define _23_ LOCAL_POSITION_23
#define _30_ LOCAL_POSITION_30
#define CENT LOCAL_POSITION_CENTER
#define XXXX LOCAL_DIRECTION_ERROR
	{ XXXX, _01_, CENT, _30_, XXXX, _1__, _3__, XXXX },
	{ _01_, XXXX, _2__, _3__, XXXX, XXXX, _2__, _0__ },
	{ CENT, _12_, XXXX, _23_, _1__, XXXX, XXXX, _3__ },
	{ _30_, CENT, _23_, XXXX, _0__, _2__, XXXX, XXXX },
	{ XXXX, XXXX, _1__, _0__, XXXX, XXXX, CENT, XXXX },
	{ _1__, XXXX, XXXX, _2__, XXXX, XXXX, XXXX, CENT },
	{ _3__, _2__, XXXX, XXXX, CENT, XXXX, XXXX, XXXX },
	{ XXXX, _0__, _3__, XXXX, XXXX, CENT, XXXX, XXXX },
	{ _0__, _1__, _2__, _3__, _01_, _12_, _23_, _30_ }
#undef _0__
#undef _1__
#undef _2__
#undef _3__
#undef _01_
#undef _12_
#undef _23_
#undef _30_
#undef CENT
#undef XXXX
};

LocalPosition tnav::nextPosition(LocalPosition position, LocalDirection direction)
{
	return NEXT_LOCAL_POSITIONS[position][direction];
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
* EXAMPLE: If the sub-tile area index == 1 and direction == LOCAL_DIRECTION_12, movementInfo will need to be shifted
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
const LocalDirection ALIGNMENT_TO_ALIGNMENT_MAP[6][6][4][8] = {
#define XXX LOCAL_DIRECTION_ERROR
#define __0 LOCAL_DIRECTION_0
#define __1 LOCAL_DIRECTION_1
#define __2 LOCAL_DIRECTION_2
#define __3 LOCAL_DIRECTION_3
#define _01 LOCAL_DIRECTION_01
#define _12 LOCAL_DIRECTION_12
#define _23 LOCAL_DIRECTION_23
#define _30 LOCAL_DIRECTION_30
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

LocalAlignment tnav::alignmentToAlignmentMap(TileSubType currentTileType, TileSubType neighborTileType, LocalDirection exitingSide, LocalAlignment alignment)
{
	return ALIGNMENT_TO_ALIGNMENT_MAP[currentTileType][neighborTileType][exitingSide][alignment];
}

LocalPosition tnav::positionToPositionMap(TileSubType currentTileType, TileSubType neighborTileType, LocalDirection exitingDirection, LocalPosition position)
{
	return ALIGNMENT_TO_ALIGNMENT_MAP[currentTileType][neighborTileType][exitingDirection][position];
}

LocalDirection tnav::directionToDirectionMap(TileSubType currentTileType, TileSubType neighborTileType, LocalDirection exitingDirection, LocalDirection direction)
{
	return ALIGNMENT_TO_ALIGNMENT_MAP[currentTileType][neighborTileType][exitingDirection][direction];
}

// Returns the adjusted relative orientation of an entity/basis when going from one tile to another.
LocalOrientation tnav::orientationToOrientationMap(TileSubType currentTileType, TileSubType neighborTileType, LocalDirection exitingSide, LocalOrientation currentOrientation)
{
	return ALIGNMENT_TO_ALIGNMENT_MAP[currentTileType][neighborTileType][exitingSide][currentOrientation];
}

// Given a local direction and a tile type, will return the global euclidean direction equivelant.
GlobalAlignment tnav::localToGlobalDir(TileSubType type, LocalDirection direction)
{
	return LOCAL_DIR_TO_GLOBAL_DIR[type][direction];
}

const static int* tnav::localPositionToSubTileIndices(LocalPosition position)
{
	return LOCAL_POSITION_TO_SUB_TILE_AREA_INDICES[position];
}

const static int tnav::nextSubTileIndex(int subTileIndex, LocalDirection direction)
{
	return NEXT_SUB_TILE_AREA_INDICES[subTileIndex][direction];
}

const static int tnav::nextSubTileShift(int subTileIndex, LocalDirection direction)
{
	return NEXT_SUB_TILE_AREA_SHIFTS[subTileIndex][direction];
}

const int* tnav::getSurroundingSubTileIndices(LocalPosition position)
{
	return LOCAL_POSITION_TO_SUB_TILE_AREA_INDICES[position];
}

bool tnav::isOrthogonal(LocalDirection direction) { return direction < 4; }

bool tnav::isDiagonal(LocalDirection direction) { return 3 < direction && direction < 8; }

LocalDirection tnav::oppositeDirection(LocalDirection currentDirection)
{
	int diagonalOffset = (currentDirection > 3) * 4;
	return LocalDirection((currentDirection + 2) % 4 + diagonalOffset);
}

const bool tnav::localDirectionHasComponent(LocalDirection direction, LocalDirection component)
{
	return HAS_COMPONENT[direction][component];
}

const uint8_t tnav::getLocalDirectionFlag(LocalDirection direction)
{
	return LOCAL_DIRECTION_TO_LOCAL_DIRECTION_FLAG[direction];
}

const LocalDirection tnav::getLocalDirection(uint8_t directionFlag)
{
	switch (directionFlag) {
	case 0b0001: return LOCAL_DIRECTION_0;
	case 0b0010: return LOCAL_DIRECTION_1;
	case 0b0100: return LOCAL_DIRECTION_2;
	case 0b1000: return LOCAL_DIRECTION_3;
	case 0b0011: return LOCAL_DIRECTION_01;
	case 0b0110: return LOCAL_DIRECTION_12;
	case 0b1100: return LOCAL_DIRECTION_23;
	case 0b1001: return LOCAL_DIRECTION_30;
	case 0b0000: return LOCAL_DIRECTION_STATIC;
	case 0b0101: return LOCAL_DIRECTION_STATIC;
	case 0b1010: return LOCAL_DIRECTION_STATIC;
	case 0b1111: return LOCAL_DIRECTION_ERROR;
	}
}