/********************************************************************
Copyright (C), 2016, tianying All rights reserved.
File:	    main.cpp
Author:		tianying
Create:		2016/01/05
Purpose:	3D building 3ds Max model
*********************************************************************/
#include "BuildingsTo3DS.h"
#include "sqlite3/sqlite3.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

void * g_dbHandle;
int main()
{
	const char* dbName="D:\\Building_13win.db";
	int iRet = sqlite3_open(dbName, (sqlite3 **)&g_dbHandle);
	if (iRet != SQLITE_OK) 
		assert(0);

	char  strSql[512] ={0};
	sqlite3_stmt* stmt=NULL;
	const   char* tail=NULL;

	sprintf(strSql,  
		/*test: "SELECT  %s,%s,%s  FROM %s WHERE %s=\"%s\"",             
		"MapID","Height","GEOMETRY","Building","MapID","595673"*/
		"SELECT  %s,%s,%s  FROM %s WHERE %s=\"%s\" AND CAST(%s AS INTEGER)>=%s ORDER BY %s ASC",             
		"MapID","Height","GEOMETRY","Building","SHPAREA","beijing","Height","100","MapID");

	int  strLen=strlen(strSql);
	unsigned int rc = sqlite3_prepare((sqlite3 *)(g_dbHandle), strSql, 
		strLen, &stmt, &tail);
	if(rc != SQLITE_OK) 
	{            
		const char* errMsg=sqlite3_errmsg((sqlite3 *)(g_dbHandle));
		assert(0);
	}

	rc = sqlite3_step(stmt);
	int preMeshID=0;
	/* Release version, start*/
	if( rc==SQLITE_ROW )  
	{  
		unsigned char* meshCode = const_cast<unsigned char*>(sqlite3_column_text(stmt,0));
		int buildingHeight = sqlite3_column_int(stmt,1);
		char* position = (char*)sqlite3_column_blob( stmt, 2 );
		int len = sqlite3_column_bytes( stmt, 2 );
		vector<double>& vPts = GetPosition(position, len);
		int meshID = atoi((char*)meshCode);

		BuildingsTo3DS::init(meshID);
		BuildingsTo3DS::addBuilding(vPts, (unsigned short)buildingHeight);

		preMeshID = meshID;

		rc = sqlite3_step(stmt);
	}

	while( rc==SQLITE_ROW )  
	{  
		unsigned char* meshCode = const_cast<unsigned char*>(sqlite3_column_text(stmt,0));
		int buildingHeight = sqlite3_column_int(stmt,1);
		char* position = (char*)sqlite3_column_blob( stmt, 2 );
		int len = sqlite3_column_bytes( stmt, 2 );
		vector<double>& vPts = GetPosition(position, len);
		int meshID = atoi((char*)meshCode);
		if( meshID!=preMeshID )
		{
			BuildingsTo3DS::generate3DSFile();
			BuildingsTo3DS::releaseMeshData();
			BuildingsTo3DS::init(meshID);
			BuildingsTo3DS::addBuilding(vPts, (unsigned short)buildingHeight);
			preMeshID = meshID;
		}
		BuildingsTo3DS::addBuilding(vPts, (unsigned short)buildingHeight);
		
		rc = sqlite3_step(stmt);
	}
	/* Release version, end*/

	/* one mesh demo, start */
	//while( rc==SQLITE_ROW )  
	//{  
	//	unsigned char* meshCode = const_cast<unsigned char*>(sqlite3_column_text(stmt,0));
	//	int buildingHeight = sqlite3_column_int(stmt,1);
	//	char* position = (char*)sqlite3_column_blob( stmt, 2 );
	//	int len = sqlite3_column_bytes( stmt, 2 );
	//	vector<double>& vPts = GetPosition(position, len);
	//	int meshID = atoi((char*)meshCode);

	//	BuildingsTo3DS::init(meshID);
	//	BuildingsTo3DS::addBuilding(vPts, (unsigned short)buildingHeight);

	//	preMeshID = meshID;

	//	rc = sqlite3_step(stmt);
	//}
	/* one mesh demo, end */

	BuildingsTo3DS::generate3DSFile();
	BuildingsTo3DS::releaseMeshData();

	sqlite3_finalize(stmt); 


	if (g_dbHandle)
	{
		sqlite3_close((sqlite3 *)g_dbHandle);
		g_dbHandle=NULL;
	} 
	return 0;
}