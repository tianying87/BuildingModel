/********************************************************************
Copyright (C), 2016, tianying All rights reserved.
File:	    BuildingsTo3DS.cpp
Author:		tianying
Create:		2016/01/05
Purpose:	3D building 3ds Max model
*********************************************************************/

#include "BuildingsTo3DS.h"
#include <math.h>
#include "lib3ds/lib3ds.h"

const float LONLAT2NORMALCOORD = 115200.0f;

ParseBuildingData* BuildingsTo3DS::m_buildingsPerMesh = NULL;
int BuildingsTo3DS::m_meshID = 0;
/***************************************/

const double PI = 3.1415926535; 
//三角化
//求多边形面积
static double IsInArea (POINT2D *points, int count) 
{
	double A = 0.0f;
	int p, q;
	for (p = count-1, q = 0; q < count; p = q++)
	{
		A += points[p].x  *points[q].y - points[q].x * points[p].y;
	}
	return A*0.5f;

}

static bool InsideTriangle(float Ax, float Ay, float Bx, float By, float Cx, float Cy, float Px, float Py)
{
	float ax, ay, bx, by, cx, cy, apx, apy, bpx, bpy, cpx, cpy;
	float cCROSSap, bCROSScp, aCROSSbp;

	ax = Cx - Bx;  
	ay = Cy - By;
	bx = Ax - Cx;  
	by = Ay - Cy;
	cx = Bx - Ax;  
	cy = By - Ay;
	apx= Px - Ax;  
	apy= Py - Ay;
	bpx= Px - Bx;  
	bpy= Py - By;
	cpx= Px - Cx;  
	cpy= Py - Cy;

	aCROSSbp = ax*bpy - ay*bpx;
	cCROSSap = cx*apy - cy*apx;
	bCROSScp = bx*cpy - by*cpx;

	return ((aCROSSbp >= 0.0f) && (bCROSScp >= 0.0f) && (cCROSSap >= 0.0f));
}


static bool Snip(POINT2D *points,int u,int v,int w,int n,int *V,float& devi)
{
	float Ax, Ay, Bx, By, Cx, Cy, Px, Py;
	Ax = points[V[u]].x;
	Ay = points[V[u]].y;
	Bx = points[V[v]].x;
	By = points[V[v]].y;
	Cx = points[V[w]].x;
	Cy = points[V[w]].y;

	//const float EPSILON=0.0000000001f;
	const float EPSILON=0.0f;
	if ( EPSILON > (((Bx-Ax)*(Cy-Ay)) - ((By-Ay)*(Cx-Ax))) ) 
	{
		return false;
	}

	int p;
	for (p = 0; p < n; p++)
	{
		if( (p == u) || (p == v) || (p == w) ) 
		{
			continue;
		}

		Px = points[V[p]].x;
		Py = points[V[p]].y;
		if (InsideTriangle(Ax,Ay,Bx,By,Cx,Cy,Px,Py)) 
		{
			return false;
		}
	}

	//cosA = (c^2 + b^2 - a^2) / (2·b·c)  余弦定理
	float AAngle,BAngle,CAngle;
	double a, b, c,aa, bb, cc;
	aa = (Bx - Cx) * (Bx - Cx) + (By - Cy) * (By - Cy);
	bb = (Ax - Cx) * (Ax - Cx) + (Ay - Cy) * (Ay - Cy);
	cc = (Bx - Ax) * (Bx - Ax) + (By - Ay) * (By - Ay);

	a = sqrtf(aa);
	b = sqrtf(bb);
	c = sqrtf(cc);

	AAngle = acosf((cc + bb- aa)/(2*b*c))*180/PI;
	BAngle = acosf((cc + aa- bb)/(2*a*c))*180/PI;
	CAngle = acosf((aa + bb- cc)/(2*b*a))*180/PI;

	devi = (AAngle - 60) * (AAngle - 60) + (BAngle - 60) * (BAngle - 60) + (CAngle - 60) * (CAngle - 60);

	return true;
}

//判断点是否是顺时针排列
static bool IsTrianlgeClockWise(POINT2D *points,int a,int b,int c)
{
	double tolerance = 0.000000000001;
	POINT2D s,e,o;

	double z =0 ;		
	o = points[ a ];
	s = points[ b ];
	e = points[ c ];
	z = (s.x - o.x)*(e.y - o.y) - (e.x-o.x )*(s.y - o.y);	

	if ( z > tolerance )
	{
		return false;
	}
	else
	{
		return true;
	}
}

static void CalcuCoordByMeshCode(int meshID, int &a_iGeoX, int &a_iGeoY)
{
	int dwMesh1Lati;
	int dwMesh1Longi;
	int dwMesh2Lati;
	int dwMesh2Longi;
	dwMesh1Lati = meshID / 10000;
	dwMesh1Longi = (meshID % 10000) / 100;
	dwMesh2Lati = (meshID % 100) / 10;
	dwMesh2Longi = meshID % 10;
	a_iGeoX = (int)((dwMesh1Longi + 50 + 10) * 60 * 60 + dwMesh2Longi * 7.5 * 60 ) * 32;	//2次mesh左下角x坐标，单位：1/32sec
	a_iGeoY = (dwMesh1Lati * 40 * 60 + dwMesh2Lati * 5 * 60) * 32;	//2次mesh左下角y坐标，单位：1/32sec
}

static int ConvertPolygons (POINT2D *points, int num,vector<TriangleObj> & arrBackTriangle) 
{
	int *V = (int *)malloc(sizeof(int)*num);
	int v;
	int n = num;
	int nv;
	int count;
	int m, u, w;

	if (n < 3) 
	{
		free (V);
		return 0;
	}
	nv = n;
	count = 2*nv;
	if ( 0.0f < IsInArea(points, n) ) 
	{
		for (v=0; v<n; v++)
		{
			V[v] = v;
		}
	} 
	else 
	{
		for (v=0; v<n; v++)
		{
			V[v] = (n-1)-v;
		}
	}

	long iTri = 0;

	for (m=0, v=nv-1; nv>2; )
	{
		/* if we loop, it is probably a non-simple polygon */
		if (0 >= (count--))
		{
			//** Triangulate: ERROR - probable bad polygon!
			free (V);
			return 0;
		}

		/* three consecutive vertices in current polygon, <u,v,w> */
		u = v  ; if (nv <= u) u = 0;     /* previous */
		v = u+1; if (nv <= v) v = 0;     /* new v    */

		// find best w (form average triangle)
		w = v+1; if (nv <= w) w = 0;     /* next     */
		
		float Devi1,Devi2;
		if(!Snip(points,u,v,w,nv,V,Devi1))
		{
			continue;
		}

		int w2= u-1;
		if (w2 < 0)
		{
			w2 = nv-1;
		}
		bool bResult = false;

		bResult = Snip(points,w2,u,v,nv,V,Devi2);
		
		//通过比较顺时针和逆时针连个个三角形与正三角形的方差，选择最优三角形
		
		if (bResult && Devi2 < Devi1)
		{
			int uTemp = u;
			int vTemp = v;
			u = w2;
			v = uTemp;
			w = vTemp;
		}

		int a,b,c,s,t;
		/* true names of the vertices */
		a = V[u]; b = V[v]; c = V[w];

		if (IsTrianlgeClockWise(points,a,b,c))  //如果是顺时针，改成逆时针排列
		{
			int temp;
			temp = a;
			a = c;
			c = temp;
		}

		/* output Triangle */
		TriangleObj backTriTemp; // TriangleObj就是uint16* triIndex = (uint16 *)pTri;的pTri
		backTriTemp.x = a;
		backTriTemp.y = b;
		backTriTemp.z = c;
		// Remove point if in straight line  修改到前面去掉中间点
		bool bFormLine = false;
		if ((abs(points[a].x - points[b].x) < SNAP_COORDINATE && abs(points[b].x - points[c].x) < SNAP_COORDINATE) ||
			(abs(points[a].y - points[b].y) < SNAP_COORDINATE && abs(points[b].y - points[c].y) < SNAP_COORDINATE)) 
		{
			bFormLine = true;

		}
		if (!bFormLine) 
		{
			arrBackTriangle.push_back(backTriTemp);
			m++;
		}
		
		/* remove v from remaining polygon */
		for(s=v,t=v+1; t < nv; s++,t++) 
		{
			V[s] = V[t]; 
		}
		nv--;
		/* resest error detection counter */
		count = 2*nv;

	}
	free (V);
	return m;
}

/***************************************/


void BuildingsTo3DS::addBuilding(vector<double>& vecPosition, unsigned short buildingHeight)
{
	m_buildingsPerMesh->addBuilding(vecPosition,buildingHeight);
}

void BuildingsTo3DS::generate3DSFile()
{
	/*beijing demo, start*/
	FILE* baseFP = fopen("D:\\ty_building\\baseCoord.txt","ab+");
	fseek(baseFP,0,SEEK_END);
	/*beijing demo, end*/

	//计算mesh左下角坐标点
	int meshMinX, meshMinY;
	CalcuCoordByMeshCode(m_meshID,meshMinX,meshMinY);

	vector<Building*>& vBuildings = m_buildingsPerMesh->m_buildings;
	unsigned int bdCnt = vBuildings.size();
	unsigned int idx = 0;
	bool hasData = true;

	while(hasData)
	{
		Lib3dsFile *file = lib3ds_file_new();
		int q=0;
		unsigned int n;
		POINT2D baseCoord;
		bool bChangedBaseCoord = false;
		for(n=0; n<bdCnt/*beijing demo:50*/; ++n)
		{
			/*beijing demo, start*/
			//if( n-idx==50 )
			//{
			//	idx = n;
			//	break;
			//}
			/*beijing demo, end*/
			POINT2D* pPts = vBuildings[n]->pPoints;
			int ptCnt = (int)(vBuildings[n]->ptCnt);
			int buildingHi = vBuildings[n]->height;

			if(!bChangedBaseCoord)
			{
				baseCoord.x = pPts[0].x*LONLAT2NORMALCOORD;
				baseCoord.y = pPts[0].y*LONLAT2NORMALCOORD;
				bChangedBaseCoord = true;
			}

			POINT2F* pPts_GLCoord = (POINT2F*)malloc(ptCnt*sizeof(POINT2F));
			for(unsigned int j=0; j<ptCnt; ++j)
			{
				pPts_GLCoord[j].x = pPts[j].x*LONLAT2NORMALCOORD - baseCoord.x /*- meshMinX*/;
				pPts_GLCoord[j].y = pPts[j].y*LONLAT2NORMALCOORD - baseCoord.y /*- meshMinY*/;
			}

			//(ptCnt-2)*2+ptCnt*2 ,face NUM
			int faceNum = (ptCnt-2)*2+ptCnt*2;
			unsigned short* g_indicesMulti=(unsigned short*)malloc(faceNum*3*sizeof(unsigned short));
			for(int qq=0; qq<ptCnt; ++qq) //侧面
			{
				g_indicesMulti[qq*6+0] = qq;		
				g_indicesMulti[qq*6+3] = qq;
				g_indicesMulti[qq*6+5] = ptCnt+qq;

				if(qq<ptCnt-1)
				{
					g_indicesMulti[qq*6+2] = ptCnt+qq+1;
					g_indicesMulti[qq*6+1] = qq+1;
					g_indicesMulti[qq*6+4] = ptCnt+qq+1;
				}
				else
				{
					g_indicesMulti[qq*6+2] = ptCnt;
					g_indicesMulti[qq*6+1] = 0;
					g_indicesMulti[qq*6+4] = ptCnt;
				}
			}

			int u2TriNum = ptCnt-2;
			vector<TriangleObj> vTriIdx;
			ConvertPolygons(pPts,ptCnt,vTriIdx);

			unsigned short* triIndex = (unsigned short*)malloc(u2TriNum*3*sizeof(unsigned short));
			for(int m=0; m<vTriIdx.size(); ++m)
			{
				triIndex[3*m+0] = vTriIdx[m].x;
				triIndex[3*m+1] = vTriIdx[m].y;
				triIndex[3*m+2] = vTriIdx[m].z;
			}
			if(vTriIdx.size()!=u2TriNum)
			{	
				continue;
			}
			int tmpIdx = 0;

			for(int qq=0; qq<u2TriNum; ++qq)//顶面
			{
				g_indicesMulti[qq*3+0+ptCnt*6] = triIndex[tmpIdx];
				g_indicesMulti[qq*3+1+ptCnt*6] = triIndex[tmpIdx+1];
				g_indicesMulti[qq*3+2+ptCnt*6] = triIndex[tmpIdx+2];
				tmpIdx += 3;
			}

			tmpIdx = 0;
			for(int qq=0; qq<u2TriNum; ++qq)//底面
			{
				g_indicesMulti[qq*3+0+ptCnt*6+u2TriNum*3] = triIndex[tmpIdx]+ptCnt;
				g_indicesMulti[qq*3+1+ptCnt*6+u2TriNum*3] = triIndex[tmpIdx+1]+ptCnt;
				g_indicesMulti[qq*3+2+ptCnt*6+u2TriNum*3] = triIndex[tmpIdx+2]+ptCnt;
				tmpIdx += 3;
			}

			// 创建3ds数据
			int i, j;
			++q;
			char num[1024]={0};
			string fName = "building";
			fName += itoa(q,num,10);
			char num1[1024]={0};
			string meshNum = itoa(q,num1,10);
			Lib3dsMesh *mesh = lib3ds_mesh_new(fName.c_str());
			Lib3dsMeshInstanceNode *inst;        
			lib3ds_file_insert_mesh(file, mesh, -1);

			int vertNum = ptCnt*2;
			lib3ds_mesh_resize_vertices(mesh, vertNum, 1, 0);
			for (i = 0; i < vertNum/2; ++i) {
				float verts[3] = {pPts_GLCoord[i].x,-pPts_GLCoord[i].y,buildingHi};
				lib3ds_vector_copy(mesh->vertices[i], verts);
			}
			for (i = vertNum/2; i < vertNum; ++i) {
				float verts[3] = {pPts_GLCoord[i-vertNum/2].x,-pPts_GLCoord[i-vertNum/2].y,0.0};
				lib3ds_vector_copy(mesh->vertices[i], verts);
			}

			lib3ds_mesh_resize_faces(mesh, faceNum);
			for (i = 0; i < faceNum; ++i) {
				for (j = 0; j < 3; ++j) {
					mesh->faces[i].index[j] = g_indicesMulti[i*3+j];
				}
			}
			inst = lib3ds_node_new_mesh_instance(mesh, meshNum.c_str(), NULL, NULL, NULL);
			lib3ds_file_append_node(file, (Lib3dsNode*)inst, NULL);


			free(g_indicesMulti);
			free(pPts_GLCoord);
			free(triIndex);
		}

		char fileName[256] = {0};
		if( n==bdCnt )
			idx = bdCnt;
		int subFileNum = idx/50;
		if( idx%50!=0 )
		{
			subFileNum += 1;
		}
		sprintf(fileName,"d:\\ty_building\\beijing.3ds");
		if (!lib3ds_file_save(file, fileName)) {
			fprintf(stderr, "ERROR: Saving 3ds file failed!\n");
		}
		lib3ds_file_free(file);

		/*beijing demo, start*/
		char baseStr[256] = {0};
		sprintf(baseStr,"%d_%d \r\n",m_meshID,subFileNum);
		char baseStrX[256] = {0};
		sprintf(baseStrX,"%f",baseCoord.x/LONLAT2NORMALCOORD);
		char baseStrY[256] = {0};
		sprintf(baseStrY,"%f",baseCoord.y/LONLAT2NORMALCOORD);
		string baseStr1("X:"); baseStr1 += baseStrX; baseStr1+="    Y:";baseStr1 += baseStrY; baseStr1 += "\r\n";

		fwrite(baseStr1.c_str(),baseStr1.length(),1,baseFP);
		/*beijing demo, end*/
		if( n==bdCnt )
			hasData = false;
	}

	/*beijing demo, start*/
	fclose(baseFP);
	/*beijing demo, end*/
}

void BuildingsTo3DS::releaseMeshData()
{
	for(int i=0; i<m_buildingsPerMesh->m_buildings.size(); ++i)
	{
		Building* pbd = m_buildingsPerMesh->m_buildings[i];
		delete pbd;
		pbd = NULL;
	}
	delete m_buildingsPerMesh;
	m_buildingsPerMesh = NULL;
}

void BuildingsTo3DS::init(int meshID)
{
	m_meshID = meshID;
	if( m_buildingsPerMesh==NULL )
	{
		m_buildingsPerMesh = new ParseBuildingData();
	}
}





