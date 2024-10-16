#pragma once

#include <iostream>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "cameraManager.h"
#include "tileNavigation.h"
#include "positionNodeNetwork.h"
#include "vectorHelperFunctions.h"

struct POV {
	// Helps orient the scene.  Changed when crossing to different tiles.  In the basis if the current tile.
	// Identity assumed on init.
	int mapIndex; 

	PositionNodeNetwork* p_nodeNetwork;
	PositionNode* node; // Tile pov resides inside.
	Camera* p_camera;

	// For the 3D view to interpolate between viewing angles when transitioning between tiles:
	glm::mat4 rotationMatrix3D, lastRotationMatrix3D;
	glm::vec3 lastCameraPositionOffset;
	float lastRotationMatrixWeight; // How much to interpolate between last and current rotation matrices.
	glm::mat4 rotationMatrix2D; // for the 2D view, needs to 'snap' between rotations on tile transition.

	POV(PositionNodeNetwork* nodeNetwork, Camera* camera) : p_nodeNetwork(nodeNetwork), p_camera(camera)
	{
		node = p_nodeNetwork->getNode(p_nodeNetwork->getTileInfo(0)->nodeIndex);
		mapIndex = ALIGNMENT_MAP_IDENTITY; // It is assumed initially that pov resides in an XYF tile.

		rotationMatrix3D = glm::mat4(1);
		lastRotationMatrix3D = glm::mat4(1);
		lastCameraPositionOffset = glm::vec3(0, 0, 0);
		lastRotationMatrixWeight = 0.0f;
		rotationMatrix2D = glm::mat4(1);
	}

	// Will adjust the position, basis, and orientation of 'upward' and 'rightward' to 
	// the tile neighbor in the given direction.
	void adjustPositionInfo(LocalDirection toNeighbor)
	{
		// Transition to a side node:
		LocalDirection oldToNeighbor = toNeighbor;
		toNeighbor = node->mapToNeighbor(toNeighbor, oldToNeighbor);
		mapIndex = tnav::combineAlignmentMappings(mapIndex, node->getNeighborMapIndex(oldToNeighbor));
		node = p_nodeNetwork->getNeighbor(node, oldToNeighbor);

		// transition to a center node == neighbor tile.
		oldToNeighbor = toNeighbor;
		toNeighbor = node->mapToNeighbor(toNeighbor, oldToNeighbor);
		mapIndex = tnav::combineAlignmentMappings(mapIndex, node->getNeighborMapIndex(oldToNeighbor));
		node = p_nodeNetwork->getNeighbor(node, oldToNeighbor);
	}

	void updatePosition()
	{
		if (p_camera->viewPlanePos.x > 1.0f) {
			lastRotationMatrixWeight = 1.0f;
			lastCameraPositionOffset = p_camera->viewPlanePos - glm::vec3(1, 0, 0);
			p_camera->viewPlanePos.x -= 1.0f;
			adjustPositionInfo(tnav::getMappedAlignment(mapIndex, LOCAL_DIRECTION_0));
		}
		else if (p_camera->viewPlanePos.x < 0.0f) {
			lastRotationMatrixWeight = 1.0f;
			lastCameraPositionOffset = p_camera->viewPlanePos + glm::vec3(1, 0, 0);
			p_camera->viewPlanePos.x += 1.0f;
			adjustPositionInfo(tnav::getMappedAlignment(mapIndex, LOCAL_DIRECTION_2));
		}

		if (p_camera->viewPlanePos.y > 1.0f) {
			lastRotationMatrix3D = rotationMatrix3D;
			lastRotationMatrixWeight = 1.0f;			
			lastCameraPositionOffset = p_camera->viewPlanePos - glm::vec3(0, 1, 0);
			p_camera->viewPlanePos.y -= 1.0f;
			adjustPositionInfo(tnav::getMappedAlignment(mapIndex, LOCAL_DIRECTION_3));
		}
		else if (p_camera->viewPlanePos.y < 0.0f) {
			lastRotationMatrix3D = rotationMatrix3D;
			lastRotationMatrixWeight = 1.0f;			
			lastCameraPositionOffset = p_camera->viewPlanePos + glm::vec3(0, 1, 0);
			p_camera->viewPlanePos.y += 1.0f;
			adjustPositionInfo(tnav::getMappedAlignment(mapIndex, LOCAL_DIRECTION_1));
		}

		// After moving the camera around, we must make sure the new position 
		// is properly recorded in all the tranforms needed for drawing!
		p_camera->getProjectionMatrix();
	}

	void update()
	{
		updatePosition();
	}
};