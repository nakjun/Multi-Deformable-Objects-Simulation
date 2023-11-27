#include "pairwise.h"

CFacePair::CFacePair(int index1,int index2, int result)
{
	faceIndex1 = index1;
	faceIndex2 = index2;
	collisionResult = result;
}