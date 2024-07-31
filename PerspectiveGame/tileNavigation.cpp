#include "tileNavigation.h"

// Returns the adjusted local orientation of an entity/basis when going from one tile to another.
const LocalDirection TileNavigator::ORIENTATION_TO_ORIENTATION_MAP[6][6][4][4] = {
#define _X LOCAL_DIRECTION_INVALID
#define _0 LOCAL_DIRECTION_0
#define _1 LOCAL_DIRECTION_1
#define _2 LOCAL_DIRECTION_2
#define _3 LOCAL_DIRECTION_3
	{ // Current Tile XYF:
		/* Destination Tile XYF: */ { { _0, _1, _2, _3 },{ _0, _1, _2, _3 },{ _0, _1, _2, _3 },{ _0, _1, _2, _3 }  /* exiting Side 0, 1, 2, 3 */ },
		/* Destination Tile XYB: */ { { _2, _1, _0, _3 },{ _0, _3, _2, _1 },{ _2, _1, _0, _3 },{ _0, _3, _2, _1 }  /* exiting Side 0, 1, 2, 3 */ },
		/* Destination Tile XZF: */ { { _X, _X, _X, _X },{ _3, _0, _1, _2 },{ _X, _X, _X, _X },{ _3, _0, _1, _2 }  /* exiting Side 0, 1, 2, 3 */ },
		/* Destination Tile XZB: */ { { _X, _X, _X, _X },{ _3, _2, _1, _0 },{ _X, _X, _X, _X },{ _3, _2, _1, _0 }  /* exiting Side 0, 1, 2, 3 */ },
		/* Destination Tile YZF: */ { { _1, _2, _3, _0 },{ _X, _X, _X, _X },{ _1, _2, _3, _0 },{ _X, _X, _X, _X }  /* exiting Side 0, 1, 2, 3 */ },
		/* Destination Tile YZB: */ { { _3, _2, _1, _0 },{ _X, _X, _X, _X },{ _3, _2, _1, _0 },{ _X, _X, _X, _X }  /* exiting Side 0, 1, 2, 3 */ }
	},
	{ // Current Tile = _XYB
		/* Destination Tile XYF: */ { { _2, _1, _0, _3 },{ _0, _3, _2, _1 },{ _2, _1, _0, _3 },{ _0, _3, _2, _1 } /* exiting Side 0, 1, 2, 3 */ },
		/* Destination Tile XYB: */ { { _0, _1, _2, _3 },{ _0, _1, _2, _3 },{ _0, _1, _2, _3 },{ _0, _1, _2, _3 } /* exiting Side 0, 1, 2, 3 */ },
		/* Destination Tile XZF: */ { { _X, _X, _X, _X },{ _3, _2, _1, _0 },{ _X, _X, _X, _X },{ _3, _2, _1, _0 } /* exiting Side 0, 1, 2, 3 */ },
		/* Destination Tile XZB: */ { { _X, _X, _X, _X },{ _3, _0, _1, _2 },{ _X, _X, _X, _X },{ _3, _0, _1, _2 } /* exiting Side 0, 1, 2, 3 */ },
		/* Destination Tile YZF: */ { { _3, _2, _1, _0 },{ _X, _X, _X, _X },{ _3, _2, _1, _0 },{ _X, _X, _X, _X } /* exiting Side 0, 1, 2, 3 */ },
		/* Destination Tile YZB: */ { { _1, _2, _3, _0 },{ _X, _X, _X, _X },{ _1, _2, _3, _0 },{ _X, _X, _X, _X } /* exiting Side 0, 1, 2, 3 */ }
	},
	{ // Current Tile = _XZF
		/* Destination Tile XYF: */ { { _1, _2, _3, _0 },{ _X, _X, _X, _X },{ _1, _2, _3, _0 },{ _X, _X, _X, _X } /* exiting Side 0, 1, 2, 3 */ },
		/* Destination Tile XYB: */ { { _3, _2, _1, _0 },{ _X, _X, _X, _X },{ _3, _2, _1, _0 },{ _X, _X, _X, _X } /* exiting Side 0, 1, 2, 3 */ },
		/* Destination Tile XZF: */ { { _0, _1, _2, _3 },{ _0, _1, _2, _3 },{ _0, _1, _2, _3 },{ _0, _1, _2, _3 } /* exiting Side 0, 1, 2, 3 */ },
		/* Destination Tile XZB: */ { { _2, _1, _0, _3 },{ _0, _3, _2, _1 },{ _2, _1, _0, _3 },{ _0, _3, _2, _1 } /* exiting Side 0, 1, 2, 3 */ },
		/* Destination Tile YZF: */ { { _X, _X, _X, _X },{ _3, _0, _1, _2 },{ _X, _X, _X, _X },{ _3, _0, _1, _2 } /* exiting Side 0, 1, 2, 3 */ },
		/* Destination Tile YZB: */ { { _X, _X, _X, _X },{ _3, _2, _1, _0 },{ _X, _X, _X, _X },{ _3, _2, _1, _0 } /* exiting Side 0, 1, 2, 3 */ }
	},
	{ // Current Tile = _XZB
		/* Destination Tile XYF: */ { { _3, _2, _1, _0 },{ _X, _X, _X, _X },{ _3, _2, _1, _0 },{ _X, _X, _X, _X } /* exiting Side 0, 1, 2, 3 */ },
		/* Destination Tile XYB: */ { { _1, _2, _3, _0 },{ _X, _X, _X, _X },{ _1, _2, _3, _0 },{ _X, _X, _X, _X } /* exiting Side 0, 1, 2, 3 */ },
		/* Destination Tile XZF: */ { { _2, _1, _0, _3 },{ _0, _3, _2, _1 },{ _2, _1, _0, _3 },{ _0, _3, _2, _1 } /* exiting Side 0, 1, 2, 3 */ },
		/* Destination Tile XZB: */ { { _0, _1, _2, _3 },{ _0, _1, _2, _3 },{ _0, _1, _2, _3 },{ _0, _1, _2, _3 } /* exiting Side 0, 1, 2, 3 */ },
		/* Destination Tile YZF: */ { { _X, _X, _X, _X },{ _3, _2, _1, _0 },{ _X, _X, _X, _X },{ _3, _2, _1, _0 } /* exiting Side 0, 1, 2, 3 */ },
		/* Destination Tile YZB: */ { { _X, _X, _X, _X },{ _3, _0, _1, _2 },{ _X, _X, _X, _X },{ _3, _0, _1, _2 } /* exiting Side 0, 1, 2, 3 */ }
	},
	{ // Current Tile = YZF
		/* Destination Tile XYF: */ { { _X, _X, _X, _X },{ _3, _0, _1, _2 },{ _X, _X, _X, _X },{ _3, _0, _1, _2 } /* exiting Side 0, 1, 2, 3 */ },
		/* Destination Tile XYB: */ { { _X, _X, _X, _X },{ _3, _2, _1, _0 },{ _X, _X, _X, _X },{ _3, _2, _1, _0 } /* exiting Side 0, 1, 2, 3 */ },
		/* Destination Tile XZF: */ { { _1, _2, _3, _0 },{ _X, _X, _X, _X },{ _1, _2, _3, _0 },{ _X, _X, _X, _X } /* exiting Side 0, 1, 2, 3 */ },
		/* Destination Tile XZB: */ { { _3, _2, _1, _0 },{ _X, _X, _X, _X },{ _3, _2, _1, _0 },{ _X, _X, _X, _X } /* exiting Side 0, 1, 2, 3 */ },
		/* Destination Tile YZF: */ { { _0, _1, _2, _3 },{ _0, _1, _2, _3 },{ _0, _1, _2, _3 },{ _0, _1, _2, _3 } /* exiting Side 0, 1, 2, 3 */ },
		/* Destination Tile YZB: */ { { _2, _1, _0, _3 },{ _0, _3, _2, _1 },{ _2, _1, _0, _3 },{ _0, _3, _2, _1 } /* exiting Side 0, 1, 2, 3 */ }
	},
	{ // Current Tile = YZB
		/* Destination Tile XYF: */ { { _X, _X, _X, _X },{ _3, _2, _1, _0 },{ _X, _X, _X, _X },{ _3, _2, _1, _0 } /* exiting Side 0, 1, 2, 3 */ },
		/* Destination Tile XYB: */ { { _X, _X, _X, _X },{ _3, _0, _1, _2 },{ _X, _X, _X, _X },{ _3, _0, _1, _2 } /* exiting Side 0, 1, 2, 3 */ },
		/* Destination Tile XZF: */ { { _3, _2, _1, _0 },{ _X, _X, _X, _X },{ _3, _2, _1, _0 },{ _X, _X, _X, _X } /* exiting Side 0, 1, 2, 3 */ },
		/* Destination Tile XZB: */ { { _1, _2, _3, _0 },{ _X, _X, _X, _X },{ _1, _2, _3, _0 },{ _X, _X, _X, _X } /* exiting Side 0, 1, 2, 3 */ },
		/* Destination Tile YZF: */ { { _2, _1, _0, _3 },{ _0, _3, _2, _1 },{ _2, _1, _0, _3 },{ _0, _3, _2, _1 } /* exiting Side 0, 1, 2, 3 */ },
		/* Destination Tile YZB: */ { { _0, _1, _2, _3 },{ _0, _1, _2, _3 },{ _0, _1, _2, _3 },{ _0, _1, _2, _3 } /* exiting Side 0, 1, 2, 3 */ }
	}
#undef _X
#undef _0
#undef _1
#undef _2
#undef _3
};

// Given a local direction and a tile type, will return the global euclidean direction equivelant.
const GlobalDirection TileNavigator::LOCAL_DIR_TO_GLOBAL_DIR[6][4] = {
	{ GlobalDirection::X_POSITIVE, GlobalDirection::Y_NEGATIVE, GlobalDirection::X_NEGATIVE, GlobalDirection::Y_POSITIVE },
	{ GlobalDirection::X_POSITIVE, GlobalDirection::Y_NEGATIVE, GlobalDirection::X_NEGATIVE, GlobalDirection::Y_POSITIVE },
	{ GlobalDirection::Z_POSITIVE, GlobalDirection::Y_NEGATIVE, GlobalDirection::Z_NEGATIVE, GlobalDirection::Y_POSITIVE },
	{ GlobalDirection::Z_POSITIVE, GlobalDirection::Y_NEGATIVE, GlobalDirection::Z_NEGATIVE, GlobalDirection::Y_POSITIVE },
	{ GlobalDirection::Y_POSITIVE, GlobalDirection::Z_NEGATIVE, GlobalDirection::Y_NEGATIVE, GlobalDirection::Z_POSITIVE },
	{ GlobalDirection::Y_POSITIVE, GlobalDirection::Z_NEGATIVE, GlobalDirection::Y_NEGATIVE, GlobalDirection::Z_POSITIVE },
};

const LocalPosition TileNavigator::NEXT_LOCAL_POSITION[9][4] = {
	// Starting Positions:
	{ LOCAL_POSITION_INVALID,  LOCAL_POSITION_INVALID,  LOCAL_POSITION_MIDDLE_0, LOCAL_POSITION_INVALID  }, // Edge   0
	{ LOCAL_POSITION_INVALID,  LOCAL_POSITION_INVALID,  LOCAL_POSITION_INVALID,  LOCAL_POSITION_MIDDLE_1 }, // Edge   1
	{ LOCAL_POSITION_MIDDLE_2, LOCAL_POSITION_INVALID,  LOCAL_POSITION_INVALID,  LOCAL_POSITION_INVALID  }, // Edge   2
	{ LOCAL_POSITION_INVALID,  LOCAL_POSITION_MIDDLE_3, LOCAL_POSITION_INVALID,  LOCAL_POSITION_INVALID  }, // Edge   3
	{ LOCAL_POSITION_EDGE_0,   LOCAL_POSITION_INVALID,  LOCAL_POSITION_CENTER,   LOCAL_POSITION_INVALID  }, // Middle 0
	{ LOCAL_POSITION_INVALID,  LOCAL_POSITION_EDGE_1,   LOCAL_POSITION_INVALID,  LOCAL_POSITION_CENTER   }, // Middle 1
	{ LOCAL_POSITION_CENTER,   LOCAL_POSITION_INVALID,  LOCAL_POSITION_EDGE_2,   LOCAL_POSITION_INVALID  }, // Middle 2
	{ LOCAL_POSITION_INVALID,  LOCAL_POSITION_CENTER,   LOCAL_POSITION_INVALID,  LOCAL_POSITION_EDGE_3   }, // Middle 3
	{ LOCAL_POSITION_MIDDLE_0, LOCAL_POSITION_MIDDLE_1, LOCAL_POSITION_MIDDLE_2, LOCAL_POSITION_MIDDLE_3 }, // Center
};

const uint16_t TileNavigator::ENTITY_LOCAL_POSITION_TO_OBSTRUCTION_MASK[10] = {
	0b0000000100010000, // Edge   0
	0b0000000000000110, // Edge   1
	0b0000100010000000, // Edge   2
	0b0110000000000000, // Edge   3
	0b0000001100110000, // Middle 0
	0b0000000001100110, // Middle 1
	0b0000110011000000, // Middle 2
	0b0110011000000000, // Middle 3
	0b0000011001100000, // Center
	0b1111111111111111, // Invalid
};

void TileNavigator::print(LocalDirection d) {
	switch (d) {
	case LOCAL_DIRECTION_0:       std::cout << "LOCAL_DIRECTION_0" << std::endl; break;
	case LOCAL_DIRECTION_1:       std::cout << "LOCAL_DIRECTION_1" << std::endl; break;
	case LOCAL_DIRECTION_2:       std::cout << "LOCAL_DIRECTION_2" << std::endl; break;
	case LOCAL_DIRECTION_3:       std::cout << "LOCAL_DIRECTION_3" << std::endl; break;
	case LOCAL_DIRECTION_STATIC:  std::cout << "LOCAL_DIRECTION_STATIC" << std::endl; break;
	case LOCAL_DIRECTION_INVALID: std::cout << "LOCAL_DIRECTION_INVALID" << std::endl; break;
	default:                      std::cout << "OUT OF SCOPE" << std::endl;
	}
}

void TileNavigator::print(LocalPosition p) {
	switch (p) {
	case LOCAL_POSITION_EDGE_0:   std::cout << "LOCAL_POSITION_EDGE_0" << std::endl; break;
	case LOCAL_POSITION_EDGE_1:	  std::cout << "LOCAL_POSITION_EDGE_1" << std::endl; break;
	case LOCAL_POSITION_EDGE_2:	  std::cout << "LOCAL_POSITION_EDGE_2" << std::endl; break;
	case LOCAL_POSITION_EDGE_3:	  std::cout << "LOCAL_POSITION_EDGE_3" << std::endl; break;
	case LOCAL_POSITION_MIDDLE_0: std::cout << "LOCAL_POSITION_MIDDLE_0" << std::endl; break;
	case LOCAL_POSITION_MIDDLE_1: std::cout << "LOCAL_POSITION_MIDDLE_1" << std::endl; break;
	case LOCAL_POSITION_MIDDLE_2: std::cout << "LOCAL_POSITION_MIDDLE_2" << std::endl; break;
	case LOCAL_POSITION_MIDDLE_3: std::cout << "LOCAL_POSITION_MIDDLE_3" << std::endl; break;
	case LOCAL_POSITION_CENTER:	  std::cout << "LOCAL_POSITION_CENTER" << std::endl; break;
	case LOCAL_POSITION_INVALID:  std::cout << "LOCAL_POSITION_INVALID" << std::endl; break;
	default:                      std::cout << "OUT OF SCOPE" << std::endl;
	}
}