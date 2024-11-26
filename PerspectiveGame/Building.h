//#ifndef BUILDING_DEFINED
//#define BUILDING_DEFINED

#pragma once
#include <iostream>
#include <vector>
#include <algorithm>

#include"dependancyHeaders.h"

#include "globalVariables.h"

struct ForceSink {
	int tileIndex;

	ForceSink(int tileIndex) :
		tileIndex(tileIndex) 
	{}

	void update();
};