#pragma once
#include "glsl.h"
#include "Particle.h"
class Cnode{
public:
	Cnode(Cnode *parent, int level, vec3 center, float width, float height, float depth);
	~Cnode();
	void createnode();
	void createChildren();
	void drawline(vec3 P1, vec3 P2, float r, float g, float b);
	void drawquad(vec3 P1, vec3 P2, vec3 P3, vec3 P4, float r, float g, float b);
	void render();
	void update(vec3 center);
	void setIndex();
public:
	Cnode *parent;
	Cnode *children[8];
	vec3 vertex[8];		// 0,1,2,3 [상단] 4,5,6,7 [하단]
	vec3 center;
	CParticle *Particle;
	float MaxX, MinX, MaxY, MinY, MaxZ, MinZ;
	float mWidth, mHeight, mDepth;
	float r, g, b;
	int level;
	int ID;
	bool haschild;
	bool render_code;
	bool objInNode;
	vector<int> *facelist;
	vector<int> *facelist2;
	int offset;
};