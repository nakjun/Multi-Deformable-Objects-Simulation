#pragma once
#include "glsl.h"
#include "Ocpart.h"
#include <time.h>

time_t start, f_end;
FILE *fp3;

Cnode::Cnode(Cnode *par, int lv, vec3 Center, float width, float height, float depth)
{
	parent = par;
	level = lv;
	center = Center;
	mWidth = width/2;
	mHeight = height/2;
	mDepth = depth/2;

	render_code = false; // false => line & true => quad

	MinX = center.x - mWidth;
	MaxX = center.x + mWidth;
	MinY = center.y - mHeight;
	MaxY = center.y + mHeight;
	MinZ = center.z - mDepth;
	MaxZ = center.z + mDepth;		
		
	haschild = false;
	createnode();

	facelist = new vector<int>();
	facelist->clear();

	facelist2 = new vector<int>();
	facelist2->clear();

	objInNode = false;
	r = g = b = 1;
	if (level < 3) createChildren();
}
Cnode::~Cnode()
{

}
void Cnode::createChildren()
{
	float nWidth = mWidth; float nHeight = mHeight; float nDepth = mDepth;
	children[0] = new Cnode(this, level + 1, vec3(center.x - nWidth, center.y + nHeight, center.z - nDepth), nWidth, nHeight, nDepth);
	children[1] = new Cnode(this, level + 1, vec3(center.x + nWidth, center.y + nHeight, center.z - nDepth), nWidth, nHeight, nDepth);
	children[2] = new Cnode(this, level + 1, vec3(center.x + nWidth, center.y + nHeight, center.z + nDepth), nWidth, nHeight, nDepth);
	children[3] = new Cnode(this, level + 1, vec3(center.x - nWidth, center.y + nHeight, center.z + nDepth), nWidth, nHeight, nDepth);

	children[4] = new Cnode(this, level + 1, vec3(center.x - nWidth, center.y - nHeight, center.z - nDepth), nWidth, nHeight, nDepth);
	children[5] = new Cnode(this, level + 1, vec3(center.x + nWidth, center.y - nHeight, center.z - nDepth), nWidth, nHeight, nDepth);
	children[6] = new Cnode(this, level + 1, vec3(center.x + nWidth, center.y - nHeight, center.z + nDepth), nWidth, nHeight, nDepth);
	children[7] = new Cnode(this, level + 1, vec3(center.x - nWidth, center.y - nHeight, center.z + nDepth), nWidth, nHeight, nDepth);

	haschild = true;
}
void Cnode::createnode()
{
	float X = mWidth;
	float Y = mHeight;
	float Z = mDepth;

	// node 惑窜 vertex 积己
	vertex[0] = vec3((center.x - mWidth), (center.y + mHeight), (center.z - mDepth));
	vertex[1] = vec3((center.x + mWidth), (center.y + mHeight), (center.z - mDepth));
	vertex[2] = vec3((center.x + mWidth), (center.y + mHeight), (center.z + mDepth));
	vertex[3] = vec3((center.x - mWidth), (center.y + mHeight), (center.z + mDepth));

	// node 窍窜 vertex 积己
	vertex[4] = vec3((center.x - mWidth), (center.y - mHeight), (center.z - mDepth));
	vertex[5] = vec3((center.x + mWidth), (center.y - mHeight), (center.z - mDepth));
	vertex[6] = vec3((center.x + mWidth), (center.y - mHeight), (center.z + mDepth));
	vertex[7] = vec3((center.x - mWidth), (center.y - mHeight), (center.z + mDepth));
}

