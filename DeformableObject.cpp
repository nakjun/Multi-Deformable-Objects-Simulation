#pragma warning(disable:4996)
#pragma once
#include "glsl.h"
#include "DeformableObject.h"
#include "ObjectFactory.h"
#include "Ocpart.h"
#include <stdlib.h>
#include <time.h>


int temp;
float collision_x;
ivec3 face;
float x = 0;
float y = 0;
float z = 0;
clock_t f_start;
clock_t f_end;
CSpring *sp;
#define sphere "../models/sphere/sphere"
#define alphaA "../models/alpha/A"
#define alphaX "../models/alpha/X"
#define alphaL "../models/alpha/L5"
#define box "../models/box/box"
#define bunny "../models/bunny/bunny_741"
#define torus "../models/torus/Torus"

float tmp;
int currIndex = 0;

CDeformable::CDeformable(int number)
{	
	ID = number;	

	if (ID == 1){
		mTick = 0.001;		
		CObjectFactory *of = new CObjectFactory();

		float X_POS = rand() % 30 - rand() % 30;
		float Y_POS = rand() % 20 + rand() % 20;

		mSrpingSystem = new CMassSpringSystem(INTEGRATION_METHOD::SEMI_EULER);
		//mss	   ks  kd    mass      position	  model_name
		of->TetrahedonLoad(mSrpingSystem, 0.0, 0.0, 1.0, vec3(X_POS, Y_POS, 0.0), torus);

		mMSSList.push_back(mSrpingSystem);

		curMSS = mMSSList.begin();

		delete of;
	}
	else{
		mTick = 0.0005;		
		CObjectFactory *of = new CObjectFactory();

		mSrpingSystem = new CMassSpringSystem(INTEGRATION_METHOD::SEMI_EULER);
		//mss	   ks  kd    mass      position	  model_name
		of->TetrahedonLoad(mSrpingSystem, 0.0, 0.0, 1.0, vec3(-2.7, 15.0, 0), bunny);

		mMSSList.push_back(mSrpingSystem);

		curMSS = mMSSList.begin();

		delete of;
	}
	
	(*curMSS)->mFaceArray2.clear();

	ivec3 facearr;
	ivec3 facearr2;
	for (int i = 0; i < (*curMSS)->mFaceArray.size(); i++)
	{
		facearr = (*curMSS)->mFaceArray.at(i);
		(*curMSS)->mParticleArray[facearr.x].facelist->push_back(ivec2(i, 0));
		facearr2.x = (*curMSS)->mParticleArray[facearr.x].facelist->size();
		(*curMSS)->mParticleArray[facearr.y].facelist->push_back(ivec2(i, 0));
		facearr2.y = (*curMSS)->mParticleArray[facearr.y].facelist->size();
		(*curMSS)->mParticleArray[facearr.z].facelist->push_back(ivec2(i, 0));
		facearr2.z = (*curMSS)->mParticleArray[facearr.z].facelist->size();

		(*curMSS)->mFaceArray2.push_back(facearr2);
		
		if (maxFaceListSize < (*curMSS)->mParticleArray[facearr.x].facelist->size()) maxFaceListSize = (*curMSS)->mParticleArray[facearr.x].facelist->size();
		if (maxFaceListSize < (*curMSS)->mParticleArray[facearr.y].facelist->size()) maxFaceListSize = (*curMSS)->mParticleArray[facearr.y].facelist->size();
		if (maxFaceListSize < (*curMSS)->mParticleArray[facearr.z].facelist->size()) maxFaceListSize = (*curMSS)->mParticleArray[facearr.z].facelist->size();

	}

	printf("max : %d", maxFaceListSize);

	Box = new CBox();
	springinformationTable = (int **)malloc((*curMSS)->GetNumParticles()*sizeof(int));	

	int *currIndex = (int *)malloc(sizeof(int)* (*curMSS)->GetNumParticles());
	for (int i = 0; i < (*curMSS)->GetNumParticles(); i++)
	{
		currIndex[i] = 0;
	}
	
	for (int i = 0; i < (*curMSS)->GetNumSpring(); i++){
		CSpring *temp = (*curMSS)->GetSpring(i);			
			
		temp->forceindex1 = temp->mIndex1 * 30 + currIndex[temp->mIndex1];
		currIndex[temp->mIndex1]++;
			
		temp->forceindex2 = temp->mIndex2 * 30 + currIndex[temp->mIndex2];
		currIndex[temp->mIndex2]++;			
	}	
	
}
CDeformable::~CDeformable()
{

}

void CDeformable::setFaceList()
{
	faceList = new vector<ivec2>();
	faceList->clear();
	int FaceCount = (*curMSS)->mFaceArray.size();
	Cnode *root = Octree->root;
	
	FILE *fp = fopen("FaceSetAttach.txt","w");
	
	sum = 0;
	int flag = 0;
	ivec3 FaceTemp;
	CParticle *NodePos[3];
	for (int a = 0; a < 8; a++)
	{
		Cnode *L2 = root->children[a];
		for (int b = 0; b < 8; b++)
		{
			Cnode *L3 = L2->children[b];
			for (int i = 0; i < FaceCount; i++)
			{
				FaceTemp = (*curMSS)->mFaceArray.at(i);
				
				NodePos[0] = (*curMSS)->GetParticle(FaceTemp.x);
				NodePos[1] = (*curMSS)->GetParticle(FaceTemp.y);
				NodePos[2] = (*curMSS)->GetParticle(FaceTemp.z);
				
				for (int j = 0; j < 3; j++)
				{
					flag = 0;
					if (L3->MinX<=NodePos[j]->mPos.x && L3->MaxX>=NodePos[j]->mPos.x) flag++;
					if (L3->MinY<=NodePos[j]->mPos.y && L3->MaxY>=NodePos[j]->mPos.y) flag++;
					if (L3->MinZ<=NodePos[j]->mPos.z && L3->MaxZ>=NodePos[j]->mPos.z) flag++;
					if (flag == 3)
					{						
						L3->facelist2->push_back(i);
						faceList->push_back(ivec2(i, a * 8 + b));
						break;
					}
				}
			}
			fprintf(fp,"[%d]¹øÂ° BB [°¹¼ö : %d]\n",a*8+b,L3->facelist2->size());			
			for (int x = 0; x < L3->facelist2->size(); x++)
			{
				fprintf(fp, "%d ", L3->facelist2->at(x));
			}
			fprintf(fp, "\n");
			L3->offset = sum;
			sum += L3->facelist2->size();			
		}
	}
	
	fclose(fp);
	printf("summary Face Count : %d\n", sum);

	FILE *fpp = fopen("DeformableFaceList.txt", "w");
	for (int i = 0; i < faceList->size(); i++)
	{
		fprintf(fpp, "[%d] %d %d\n", i, faceList->at(i).x, faceList->at(i).y);
	}
	fclose(fpp);
}

void CDeformable::SetBoundingBox(int offset){
	float min, max;
	min = (*curMSS)->GetParticle(0)->mPos.x;
	max = (*curMSS)->GetParticle(0)->mPos.x;
	for (int i = 0; i < (*curMSS)->mNumParticles; i++){
		if (min >(*curMSS)->GetParticle(i)->mPos.x) min = (*curMSS)->GetParticle(i)->mPos.x;
		if (max < (*curMSS)->GetParticle(i)->mPos.x) max = (*curMSS)->GetParticle(i)->mPos.x;
	}
	Box->minX = min; Box->maxX = max;
	

	min = (*curMSS)->GetParticle(0)->mPos.y;
	max = (*curMSS)->GetParticle(0)->mPos.y;
	for (int i = 0; i < (*curMSS)->mNumParticles; i++){
		if (min >(*curMSS)->GetParticle(i)->mPos.y) min = (*curMSS)->GetParticle(i)->mPos.y;
		if (max < (*curMSS)->GetParticle(i)->mPos.y) max = (*curMSS)->GetParticle(i)->mPos.y;
	}
	Box->minY = min; Box->maxY = max;
	

	min = (*curMSS)->GetParticle(0)->mPos.z;
	max = (*curMSS)->GetParticle(0)->mPos.z;
	for (int i = 0; i < (*curMSS)->mNumParticles; i++){
		if (min >(*curMSS)->GetParticle(i)->mPos.z) min = (*curMSS)->GetParticle(i)->mPos.z;
		if (max < (*curMSS)->GetParticle(i)->mPos.z) max = (*curMSS)->GetParticle(i)->mPos.z;
	}
	Box->minZ = min; Box->maxZ = max;
	

	center = vec3((Box->maxX + Box->minX) / 2, (Box->maxY + Box->minY) / 2, (Box->maxZ + Box->minZ) / 2);
	
	Octree = new CTree(center, (Box->maxX - Box->minX) / 2, (Box->maxY - Box->minY) / 2, (Box->maxZ - Box->minZ) / 2);
}
