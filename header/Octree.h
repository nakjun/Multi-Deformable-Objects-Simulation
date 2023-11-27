#pragma once
#include "glsl.h"
#include "Ocpart.h"

class CTree
{
public:
	CTree::CTree(vec3 Center, float width, float height, float depth)
	{
		root = new Cnode(root, 1, Center, width, height, depth);
		root->level = 1;
		//root->DefObj.clear();
	}
	~CTree();	
public:
	int level;
	Cnode *root;
};

