/********************************************************************
Copyright (C), 2016, tianying All rights reserved.
File:	    ParseBuildingData.cpp
Author:		tianying
Create:		2016/01/05
Purpose:	3D building 3ds Max model
*********************************************************************/
#include "ParseBuildingData.h"

static void filterDuplicatePts( POINT2D*& ppt, unsigned short& nCount )
{
	unsigned short i, j, k;
	i = 0;
	while (i < nCount -1) 
	{
		j = i + 1;
		while (j < nCount) 
		{
			if (abs(ppt[i].x - ppt[j].x) < SNAP_COORDINATE && abs(ppt[i].y - ppt[j].y) < SNAP_COORDINATE)
			{
				for (k = j + 1; k < nCount; k++) 
				{
					ppt[k - 1] = ppt[k];
				}
				nCount--;
			}
			j++;
		}
		i++;
	}
}

//同一条直线上的点
static void filterInsidePts( POINT2D*& ppt, unsigned short& nCount )
{
	unsigned short i, k;
	for (i = 0; i < nCount - 2; i++) 
	{
		if ((abs(ppt[i].x - ppt[i+1].x)< SNAP_COORDINATE && abs(ppt[i+1].x - ppt[i+2].x) < SNAP_COORDINATE) ||
			(abs(ppt[i].y - ppt[i+1].y) < SNAP_COORDINATE && abs(ppt[i+1].y - ppt[i+2].y) < SNAP_COORDINATE) ||
			(abs((ppt[i].x-ppt[i+1].x) * (ppt[i+2].y-ppt[i+1].y) - (ppt[i+2].x-ppt[i+1].x) * (ppt[i].y-ppt[i+1].y)) < SNAP_COORDINATE &&
			 (ppt[i+1].x-ppt[i].x) * (ppt[i+1].x-ppt[i+2].x) <= 0 &&
			 (ppt[i+1].y-ppt[i].y) * (ppt[i+1].y-ppt[i+2].y) <= 0)) 
		{
			for (k = i + 2; k < nCount; k++) 
			{
				ppt[k - 1] = ppt[k];
			}
			nCount--;
		}
	}
}

void ParseBuildingData::addBuilding(vector<double>& vecPosition, unsigned short buildingHeight)
{
	unsigned short ptCnt = vecPosition.size()/2;

	POINT2D* ppt = (POINT2D*)malloc( ptCnt*sizeof(POINT2D) );
	for(int n=0; n<ptCnt; ++n)
	{
		do_decode(vecPosition[n*2], vecPosition[n*2+1], ppt[n].x, ppt[n].y );
	}
	filterDuplicatePts(ppt, ptCnt);
	filterInsidePts(ppt, ptCnt);
	Building* pbd = new Building(buildingHeight, ptCnt);
	POINT2D*& pPoints = pbd->pPoints;
	for(int i=0; i<ptCnt; ++i)
	{
		pPoints[i].x = ppt[i].x;
		pPoints[i].y = ppt[i].y;
	}
	free(ppt);
	m_buildings.push_back(pbd);
}
