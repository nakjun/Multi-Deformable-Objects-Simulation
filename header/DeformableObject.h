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

	//CPU ���
	void Render();
	void Update();
	void AddForce(vec3 force);	
	void CollisionCheck();
	void UpdateSpringSystem();
	void GetCenter();
	void GetAABB();

	//GPU ���
	void SetBoundingBox( float X, float Y, float Z);
	void setFaceList();
public:
	int ID;
	
	float mTick;			//time step
	float mMass;				//��ƼŬ �Ѱ��� ���� ������// �����Ϸ��� ObjectFactory //mMass /= v->size(); ����

	float mKS;				// ks = 380
	float mKD;				// kd = 5

	int **springinformationTable;

	int TexID[8];
	//CPU ���
	CBox *Box;

	int maxFaceListSize;

	//GPU ���
	CTree *Octree;
	vec3 center;	
	CParticle *P;	
	CMassSpringSystem *mSrpingSystem;
	int sum;
	//CPU ���
	std::vector<CMassSpringSystem*>::iterator curMSS;
	std::vector<CMassSpringSystem*> mMSSList;	//mass spring systsem ����Ʈ
	vector<ivec2> *faceList;
};