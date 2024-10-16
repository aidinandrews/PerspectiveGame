#pragma once
#include <iostream>

#include "tile.h"

struct Player {
	struct PlayerGpuInfo {
		alignas(16) int tileIndices[4];
		// relativePos is relative to the tile such that:
		// vert[2] -> vert[1] == +x && vert[2] -> vert[3] == +y. 
		// Note: vert[0] == initialVert().
		alignas(32) glm::vec2 relativePos[4]; 
		alignas(16) glm::vec4 color;
	};

	Camera *p_camera;

	TileTarget* povTileTarget;
	glm::vec2 pos; // Relative to the povTile.
	PlayerGpuInfo gpuInfo;

	Player(Camera* camera, TileTarget* povTileTarget) : p_camera(camera), povTileTarget(povTileTarget) {

	}

	void update() {
		pos = p_camera->viewPlanePos;

		gpuInfo.color = glm::vec4(1, 1, 1, 1);

		gpuInfo.tileIndices[0] = povTileTarget->node->index;

		//float tileX = 

		//gpuInfo.relativePos[0] = glm::vec2(
		
											);
	}
};