#pragma once
#include "MassSpringSystem.h"
#include "Spring.h"

class CObjectFactory
{
public:
	CObjectFactory(void);
	virtual ~CObjectFactory(void);
	
	void TetrahedonLoad(CMassSpringSystem *mss,float ks, float kd, float totalMass, vec3 pos, std::string file);
	void ObjLoad(CMassSpringSystem *mss,float ks, float kd, float totalMass, vec3 pos, std::string file);

private:
	void ObjFromFile(std::vector<CSpring> *s, std::vector<ivec3> *f, std::vector<vec3> *v, std::string file);

	void NodeLoad(std::vector<vec3> *v, std::string file);
	void FaceLoad(std::vector<ivec3> *f, std::string file);
	void ElementLoad(std::vector<CSpring> *s, std::vector<ivec3> *f, std::vector<vec3> *v, std::string file);

	void InitMassSpring(CMassSpringSystem *mss, std::vector<vec3> *v, std::vector<ivec3> *f, std::vector<CSpring> *s);
	void CheckDuplicateSprings(CMassSpringSystem *mms);

	void AddSpring(std::vector<CSpring> *s, CSpring spring);
public:
	float mKD;
	float mKS;
	float mMass;

	vec3 mPos;

	std::vector<vec3> mVertices;
	std::vector<ivec3> mIndices;
	std::vector<CSpring> mSprings;
};

