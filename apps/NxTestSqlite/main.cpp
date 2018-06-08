//------------------------------------------------------------------------------
//
//	Copyright (C) 2017 Nexell Co. All Rights Reserved
//	Nexell Co. Proprietary & Confidential
//
//	NEXELL INFORMS THAT THIS CODE AND INFORMATION IS PROVIDED "AS IS" BASE
//  AND	WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING
//  BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS
//  FOR A PARTICULAR PURPOSE.
//
//	Module		:
//	File		:
//	Description	:
//	Author		:
//	Export		:
//	History		:
//
//------------------------------------------------------------------------------

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include <sqlite3.h>

//------------------------------------------------------------------------------
static int32_t cbSqliteRowCallback( void *pObj, int32_t iColumnNum, char **ppColumnValue, char **ppColumnName )
{
	for( int32_t i = 0; i < iColumnNum; i++ )
	{
		printf("[ %d ] %s : %s\n", i, ppColumnName[i], ppColumnValue[i] ? ppColumnValue[i] : "NULL");
	}

	return 0;
}

//------------------------------------------------------------------------------
// Query Example
//
// NxTestSqlite test.db "CREATE TABLE tbl_sample(_id INTEGER PRIMARY KEY AUTOINCREMENT, _date DATETIME DEFAULT CURRENT_TIMESTAMP, _comment TEXT NOT NULL)"
// NxTestSqlite test.db "INSERT INTO tbl_sample(_comment) VALUES('sample1')"
//
// NxTestSqlite /home/root/mediainfo.db "INSERT INTO tbl_media (_path, _type) VALUES( '/home/root/mediainfo.db', 'database' );"
// NxTestSqlite /home/root/mediainfo.db "SELECT * FROM tbl_media;"
//

//------------------------------------------------------------------------------
int32_t main(int32_t argc, char **argv)
{
	int32_t iRet = 0;

	sqlite3 *pSqlite = NULL;
	char *pErrorMsg = NULL, *pQuery = NULL;

	if( 3 != argc )
	{
		fprintf(stderr,"Usage: ./NxTestSqlite [dbname] [query]\n");
		exit(0);
	}

	iRet = sqlite3_open( argv[1], &pSqlite );
	if( SQLITE_OK != iRet )
	{
		fprintf( stderr, "Fail, sqlite3_open().\n" );
		exit(0);
	}

	pQuery = argv[2];
	iRet = sqlite3_exec( pSqlite, pQuery, cbSqliteRowCallback, NULL, &pErrorMsg );
	if( SQLITE_OK != iRet )
	{
		fprintf( stderr, "Fail, sqlite3_exec(). ( %s )\n", pErrorMsg );
	}

	if( pSqlite )
	{
		sqlite3_close( pSqlite );
	}

	return 0;
}
