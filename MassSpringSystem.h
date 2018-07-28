#pragma once
#include "glsl.h"
#include "Particle.h"
#include "Spring.h"
// 1: Explicit Euler Method
// 2: Mid-point Method
// 3: Modified Euler's method (Heun's method)
// 4: 4th Order Runge Kutta method
// 5: Semi-Implicit Euler Method
namespace INTEGRATION_METHOD
{
	enum Enum
	{
		EXPLICIT_EULER,
		MID_POINT,
		MODIFIED_EULER,
		RUNGE_KUTTA_4TH,
		SEMI_EULER
	};
}

//spring system class
class CMassSpringSystem
{
public:
	CMassSpringSystem(int method);
	CMassSpringSystem(int nParticle, int nSprings, int method);
	~CMassSpringSystem();

	void Create(int nParticles, int nSprings);

	CParticle* GetParticle(int idx);
	CSpring* GetSpring(int idx);
	vec3 addforce;
	//calculate force and state.
	void SetForceZero();
	void ComputeForce();
	void ComputeForce2();
	void ComputeSpringForce(int springIdx);
	void ComputeNextStep(float dt, int ID);

	int GetNumParticles(){ return mNumParticles; }
	int GetNumSpring()	 { return mNumSprings; }
	void SetStatic(int i){ mDynamic[i] = false; }

	void DeleteAllData();
public:
	int mIntMethod; // Integration Method
	//number of particles and springs
	int mNumParticles;
	int mNumSprings;
	FILE *mFILE;
	CParticle *mParticleArray;
	CSpring *mSpringArray;
	bool *mDynamic;

	vector<ivec3> mFaceArray;
};

