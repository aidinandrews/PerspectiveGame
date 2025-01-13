#pragma once
#include<iostream>
#include<vector>
#include<array>

#include"tileNavigation.h"

struct ForceManager
{
	// * both vectors here miror the node vector in TileNodeNetwork
	std::vector<bool> forceList; // Sets of 4 bools represent force components in local bases of TileNodes.
	std::vector<int> freeForceListIndices;
	
	// Given an index to a component in the force list, will return that component's node index.
	int getNodeIndex(int forceListComponentIndex)
	{
		int index = forceListComponentIndex;
		index -= index % 4; // go to the initial index of the component list
		index /= 4; // there are 4x more bools in the forceList than nodes in the node list.
		return index;
	}

	LocalDirection getForce(int index)
	{
		uint_fast8_t forceFlags = 0;
		forceFlags |= (forceList[index + 0] << 0);
		forceFlags |= (forceList[index + 1] << 1);
		forceFlags |= (forceList[index + 2] << 2);
		forceFlags |= (forceList[index + 3] << 3);

		switch (forceFlags) {
		case 0b0000: return LOCAL_DIRECTION_STATIC;
		case 0b0001: return LOCAL_DIRECTION_0;
		case 0b0010: return LOCAL_DIRECTION_1;
		case 0b0100: return LOCAL_DIRECTION_2;
		case 0b1000: return LOCAL_DIRECTION_3;
		case 0b0011: return LOCAL_DIRECTION_0_1;
		case 0b0110: return LOCAL_DIRECTION_1_2;
		case 0b1100: return LOCAL_DIRECTION_2_3;
		case 0b1001: return LOCAL_DIRECTION_3_0;
		default: return LOCAL_DIRECTION_ERROR;
		}
	}

	void setForce(int index, LocalDirection forceDir)
	{
		static const std::array<std::array<int, 4>, 9> forces = {{ 
			{{ 1, 0, 0, 0 }}, // LOCAL_DIRECTION_0 
			{{ 0, 1, 0, 0 }}, // LOCAL_DIRECTION_1 
			{{ 0, 0, 1, 0 }}, // LOCAL_DIRECTION_2 
			{{ 0, 0, 0, 1 }}, // LOCAL_DIRECTION_3
			{{ 1, 1, 0, 0 }}, // LOCAL_DIRECTION_0_1 
			{{ 0, 1, 1, 0 }}, // LOCAL_DIRECTION_1_2
			{{ 0, 0, 1, 1 }}, // LOCAL_DIRECTION_2_3 
			{{ 1, 0, 0, 1 }}, // LOCAL_DIRECTION_3_0 
			{{ 0, 0, 0, 0 }}, // LOCAL_DIRECTION_STATIC 
		}};
		std::copy(forces[forceDir].begin(), forces[forceDir].end(), forceList.begin() + index);
	}

	void alterForce(int forceIndex, MapType map)
	{
		LocalDirection d = getForce(forceIndex);
		if (d == LOCAL_DIRECTION_STATIC) return;
		setForce(forceIndex, tnav::map(map, d));
	}

	int addForce(LocalDirection d, int nodeIndex)
	{
		const static std::array<std::array<int, 4>, 9> forces = {{
			{{1, 0, 0, 0}}, // LOCAL_DIRECTION_0 
			{{0, 1, 0, 0}}, // LOCAL_DIRECTION_1 
			{{0, 0, 1, 0}}, // LOCAL_DIRECTION_2 
			{{0, 0, 0, 1}}, // LOCAL_DIRECTION_3 
			{{1, 1, 0, 0}}, // LOCAL_DIRECTION_0_1 
			{{0, 1, 1, 0}}, // LOCAL_DIRECTION_1_2 
			{{0, 0, 1, 1}}, // LOCAL_DIRECTION_2_3 
			{{1, 0, 0, 1}}, // LOCAL_DIRECTION_3_0 
			{{0, 0, 0, 0}}  // LOCAL_DIRECTION_STATIC 
		}}; 
			
		if (freeForceListIndices.size() > 0) {
			int i = freeForceListIndices.back();
			freeForceListIndices.pop_back();
			forceList.insert(forceList.begin() + i, forces[d].begin(), forces[d].end());
			return i;
		}
		else {
			forceList.insert(forceList.end(), forces[d].begin(), forces[d].end());
			return (int)forceList.size() - 4;
		}
	}

	void removeForce(int forceIndex)
	{
		if (forceIndex == forceList.size() - 4) {
			forceList.pop_back();
			forceList.pop_back();
			forceList.pop_back();
			forceList.pop_back();
		}
		else {
			freeForceListIndices.push_back(forceIndex);
			setForce(forceIndex, LOCAL_DIRECTION_STATIC);
		}
	}
};