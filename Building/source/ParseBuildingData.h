/********************************************************************
Copyright (C), 2016, tianying All rights reserved.
File:	    ParseBuildingData.h
Author:		tianying
Create:		2016/01/05
Purpose:	3D building 3ds Max model
*********************************************************************/
#include <map>
#include <vector>
#include <string>

using namespace std;
#include "common.h"
const double SNAP_COORDINATE = 0.0000000001;

struct POINT2D
{
	double x,y;
};
struct POINT2F
{
	float x,y;
};
struct Building
{
	unsigned short height;
	POINT2D* pPoints;
	char   ptCnt;
	Building( unsigned short h, char cnt, POINT2D* pPts=NULL )
	{
		height = h;
		ptCnt = cnt;
		pPoints = (POINT2D*)malloc(ptCnt*sizeof(POINT2D));
		if( pPts!=NULL )
		{
			memcpy(pPoints, pPts, ptCnt*sizeof(POINT2D));
		}
	}
	~Building()
	{
		free(pPoints);
	}
private:
	Building(const Building& rhs)
	{
		if( this!=&rhs )
		{
			height = rhs.height;
			ptCnt = rhs.ptCnt;
			pPoints = (POINT2D*)malloc(ptCnt*sizeof(POINT2D));
			memcpy(pPoints, rhs.pPoints, ptCnt*sizeof(POINT2D));
		}
	}
	Building& operator=(const Building& rhs)
	{
		if( this!=&rhs )
		{
			height = rhs.height;
			ptCnt = rhs.ptCnt;
			pPoints = (POINT2D*)malloc(ptCnt*sizeof(POINT2D));
			memcpy(pPoints, rhs.pPoints, ptCnt*sizeof(POINT2D));
		}
		return *this;
	}

private:
	Building(){}
};

class ParseBuildingData
{
public:
	ParseBuildingData(){}
	~ParseBuildingData()
	{

	}

	void addBuilding(vector<double>& vecPosition, unsigned short buildingHeight);

	vector<Building*> m_buildings;
};

