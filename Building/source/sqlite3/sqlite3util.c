#include <stdlib.h>
#include "sqlite3.h"
#include "sqlite3util.h" 

int isTableExists(sqlite3*  aHDB,
                  const char* aTableName
                  )
{ 
    int  tblCnt=0;
    char** dbResult = NULL; //是char ** 类型，两个*号
    int   nRow=0, nColumn=0;
    int   ret =0;  

    char strSql[128];
    sprintf(strSql,
        "select count(*) from sqlite_master where type='table' and name='%s'",
        aTableName); 

    ret = sqlite3_get_table(aHDB,strSql, &dbResult, &nRow, &nColumn, NULL );
    if(ret == SQLITE_OK) 
    {
        if( (nRow ==1) && (nColumn ==1) )
        {
            tblCnt=atoi(dbResult[nColumn]); 
        }        
    } 
    sqlite3_free_table(dbResult);

    return ( tblCnt> 0 );
}

int  table_clear(sqlite3*  aHDB,
    const char* aTableName
    )
{
    int ret=0;
    char zSql[256] = {0}; 
    do 
    { 
        // 清空 TRKPLANTmp_TABLE 表
        sprintf(zSql,"DELETE FROM %s",aTableName); 
        ret = sqlite3_exec( aHDB, zSql, 0, 0, 0 );
        if (ret!= SQLITE_OK) 
        {
            const char *error = sqlite3_errmsg(aHDB);
            ret = -100;
        }

    } while (0); 

    return ret;
}


//SHOWTIME("CSqlBinFile::doesBlockExist");
//char szSQL[128];
//sprintf( szSQL, "select count(*) from %s where bid = %d;", m_szTbl, nBid );//
//sqlite3_stmt *stmt = execPrepareStep( szSQL );
//if( !stmt ) return 0;
//int nCnt = atoi( (const char*)sqlite3_column_text(stmt, 0) );
//sqlite3_finalize(stmt);
//return nCnt;

sqlite3_stmt* 
    execPrepareStep(sqlite3*  aHDB, 
                    const char* szSQL 
                    )
{ 
    sqlite3_stmt *stmt = NULL;
    do 
    {
        int nRet = sqlite3_prepare_v2(aHDB, szSQL, -1, &stmt, 0);
        if (nRet != SQLITE_OK)
        {
            //fprintf(stderr,"sqlite3_prepare fail: %d\n",nRet );            
            stmt = NULL;
            break;
        }
        nRet = sqlite3_step( stmt );

        if( ( nRet != SQLITE_DONE && nRet != SQLITE_ROW && nRet != SQLITE_OK ) ||
             nRet == SQLITE_DONE 
             )
        {
            //fprintf( "fail2:%d\n", nRet );             
            nRet = sqlite3_finalize(stmt);
            return NULL;		
        }

    } while (0);
    // at least 1 row (SQLITE_ROW)
    return stmt;
}