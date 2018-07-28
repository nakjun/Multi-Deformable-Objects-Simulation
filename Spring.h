#pragma once

//spring class
class CSpring
{
public:
	CSpring(void);
	~CSpring(void);
	void Init(float ks, float kd, float len, int idx1, int idx2)
	{
		mKs = ks;	mKd = kd;	mRestLen = len; mIndex1 = idx1; mIndex2 = idx2;
	};

public: 
	int mIndex1;	//spring's two particles numnber
	int mIndex2;

	float mRestLen;	//initialize length that is between pt1 and pt2

	int forceindex1;
	int forceindex2;

	//CPU »ç¿ë
	float mKs;		//spring constant
	float mKd;		//damping coefiicient

};