#include "glsl.h"

class CBox
{
//�޼ҵ�
public:
	CBox();
	~CBox();
	void setPosition(float minX, float maxX, float minY, float maxY, float minZ, float maxZ);
	void render();	

//�ʵ�
public:
	float minX, maxX;
	float minY, maxY;
	float minZ, maxZ;
	float level;
};
