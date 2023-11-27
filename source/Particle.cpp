#include "glsl.h"
#include "Particle.h"
#include "MassSpringSystem.h"


CParticle::CParticle(void)
{
	mForce = mVel = mPos = vec3(0);
	mMass = 1.0;
	facelist = new vector<ivec2>();
	facelist->clear();
}

CParticle::~CParticle(void)
{
}

void CParticle::Init()
{
	mForce = mVel = mPos = vec3(0);
}

vec3 CParticle::Accelation(vec3 &p, vec3 &v)
{
	return vec3(mForce / mMass);
}
vec3 CParticle::Accelation()
{
	return vec3(mForce / mMass);
}

//simulation interface
void CParticle::UpdateState(float h, int method)
{
	mPrevForce = mForce;
	mPrevPos = mPos;
	mPrevVel = mVel;

	vec3 mPosTemp, mVelTemp;
	float halfTime = 0.5 * h;

	vec3 K1_Pos, K2_Pos, K3_Pos, K4_Pos;
	vec3 K1_Vel, K2_Vel, K3_Vel, K4_Vel;

	float sixth = 1 / 6.0;
	float third = 1 / 3.0;

	switch (method)
	{
	case INTEGRATION_METHOD::EXPLICIT_EULER: // Euler Method
		mPos = mPos + (h * mVel);             // X(t+1) = X(t) + h * V(t)
		mVel = mVel + h * (mForce / mMass);   // V(t+1) = V(t) + h * (F(t) / M)
		break;

	case INTEGRATION_METHOD::MID_POINT: // Mid-point Method	
		mPosTemp = mPos + (halfTime * mVel);           // X(t+1/2) = X(t) + h/2 * V(t) 
		mVelTemp = mVel + (halfTime *  Accelation());    // V(t+1/2) = V(t) + h/2 * (F(t) / M)

		mPos = mPos + (h * mVelTemp);                  // X(t+1) = X(t) + h * V(t+1/2) 
		mVel = mVel + (h *  Accelation());             // V(t+1) = V(t) + h * (F(t) / M)
		break;

	case INTEGRATION_METHOD::MODIFIED_EULER: // Modified Euler's method (Heun's method)
		mPosTemp = mPos + (h * mVel);                           // ~X(t+1) = X(t) + h * V(t) 
		mVelTemp = mVel + (h * Accelation());                   // ~V(t+1) = V(t) + h * (F(t) / M)

		mPos = mPos + halfTime * (mVel + mVelTemp);             // X(t+1) = X(t) + h/2 * (V(t) + ~V(t+1)) 	 
		mVel = mVel + halfTime * (Accelation() + Accelation()); // V(t+1) = V(t) + h/2 * ((F(t) / M) + (F(t) / M)) 
		break;

	case INTEGRATION_METHOD::RUNGE_KUTTA_4TH: // 4th Order Runge Kutta method
		K1_Pos = (h * mVel);                 // K1_Pos = h * V(t) 
		K1_Vel = (h * Accelation());         // K1_Vel = h * (F(t) / M)

		mPosTemp = mPos + (halfTime * K1_Pos);
		mVelTemp = mVel + (halfTime * K1_Vel);

		K2_Pos = (h * mVelTemp);             // K2_Pos = X(t) + h/2 * K1_Vel 
		K2_Vel = (h * Accelation());         // K2_Vel = V(t) + h/2 * (F(t) / M)

		mPosTemp = mPos + (halfTime * K2_Pos);
		mVelTemp = mVel + (halfTime * K2_Vel);

		K3_Pos = (h * mVelTemp);             // K3_Pos = X(t) + h/2 * K2_Vel              
		K3_Vel = (h * Accelation());         // K3_Vel = V(t) + h/2 * (F(t) / M)          

		mPosTemp = mPos + K3_Pos;
		mVelTemp = mVel + K3_Vel;

		K4_Pos = (h * mVelTemp);             // K4_Pos = X(t) + h * K3_Vel         
		K4_Vel = (h * Accelation());         // K4_Vel = V(t) + h * (F(t) / M)    

		// X(t+1) = X(t) + 1/6 * (K1_Pos + 2 * K2_Pos + 2 * K3_Pos + K4_Pos)
		// V(t+1) = V(t) + 1/6 * (K1_Vel + 2 * K2_Vel + 2 * K3_Vel + K4_Vel)
		mPos = mPos + (sixth * K1_Pos) + (third * K2_Pos) + (third * K3_Pos) + (sixth * K4_Pos);
		mVel = mVel + (sixth * K1_Vel) + (third * K2_Vel) + (third * K3_Vel) + (sixth * K4_Vel);
		break;

	case INTEGRATION_METHOD::SEMI_EULER: // Semi-Implicit Euler Method	
		mVel = mVel + h * Accelation();   // V(t+1) = V(t) + h * (F(t) / M)
		mPos = mPos + (h * mVel);         // X(t+1) = X(t) + h * V(t+1)	
		break;
	}
}




void CParticle::SetForce(vec3 f)
{
	mForce = f;
}
void CParticle::SetAccel(vec3 a)
{
	mForce = a * mMass;
}
void CParticle::SetVel(vec3 v)
{
	mVel = v;
}
void CParticle::SetPos(vec3 p)
{
	mPos = p;
}

vec3 CParticle::GetForce()
{
	return mForce;
}
vec3 CParticle::GetVel()
{
	return mVel;
}
vec3 CParticle::GetPos()
{
	return mPos;
}

void CParticle::SetMass(float m)
{
	mMass = m;
}
float CParticle::GetMass()
{
	return mMass;
}
void CParticle::GetMass(float &m)
{
	m = mMass;
}

void CParticle::AddForce(vec3 f)
{
	mForce += f;
}