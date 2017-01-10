/********************************************************************
Copyright (C), 2016, tianying All rights reserved.
File:	    BuildingsTo3DS.h
Author:		tianying
Create:		2016/01/05
Purpose:	3D building 3ds Max model
*********************************************************************/
#include "ParseBuildingData.h"
#include <map>
using namespace std;

/* 三角形实体 */ 
struct tagTriangleObj
{
	long x,y,z;
	tagTriangleObj(long a=0, long b=0, long c = 0) { x=a; y=b;z=c;}
};
typedef struct tagTriangleObj TriangleObj;

class BuildingsTo3DS
{
public:
	static void init(int meshID);
	static void addBuilding(vector<double>& vecPosition, unsigned short buildingHeight);
	static void generate3DSFile();
	static void releaseMeshData();
private:

	static int m_meshID;
	static ParseBuildingData* m_buildingsPerMesh; 
};