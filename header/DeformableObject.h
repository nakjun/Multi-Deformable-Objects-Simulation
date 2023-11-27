#pragma once
#include "glsl.h"
#include "Particle.h"
#include "MassSpringSystem.h"
#include "BoundingBox.h"
#include "Octree.h"
class CDeformable
{
public:
	CDeformable(int number,int height);
	~CDeformable();

	//CPU 사용
	void Render();
	void Update();
	void AddForce(vec3 force);	
	void CollisionCheck();
	void UpdateSpringSystem();
	void GetCenter();
	void GetAABB();

	//GPU 사용
	void SetBoundingBox( float X, float Y, float Z);
	void setFaceList();
public:
	int ID;
	
	float mTick;			//time step
	float mMass;				//파티클 한개에 대한 무게임// 수정하려면 ObjectFactory //mMass /= v->size(); 해제

	float mKS;				// ks = 380
	float mKD;				// kd = 5

	int **springinformationTable;

	int TexID[8];
	//CPU 사용
	CBox *Box;

	int maxFaceListSize;

	//GPU 사용
	CTree *Octree;
	vec3 center;	
	CParticle *P;	
	CMassSpringSystem *mSrpingSystem;
	int sum;
	//CPU 사용
	std::vector<CMassSpringSystem*>::iterator curMSS;
	std::vector<CMassSpringSystem*> mMSSList;	//mass spring systsem 리스트
	vector<ivec2> *faceList;
};