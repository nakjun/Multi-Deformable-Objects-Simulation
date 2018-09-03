#pragma once
#include "glsl.h"

class CParticle
{
public:
	CParticle(void);
	~CParticle(void);

	void SetForce(vec3 f);
	void SetAccel(vec3 a);
	void SetVel(vec3 v);
	void SetPos(vec3 p);

	vec3 GetForce();
	vec3 GetVel();
	vec3 GetPos();

	void SetMass(float m);
	float GetMass();
	void GetMass(float &m);

	void AddForce(vec3 f);
	vec3 Accelation(vec3 &p, vec3 &v);
	vec3 Accelation();

	//simulation interface
	void Init();
	void UpdateState(float h, int method);
public:
	float mMass;
	vec3 mPos;
	vec3 mVel;
	vec3 mForce;

	vec3 mPrevPos;
	vec3 mPrevVel;
	vec3 mPrevForce;

	vector<ivec2> *facelist;
};

