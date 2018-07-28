#pragma warning(disable:4996)

#include "MassSpringSystem.h"
#include <time.h>
vec3 spring_force;
vec3 spforce;
FILE *fp2;
int i;
CSpring* spring;
CParticle *p1, *p2, *p;
float ks, kd;
vec3 forceDirection, VelocityDirection;
float len, damp;

bool force_flag = 0;
vec3 force;

clock_t func_start;
clock_t func_end;
double func_result;

CMassSpringSystem::CMassSpringSystem(int method)
{
	mIntMethod = method;
	mNumSprings = mNumSprings = 0;
	mParticleArray = 0;
	mSpringArray = 0;
	mDynamic = 0;
	std::string fileName = "result.txt";
	mFILE = fopen(fileName.c_str(), "w");
	fp2 = fopen("spring_force.txt", "w");
}

CMassSpringSystem::CMassSpringSystem(int nParticle, int nSprings, int method)
{
	mNumSprings = nSprings;
	mNumParticles = nParticle;
	mParticleArray = new CParticle[mNumParticles];	//파티클, 스프링 배열 할당
	mSpringArray = new CSpring[mNumSprings];
	mDynamic = new bool[mNumParticles];
	mIntMethod = method;

	for (int i = 0; i < mNumParticles; ++i)
	{
		mDynamic[i] = true;
	}
	fp2 = fopen("spring_force.txt", "w");
}

CMassSpringSystem::~CMassSpringSystem()
{
	DeleteAllData();
}

void CMassSpringSystem::DeleteAllData(){
	if (mParticleArray)
	{
		delete[] mParticleArray;
		delete[] mSpringArray;
		delete[] mDynamic;

		mParticleArray = NULL;
		mSpringArray = NULL;
		mDynamic = NULL;

		mFaceArray.clear();
		fclose(mFILE);
	}
}
void CMassSpringSystem::Create(int nParticles, int nSprings)
{
	mNumSprings = nSprings;
	mNumParticles = nParticles;

	DeleteAllData();

	mParticleArray = new CParticle[mNumParticles];		//파티클, 스프링 배열 할당
	mSpringArray = new CSpring[mNumSprings];
	mDynamic = new bool[mNumParticles];
	for (int i = 0; i < mNumParticles; ++i)
	{
		mDynamic[i] = true;
		mParticleArray[i].Init();
	}
}

CParticle* CMassSpringSystem::GetParticle(int idx)
{
	return &(mParticleArray[idx]);
}
CSpring* CMassSpringSystem::GetSpring(int idx)
{
	return &(mSpringArray[idx]);
}

float DotProduct(glm::vec3 *v1, glm::vec3 *v2)
{

	return (v1->x * v2->x) + (v1->y * v2->y) + (v1->z * v2->z);
}

//스프링 힘 계산
void CMassSpringSystem::ComputeSpringForce(int springIdx)
{
	spring = GetSpring(springIdx);
	p1 = GetParticle(spring->mIndex1);		//get one spring's particles
	p2 = GetParticle(spring->mIndex2);

	ks = spring->mKs * p1->mMass;
	kd = spring->mKd;

	forceDirection = p1->GetPos() - p2->GetPos();	//calc direction // 낙지추가
	VelocityDirection = p1->GetVel() - p2->GetVel();
	len = glm::length(forceDirection);

	spforce = (len - spring->mRestLen) * ks * forceDirection;
	damp = (DotProduct(&VelocityDirection, &forceDirection) * kd) / len;

	spring_force = forceDirection / len;
	spring_force += -(spforce + damp);

	p1->AddForce(spring_force);
	p2->AddForce(-spring_force);
}

//시뮬레이션 수행
void CMassSpringSystem::ComputeNextStep(float dt, int ID)
{
	SetForceZero();
	if (ID == 1)
		ComputeForce();
	else
		ComputeForce2();

	func_start = clock();
	// Compute spring force
	for (i = 0; i < mNumSprings; ++i)
		ComputeSpringForce(i);

	for (i = 0; i < mNumParticles; ++i)
	{
		p = GetParticle(i);
		p->UpdateState(dt, mIntMethod);
		//printf("%f %f %f\n", p->mForce.x, p->mForce.y, p->mForce.z);
	}

}

void CMassSpringSystem::SetForceZero()
{
	for (i = 0; i < mNumParticles; ++i)
	{
		p = GetParticle(i);
		p->SetForce(vec3(0));
	}
}

void CMassSpringSystem::ComputeForce()
{
	for (i = 0; i < mNumParticles; ++i)
	{
		p = GetParticle(i);

		force = vec3(20.0 * p->mMass, 0.0 *p->mMass, 0);
		p->SetForce(force); // Gravity apply
	}
}
void CMassSpringSystem::ComputeForce2()
{
	for (i = 0; i < mNumParticles; ++i)
	{
		p = GetParticle(i);
		force = vec3(-20.0 * p->mMass, 0.0 *p->mMass, 0);
		p->SetForce(force); // Gravity apply
	}
}