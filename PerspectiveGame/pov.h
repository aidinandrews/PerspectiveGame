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
private:
	PositionNode* node; // Tile pov resides inside.

public:
	// Helps orient the scene.  Changed when crossing to different tiles.  In the basis if the current tile.
	// Identity assumed on init.
	int mapIndex; 
	LocalDirection localNorth, localSouth, localEast, localWest;

	PositionNodeNetwork* p_nodeNetwork;
	Camera* p_camera;

	// For the 3D view to interpolate between viewing angles when transitioning between tiles:
	glm::mat4 rotationMatrix3D, lastRotationMatrix3D;
	glm::vec3 lastCameraPositionOffset;
	float lastRotationMatrixWeight; // How much to interpolate between last and current rotation matrices.
	glm::mat4 rotationMatrix2D; // for the 2D view, needs to 'snap' between rotations on tile transition.

private:
	// Will adjust the position, basis, and orientation of 'upward' and 'rightward' to 
	// the tile neighbor in the given direction.
	void shiftTile(LocalDirection toNeighbor)
	{
		LocalDirection D = toNeighbor;
		int m1 = node->getNeighborMapIndex(toNeighbor);
		// Transition to a side node:
		toNeighbor = tnav::getMappedAlignment(node->getNeighborMapIndex(toNeighbor), toNeighbor);
		node = p_nodeNetwork->getNode(node->getNeighborIndex(D));
		// Transistion to the neighbor tile (a center node).
		int m2 = node->getNeighborMapIndex(toNeighbor);
		node = p_nodeNetwork->getNode(node->getNeighborIndex(toNeighbor));

		// Adjust the window space -> tile space mappings:
		mapIndex = tnav::combineAlignmentMappings(tnav::combineAlignmentMappings(mapIndex, m1), m2);
	}

public:
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

	PositionNode* getNode() { return node; }
	PositionNode* getTile() { return node; }

	LocalDirection getNorth() { return tnav::getMappedAlignment(mapIndex, LOCAL_DIRECTION_3); }
	LocalDirection getSouth() { return tnav::getMappedAlignment(mapIndex, LOCAL_DIRECTION_1); }
	LocalDirection getEast() { return tnav::getMappedAlignment(mapIndex, LOCAL_DIRECTION_0); }
	LocalDirection getWest() { return tnav::getMappedAlignment(mapIndex, LOCAL_DIRECTION_2); }

	void shiftPovEast()  { shiftTile(getEast());  }
	void shiftPovWest()  { shiftTile(getWest());  }
	void shiftPovNorth() { shiftTile(getNorth()); }
	void shiftPovSouth() { shiftTile(getSouth()); }

	void updatePosition()
	{
		if (p_camera->viewPlanePos.x > 1.0f) {
			lastRotationMatrixWeight = 1.0f;
			lastCameraPositionOffset = p_camera->viewPlanePos - glm::vec3(1, 0, 0);
			p_camera->viewPlanePos.x -= 1.0f;
			shiftPovEast();
		}
		else if (p_camera->viewPlanePos.x < 0.0f) {
			lastRotationMatrixWeight = 1.0f;
			lastCameraPositionOffset = p_camera->viewPlanePos + glm::vec3(1, 0, 0);
			p_camera->viewPlanePos.x += 1.0f;
			shiftPovWest();
		}

		if (p_camera->viewPlanePos.y > 1.0f) {
			lastRotationMatrix3D = rotationMatrix3D;
			lastRotationMatrixWeight = 1.0f;			
			lastCameraPositionOffset = p_camera->viewPlanePos - glm::vec3(0, 1, 0);
			p_camera->viewPlanePos.y -= 1.0f;
			shiftPovNorth();
		}
		else if (p_camera->viewPlanePos.y < 0.0f) {
			lastRotationMatrix3D = rotationMatrix3D;
			lastRotationMatrixWeight = 1.0f;			
			lastCameraPositionOffset = p_camera->viewPlanePos + glm::vec3(0, 1, 0);
			p_camera->viewPlanePos.y += 1.0f;
			shiftPovSouth();
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