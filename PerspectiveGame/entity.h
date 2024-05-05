#pragma once

struct Entity {
	enum Type {
		NONE,
		RED_CIRCLE,
		BLUE_CIRCLE,
	};

	Type type;
	float offset;
	int offsetSide; // == tile.sideInfos index.
};