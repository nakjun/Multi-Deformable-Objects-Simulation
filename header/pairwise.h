#pragma once
#include "glsl.h"

class CFacePair
{
public:
	CFacePair(int index1,int index2, int result);
	~CFacePair();
public:
	int faceIndex1;
	int faceIndex2;
	int collisionResult;
};

