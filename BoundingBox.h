#include "glsl.h"

class CBox
{
//메소드
public:
	CBox();
	~CBox();
	void setPosition(float minX, float maxX, float minY, float maxY, float minZ, float maxZ);
	void render();	

//필드
public:
	float minX, maxX;
	float minY, maxY;
	float minZ, maxZ;
	float level;
};
