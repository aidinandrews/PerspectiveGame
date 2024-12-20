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
#include "globalVariables.h"

struct POV {
private:
	int nodeIndex; // Tile pov resides inside.
	float ROTATION_SPEED_3D = 5.0f;

public:
	// Helps orient the scene.  Changed when crossing to different tiles.  In the basis if the current tile.
	// Identity assumed on init.
	MapType mapType;
	LocalDirection localNorth, localSouth, localEast, localWest;

	PositionNodeNetwork* p_nodeNetwork;
	Camera* p_camera;

	TileType oldTileType;
	glm::mat4 currentRotation; // tells how many rotations arount the x, y, and z axes
	glm::mat4 finalRotation;
	// For the 3D view to interpolate between viewing angles when transitioning between tiles:
	//glm::mat4 rotationMatrix3D;
	glm::mat4 lastRotationMatrix3D;
	glm::vec3 lastCamPos, lastCamTarget, lastCamUp;

	glm::vec3 lastCameraPositionOffset;
	float lastRotationMatrixWeight; // How much to interpolate between last and current rotation matrices.
	glm::mat4 rotationMatrix2D; // for the 2D view, needs to 'snap' between rotations on tile transition.

private:
	struct CamInfo {
		glm::vec3 pos, target, up;
		CamInfo(glm::vec3 pos, glm::vec3 target, glm::vec3 up) : pos(pos), target(target), up(up) {}
		CamInfo() {}
	};
	CamInfo currentCamInfo, lastCamInfo;




public:
	POV(PositionNodeNetwork* nodeNetwork, Camera* camera) : p_nodeNetwork(nodeNetwork), p_camera(camera)
	{
		nodeIndex = 0;
		mapType = MAP_TYPE_IDENTITY; // It is assumed initially that pov resides in an XYF tile.

		lastRotationMatrix3D = glm::mat4(1);
		lastCameraPositionOffset = glm::vec3(0, 0, 0);
		lastRotationMatrixWeight = 0.0f;
		rotationMatrix2D = glm::mat4(1);
		currentRotation = glm::mat4(1);
	}

	PositionNode* getNode() { return p_nodeNetwork->getNode(nodeIndex); }
	TileInfo* getTile() { return p_nodeNetwork->getTileInfo(getNode()->getTileInfoIndex()); }

	const LocalDirection getNorth() { return tnav::map(mapType, LOCAL_DIRECTION_3); }
	const LocalDirection getSouth() { return tnav::map(mapType, LOCAL_DIRECTION_1); }
	const LocalDirection getEast() { return tnav::map(mapType, LOCAL_DIRECTION_0); }
	const LocalDirection getWest() { return tnav::map(mapType, LOCAL_DIRECTION_2); }

	void shiftPovEast() { shiftTile(getEast()); }
	void shiftPovWest() { shiftTile(getWest()); }
	void shiftPovNorth() { shiftTile(getNorth()); }
	void shiftPovSouth() { shiftTile(getSouth()); }

	void shiftTileSimple(LocalDirection d)
	{
		using namespace tnav;

		PositionNode* node = getNode();
		LocalDirection D = d;
		MapType m1 = node->getNeighborMap(d);

		// Transition to a side node:
		d = map(node->getNeighborMap(d), d);
		node = p_nodeNetwork->getNode(node->getNeighborIndex(D));

		// Transistion to the neighbor tile (a center node).
		MapType m2 = node->getNeighborMap(d);
		node = p_nodeNetwork->getNode(node->getNeighborIndex(d));

		// Adjust the window space -> tile space mappings:
		mapType = combineMaps(mapType, m1);
		mapType = combineMaps(mapType, m2);

		nodeIndex = node->getIndex();
	}

	// Will adjust the position, basis, and orientation of 'upward' and 'rightward' to 
	// the tile neighbor in the given direction.
	void shiftTile(LocalDirection d)
	{
		using namespace tnav;

		PositionNode* oldNode = getNode();

		// orthos are the directions to the closest neighbor not in the direction of d.
		LocalDirection oldOrtho = (d == getNorth() || d == getSouth())
			? (p_camera->viewPlanePos.x < 0.5f) ? getWest() : getEast()
			: (p_camera->viewPlanePos.y < 0.5f) ? getSouth() : getNorth();
		LocalDirection newOrtho = p_nodeNetwork->mapToSecondNeighbor(oldNode, d, oldOrtho);

		PositionNode* newNode = oldNode;
		LocalDirection D = d;
		MapType m1 = newNode->getNeighborMap(d);

		// Transition to a side node:
		d = map(newNode->getNeighborMap(d), d);
		newNode = p_nodeNetwork->getNode(newNode->getNeighborIndex(D));

		// Transistion to the neighbor tile (a center node).
		MapType m2 = newNode->getNeighborMap(d);
		newNode = p_nodeNetwork->getNode(newNode->getNeighborIndex(d));

		// Adjust the window space -> tile space mappings:
		mapType = combineMaps(mapType, m1);
		mapType = combineMaps(mapType, m2);

		nodeIndex = newNode->getIndex();

		bool sameType = oldNode->getNodeType() == newNode->getNodeType();
		TileType ta = p_nodeNetwork->getTileInfo(oldNode->getTileInfoIndex(), oldOrtho)->type;
		TileType tb = p_nodeNetwork->getTileInfo(newNode->getTileInfoIndex(), newOrtho)->type;
		bool sameConnectedType = ta == tb;
		if (sameType && sameConnectedType) return;

		// used for 3D transformation matrix lerping:
		lastRotationMatrixWeight = 1.0f;
		lastCamInfo = currentCamInfo;
	}

	// once the pov crosses an edge, it may have to update its rotation matrix to face the tile in
	// 3D space.  This function makes sure that currentRotation always does this.
	glm::mat4 rotMatAdjustment3D(TileInfo* tile, LocalDirection dir, float angleAmount)
	{
		TileType oldTileType = tile->type;
		TileType newTileType = p_nodeNetwork->getTileInfo(getTile(), dir)->type;
		if (angleAmount == 0 || oldTileType == newTileType) return glm::mat4(1); // No rotation is change necessary if the types are the same.

		float angle;
		glm::vec3 axisOfRotation;
		if (dir == getNorth()) {
			axisOfRotation = tnav::getCenterToEdgeVec(oldTileType, getEast())
				- tnav::getCenterToEdgeVec(oldTileType, getWest());
		}
		else if (dir == getSouth()) {
			axisOfRotation = tnav::getCenterToEdgeVec(oldTileType, getWest())
				- tnav::getCenterToEdgeVec(oldTileType, getEast());
		}
		else if (dir == getEast()) {
			axisOfRotation = tnav::getCenterToEdgeVec(oldTileType, getSouth())
				- tnav::getCenterToEdgeVec(oldTileType, getNorth());
		}
		else { // dir is getWest()
			axisOfRotation = tnav::getCenterToEdgeVec(oldTileType, getNorth())
				- tnav::getCenterToEdgeVec(oldTileType, getSouth());
		}

		glm::vec3 toEdge = tnav::getCenterToEdgeVec(oldTileType, dir);

		if (tnav::getNormal(newTileType) == -toEdge) {
			angle = -(float)M_PI / 2.0f;
		}
		else if (tnav::getNormal(newTileType) == toEdge) {
			angle = (float)M_PI / 2.0f;
		}
		else {
			angle = (float)M_PI;
		}

		return glm::rotate(glm::mat4(1), angleAmount * angle, axisOfRotation);
	}

	void updatePosition()
	{
		if (p_camera->viewPlanePos.x > 1.0f) {
			lastCameraPositionOffset = p_camera->viewPlanePos - glm::vec3(1, 0, 0);
			p_camera->viewPlanePos.x -= 1.0f;
			shiftTile(getEast());
		}
		else if (p_camera->viewPlanePos.x < 0.0f) {
			lastCameraPositionOffset = p_camera->viewPlanePos + glm::vec3(1, 0, 0);
			p_camera->viewPlanePos.x += 1.0f;
			shiftTile(getWest());
		}

		if (p_camera->viewPlanePos.y > 1.0f) {
			lastCameraPositionOffset = p_camera->viewPlanePos - glm::vec3(0, 1, 0);
			p_camera->viewPlanePos.y -= 1.0f;
			shiftTile(getNorth());
		}
		else if (p_camera->viewPlanePos.y < 0.0f) {
			lastCameraPositionOffset = p_camera->viewPlanePos + glm::vec3(0, 1, 0);
			p_camera->viewPlanePos.y += 1.0f;
			shiftTile(getSouth());
		}
	}

	void update()
	{
		updatePosition();
		update3dTransfMatrix();
	}

	void update3dTransfMatrix()
	{
		currentCamInfo = getCurrentCamInfo();

		if (lastRotationMatrixWeight > 0) {
			lastRotationMatrixWeight -= ROTATION_SPEED_3D * DeltaTime;

			float weight = sin(lastRotationMatrixWeight);
			currentCamInfo = CamInfo(
				glm::mix(currentCamInfo.pos, lastCamInfo.pos, weight),
				glm::mix(currentCamInfo.target, lastCamInfo.target, weight),
				glm::mix(currentCamInfo.up, lastCamInfo.up, weight));

		}

		glm::mat4 vieMatrix = glm::lookAt(currentCamInfo.pos, currentCamInfo.target, currentCamInfo.up);
		finalRotation = getCurrentProjectionMatrix(1.0f) * vieMatrix;
	}

	glm::mat4 getCurrentProjectionMatrix(float aspectRatio)
	{
		constexpr float fov = glm::radians(45.0f); // Field of view in radians 
		float nearPlane = 0.5f, farPlane = 100.0f;
		return glm::perspective(fov, aspectRatio, nearPlane, farPlane);
	}

	CamInfo getCurrentCamInfo()
	{
		glm::vec3 screenTilePos = ((2.0f * p_camera->viewPlanePos) - 1.0f) * 0.5f;
		glm::vec3 pos3D = getNode()->getPosition();
		pos3D += tnav::getCenterToEdgeVec(getTile()->type, getEast()) * screenTilePos.x;
		pos3D += tnav::getCenterToEdgeVec(getTile()->type, getNorth()) * screenTilePos.y;

		glm::mat4 rotAdj(1);
		float adj = (!tnav::isFront(getTile()->type)) ? 1.0f : 1.0f;
		if (p_camera->viewPlanePos.x > 0.5f) {
			float a = (p_camera->viewPlanePos.x - 0.5f) * 2;
			rotAdj = rotMatAdjustment3D(getTile(), getEast(), adj * a / 2.0f);
		}
		else {
			float a = -2.0f * (p_camera->viewPlanePos.x - 0.5f);
			rotAdj = rotMatAdjustment3D(getTile(), getWest(), adj * a / 2.0f);
		}

		if (p_camera->viewPlanePos.y > 0.5f) {
			float a = (p_camera->viewPlanePos.y - 0.5f) * 2;
			rotAdj = rotMatAdjustment3D(getTile(), getNorth(), adj * a / 2.0f) * rotAdj;
		}
		else {
			float a = -2.0f * (p_camera->viewPlanePos.y - 0.5f);
			rotAdj = rotMatAdjustment3D(getTile(), getSouth(), adj * a / 2.0f) * rotAdj;
		}

		glm::vec3 cameraTarget = pos3D;
		float zoom = (float)pow(2, p_camera->zoom);
		glm::vec3 cameraPosition = glm::vec4(zoom * tnav::getNormal(getTile()->type), 1) * rotAdj ;
		cameraPosition += cameraTarget;
		glm::vec3 cameraUp = glm::vec4(tnav::getCenterToEdgeVec(getTile()->type, getNorth()), 1) * rotAdj;
		//glm::mat4 viewMatrix = glm::lookAt(cameraPosition, cameraTarget, cameraUp);

		return CamInfo(cameraPosition, cameraTarget, cameraUp);
	}
};
