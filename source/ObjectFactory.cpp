#pragma warning(disable:4996)

#include "glsl.h"
#include "ObjectFactory.h"
#include "Spring.h"


#define sphereEle "../models/sphere/sphere.ele"
#define alphaAEle "../models/alpha/A.ele"
#define alphaXEle "../models/alpha/X.ele"
#define alphaLEle "../models/alpha/L5.ele"
#define boxEle "../models/box/box.ele"
#define bunnyEle "../models/bunny/bunny_741.ele"
#define TorusEle "../models/torus/Torus.ele"

#define sphereFace "../models/sphere/sphere.face"
#define alphaFace "../models/alpha/A.face"
#define alphaxFace "../models/alpha/X.face"
#define alphaLFace "../models/alpha/L5.face"
#define boxFace "../models/box/box.face"
#define bunnyFace "../models/bunny/bunny_741.face"
#define TorusFace "../models/torus/Torus.face"

int ***Face;
int **Matrix;
int MaxVertex;
FILE *fp_spring;


CObjectFactory::CObjectFactory(void)
{
}
CObjectFactory::~CObjectFactory(void)
{
}

void CObjectFactory::InitMassSpring(
	CMassSpringSystem *mss,
	std::vector<vec3> *v,
	std::vector<ivec3> *f,
	std::vector<CSpring> *s)
{
	mss->Create(v->size(), s->size());

	//copy vertices to mss particles's pos
	for (int i = 0; i < mss->GetNumParticles(); ++i){
		CParticle *p = mss->GetParticle(i);
		p->mPos = (*v)[i];
		p->mPos += mPos;
		p->mMass = mMass;
	}

	mss->mFaceArray = (*f);

	//copy springs to mss springs
	for (int i = 0; i < mss->GetNumSpring(); ++i){
		CSpring *sp = mss->GetSpring(i);
		*sp = (*s)[i];
	}
}

//obj loader
void CObjectFactory::ObjLoad(CMassSpringSystem *mss, float ks, float kd, float totalMass, vec3 pos, std::string file)
{
	mKS = ks;
	mKD = kd;
	mMass = totalMass;

	mPos = pos;

	mVertices.clear();
	mIndices.clear();
	mSprings.clear();

	std::string nodeFile(file);		nodeFile.append(".obj");

	ObjFromFile(&mSprings, &mIndices, &mVertices, nodeFile);	//obj 파일에서 로드

	InitMassSpring(mss, &mVertices, &mIndices, &mSprings);	//스프링 중복 검사 및 데이터 저장
}


void CObjectFactory::ObjFromFile(std::vector<CSpring> *s, std::vector<ivec3> *f, std::vector<vec3> *v, std::string file)
{
	FILE* fp = fopen(file.c_str(), "r");

	if (fp == NULL)
	{
		printf("CObjectFactory ObjFromFile :: %s file open error\n", file);
		exit(1);
	}

	char tempStr[128];
	vec3 tempVertex;
	ivec3 tempFace;

	int index = 0;


	while (!feof(fp))
	{
		fscanf(fp, "%s", tempStr);
		//printf("%s", tempStr);
		if (strcmp(tempStr, "v") == 0)
		{
			fscanf(fp, "%f %f %f", &tempVertex.x, &tempVertex.y, &tempVertex.z);
			v->push_back(tempVertex);	//add new vertex
		}

		if (strcmp(tempStr, "f") == 0)
		{
			fscanf(fp, "%d %d %d", &tempFace.x, &tempFace.y, &tempFace.z);
			tempFace.x = abs(tempFace.x) - 1;		//인덱스가 1부터 시작 하므로 1 제거
			tempFace.y = abs(tempFace.y) - 1;
			tempFace.z = abs(tempFace.z) - 1;

			f->push_back(tempFace);		//add new face

			//calc new sprirng
			ivec3 vertexIndex = tempFace;

			float restLen[3] = { 0 };	//삼각형은 스프링 3개
			restLen[0] = glm::length((*v)[vertexIndex.x] - (*v)[vertexIndex.y]);
			restLen[1] = glm::length((*v)[vertexIndex.y] - (*v)[vertexIndex.z]);
			restLen[2] = glm::length((*v)[vertexIndex.z] - (*v)[vertexIndex.x]);

			CSpring sp[3];
			sp[0].Init(mKS, mKD, restLen[0], vertexIndex.x, vertexIndex.y);
			sp[1].Init(mKS, mKD, restLen[1], vertexIndex.y, vertexIndex.z);
			sp[2].Init(mKS, mKD, restLen[2], vertexIndex.z, vertexIndex.x);

			for (int i = 0; i<3; i++)
				AddSpring(s, sp[i]);	//add spring and check duplication
			index++;

		}

	}
}


//tet loader
void CObjectFactory::TetrahedonLoad(CMassSpringSystem *mss, float ks, float kd, float totalMass, vec3 pos, std::string file)
{
	mKS = ks;
	mKD = kd;
	mMass = totalMass;

	mPos = pos;

	mVertices.clear();
	mIndices.clear();
	mSprings.clear();

	std::string nodeFile(file);		nodeFile.append(".node");
	std::string faceFile(file);		faceFile.append(".face");
	std::string tetraFile(file);	tetraFile.append(".ele");

	NodeLoad(&mVertices, nodeFile);			//파일로 부터 정점, 면, 테트라헤드론 읽기
	//printf("Max Vertex Size : %d\n",MaxVertex);
	Matrix = (int **)malloc(sizeof(int *)* MaxVertex);

	for (int i = 0; i<MaxVertex; i++){ Matrix[i] = (int *)malloc(sizeof(int)* MaxVertex); for (int j = 0; j<MaxVertex; j++)Matrix[i][j] = 0; }

	FaceLoad(&mIndices, faceFile);
	ElementLoad(&mSprings, &mIndices, &mVertices, tetraFile);

	InitMassSpring(mss, &mVertices, &mIndices, &mSprings);	//스프링 중복 검사 및 데이터 저장
}


void CObjectFactory::NodeLoad(std::vector<vec3> *v, std::string file)
{
	//printf("node file open~ing\n");
	FILE *fp = fopen(file.c_str(), "r");
	if (fp == NULL)
	{
		printf("CObjectFactory node :: %s file open error\n", file);
		exit(1);
	}

	unsigned int maxVertices = 0;
	int t;
	fscanf(fp, "%d %d %d %d", &maxVertices, &t, &t, &t);

	vec3 temp;
	for (unsigned int i = 0; i < maxVertices; ++i)
	{
		fscanf(fp, "%d %f %f %f", &t, &temp.x, &temp.y, &temp.z);
		v->push_back(temp);
	}
	MaxVertex = maxVertices;
	//printf("node file close\n");
}
void CObjectFactory::FaceLoad(std::vector<ivec3> *v, std::string file)
{
	//printf("face file open\n");
	FILE *fp = fopen(file.c_str(), "r");
	
	if (fp == NULL)
	{
		printf("CObjectFactory face :: %s file open error\n", file);
		//exit(1);
	}
	else{

		unsigned int maxNodes = 0;
		int t;
		fscanf(fp, "%d %d", &maxNodes, &t);

		int tmp;
				
		ivec3 temp;
		for (unsigned int i = 0; i < maxNodes; ++i)
		{
			if (t == 1)
			{
				fscanf(fp, "%d %d %d %d %d", &tmp, &temp.x, &temp.y, &temp.z, &tmp);				

			}				
			else if (t == 0)
			{
				fscanf(fp, "%d %d %d %d", &tmp, &temp.x, &temp.y, &temp.z);				
			}
				
			if (!file.compare(boxFace) || !file.compare(sphereFace) || !file.compare(bunnyFace) || !file.compare(TorusFace)){

			}
			else if (!file.compare(alphaFace) || !file.compare(alphaLFace) || !file.compare(alphaxFace)){
				temp.x--; temp.y--; temp.z--;
			}
			v->push_back(temp);
		}
		//printf("face file close\n");		
	}
}
void CObjectFactory::ElementLoad(std::vector<CSpring> *s, std::vector<ivec3> *f, std::vector<vec3> *v, std::string file)
{
	//printf("element file open\n");
	FILE *fp = fopen(file.c_str(), "r");
	if (fp == NULL)
	{
		printf("CObjectFactory spring :: %s file open error\n", file);
		exit(1);
	}
	fp_spring = fopen("SPRING LEN.txt", "w");
	unsigned int maxElements = 0;
	int t = 0;
	fscanf(fp, "%d %d %d", &maxElements, &t, &t);
	CSpring sp[6];
	int vertexIndex[4] = { 0 };
	double restLen[6] = { 0 };
	GLuint node[4] = { 0 };
	vec3 posDirection;
	//printf("%u%",maxElements);
	for (unsigned int i = 0; i < maxElements; ++i)
	{
		fscanf(fp, "%d %d %d %d %d", &t, &node[0], &node[1], &node[2], &node[3]);

		//init spring info
		if (!file.compare(boxEle)){
			vertexIndex[0] = node[0];
			vertexIndex[1] = node[1];	//삼각형 중 정점1 위치
			vertexIndex[2] = node[2];	//삼각형 중 정점2 위치
			vertexIndex[3] = node[3];	//삼각형 중 정점2 위치
		}
		else if (!file.compare(sphereEle) || !file.compare(bunnyEle) || !file.compare(TorusEle)){
			vertexIndex[0] = node[0];
			vertexIndex[1] = node[1];	//삼각형 중 정점1 위치
			vertexIndex[2] = node[2];	//삼각형 중 정점2 위치
			vertexIndex[3] = node[3];	//삼각형 중 정점2 위치
		}
		else if (!file.compare(alphaAEle) || !file.compare(alphaLEle) || !file.compare(alphaXEle)){
			vertexIndex[0] = node[0] - 1;
			vertexIndex[1] = node[1] - 1;	//삼각형 중 정점1 위치
			vertexIndex[2] = node[2] - 1;	//삼각형 중 정점2 위치
			vertexIndex[3] = node[3] - 1;	//삼각형 중 정점2 위치
		}

		posDirection = (*v)[vertexIndex[0]] - (*v)[vertexIndex[1]];
		restLen[0] = glm::length(posDirection);
		posDirection = (*v)[vertexIndex[1]] - (*v)[vertexIndex[2]];
		restLen[1] = glm::length(posDirection);
		posDirection = (*v)[vertexIndex[0]] - (*v)[vertexIndex[2]];
		restLen[2] = glm::length(posDirection);
		posDirection = (*v)[vertexIndex[0]] - (*v)[vertexIndex[3]];
		restLen[3] = glm::length(posDirection);
		posDirection = (*v)[vertexIndex[1]] - (*v)[vertexIndex[3]];
		restLen[4] = glm::length(posDirection);
		posDirection = (*v)[vertexIndex[2]] - (*v)[vertexIndex[3]];
		restLen[5] = glm::length(posDirection);

		/*restLen[0] = sqrt(((*v)[vertexIndex[0]].x - (*v)[vertexIndex[1]].x) * ((*v)[vertexIndex[0]].x - (*v)[vertexIndex[1]].x) +
			((*v)[vertexIndex[0]].y - (*v)[vertexIndex[1]].y) * ((*v)[vertexIndex[0]].y - (*v)[vertexIndex[1]].y) +
			((*v)[vertexIndex[0]].z - (*v)[vertexIndex[1]].z) * ((*v)[vertexIndex[0]].z - (*v)[vertexIndex[1]].z));
		restLen[1] = sqrt(((*v)[vertexIndex[1]].x - (*v)[vertexIndex[2]].x) * ((*v)[vertexIndex[1]].x - (*v)[vertexIndex[2]].x) +
			((*v)[vertexIndex[1]].y - (*v)[vertexIndex[2]].y) * ((*v)[vertexIndex[1]].y - (*v)[vertexIndex[2]].y) +
			((*v)[vertexIndex[1]].z - (*v)[vertexIndex[2]].z) * ((*v)[vertexIndex[1]].z - (*v)[vertexIndex[2]].z));
		restLen[2] = sqrt(((*v)[vertexIndex[0]].x - (*v)[vertexIndex[2]].x) * ((*v)[vertexIndex[0]].x - (*v)[vertexIndex[2]].x) +
			((*v)[vertexIndex[0]].y - (*v)[vertexIndex[2]].y) * ((*v)[vertexIndex[0]].y - (*v)[vertexIndex[2]].y) +
			((*v)[vertexIndex[0]].z - (*v)[vertexIndex[2]].z) * ((*v)[vertexIndex[0]].z - (*v)[vertexIndex[2]].z));
		restLen[3] = sqrt(((*v)[vertexIndex[0]].x - (*v)[vertexIndex[3]].x) * ((*v)[vertexIndex[0]].x - (*v)[vertexIndex[3]].x) +
			((*v)[vertexIndex[0]].y - (*v)[vertexIndex[3]].y) * ((*v)[vertexIndex[0]].y - (*v)[vertexIndex[3]].y) +
			((*v)[vertexIndex[0]].z - (*v)[vertexIndex[3]].z) * ((*v)[vertexIndex[0]].z - (*v)[vertexIndex[3]].z));
		restLen[4] = sqrt(((*v)[vertexIndex[1]].x - (*v)[vertexIndex[3]].x) * ((*v)[vertexIndex[1]].x - (*v)[vertexIndex[3]].x) +
			((*v)[vertexIndex[1]].y - (*v)[vertexIndex[3]].y) * ((*v)[vertexIndex[1]].y - (*v)[vertexIndex[3]].y) +
			((*v)[vertexIndex[1]].z - (*v)[vertexIndex[3]].z) * ((*v)[vertexIndex[1]].z - (*v)[vertexIndex[3]].z));
		restLen[5] = sqrt(((*v)[vertexIndex[2]].x - (*v)[vertexIndex[3]].x) * ((*v)[vertexIndex[2]].x - (*v)[vertexIndex[3]].x) +
			((*v)[vertexIndex[2]].y - (*v)[vertexIndex[3]].y) * ((*v)[vertexIndex[2]].y - (*v)[vertexIndex[3]].y) +
			((*v)[vertexIndex[2]].z - (*v)[vertexIndex[3]].z) * ((*v)[vertexIndex[2]].z - (*v)[vertexIndex[3]].z));*/

		fprintf(fp_spring, "spring index[%d], %f %f %f %f %f %f\n", i, restLen[0], restLen[1], restLen[2], restLen[3], restLen[4], restLen[5]);

		sp[0].Init(mKS, mKD, restLen[0], vertexIndex[0], vertexIndex[1]);
		sp[1].Init(mKS, mKD, restLen[1], vertexIndex[1], vertexIndex[2]);
		sp[2].Init(mKS, mKD, restLen[2], vertexIndex[0], vertexIndex[2]);
		sp[3].Init(mKS, mKD, restLen[3], vertexIndex[0], vertexIndex[3]);
		sp[4].Init(mKS, mKD, restLen[4], vertexIndex[1], vertexIndex[3]);
		sp[5].Init(mKS, mKD, restLen[5], vertexIndex[2], vertexIndex[3]);

		for (int k = 0; k < 6; ++k)
			AddSpring(s, sp[k]);
	}
	//printf("element file close\n");
	fclose(fp_spring);
	for (int i = 0; i<MaxVertex; i++) free(Matrix[i]);
	free(Matrix);
}

void CObjectFactory::CheckDuplicateSprings(CMassSpringSystem *mms)
{
	static int a = 0;
	CSpring *sp1, *sp2;
	for (int i = 0; i < mms->mNumSprings; ++i)	//전체 스프링 중에
	{
		sp1 = mms->GetSpring(i);			//하나 골라서

		for (int j = i + 1; j < mms->mNumSprings; ++j)	//나머지 중에
		{
			sp2 = mms->GetSpring(j);	//하나 골라서

			if (sp1 != sp2)						//자기 자신은 제외 하고 나머지 중에
			{
				if ((sp1->mIndex1 == sp2->mIndex1 && sp1->mIndex2 == sp2->mIndex2) ||
					sp1->mIndex1 == sp2->mIndex2 && sp1->mIndex2 == sp2->mIndex1)
				{
					printf("duplicate spring exist :: spring = %d\n", j);
					a++;
				}
			}
		}
	}

	printf("total spring num = %d\n", mms->mNumSprings);
	printf("duplicate spring num = %d\n", a);
}

void CObjectFactory::AddSpring(std::vector<CSpring> *s, CSpring spring)
{
	CSpring newSp = spring;
	bool bDuplicate = false;

	//전체 스프링중에서 새로운 스프링과 중복되는 스프링이 존재 하는지 검사
	/*for(std::vector<CSpring>::iterator sp = (*s).begin(); sp != (*s).end(); ++sp)
	{
	if((newSp.mIndex1 == (*sp).mIndex1 && newSp.mIndex2 == (*sp).mIndex2) ||
	(newSp.mIndex1 == (*sp).mIndex2 && newSp.mIndex2 == (*sp).mIndex1))
	{
	bDuplicate = true;
	break;
	}
	}*/


	if (Matrix[newSp.mIndex1][newSp.mIndex2] == 1)
	{
		bDuplicate = true;
	}
	else
	{
		Matrix[newSp.mIndex1][newSp.mIndex2] = 1;
		Matrix[newSp.mIndex2][newSp.mIndex1] = 1;
	}

	/*for(int i = 0; i < s->size(); ++i)
	{
	if((newSp.mIndex1 == (*s)[i].mIndex1 && newSp.mIndex2 == (*s)[i].mIndex2))
	{
	bDuplicate = true;
	break;
	}
	else if((newSp.mIndex1 == (*s)[i].mIndex2 && newSp.mIndex2 == (*s)[i].mIndex1))
	{
	bDuplicate = true;
	break;
	}
	}*/

	if (bDuplicate != true)		//중복이 아님
	{
		s->push_back(newSp);	//리스트에 추가
	}
	//중복이면 그냥 넘어가기
}