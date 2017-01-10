#ifndef SQLITE3_UTIL_H_INCLUDED
#define SQLITE3_UTIL_H_INCLUDED

/*
** Make sure we can call this stuff from C++.
*/
#ifdef __cplusplus
extern "C" {
#endif

int  isTableExists(sqlite3*  aHDB,const char* aTableName);
int  table_clear(sqlite3*  aHDB,
    const char* aTableName
    );

sqlite3_stmt* execPrepareStep(sqlite3*  aHDB,  const char* szSQL);

#define SQL_STR_MAX_LEN  512

#ifdef __cplusplus
}  /* End of the 'extern "C"' block */
#endif

#endif //#ifndef SQLITE3_UTIL_H_INCLUDED
 