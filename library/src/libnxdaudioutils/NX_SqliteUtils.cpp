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
#include <unistd.h>
#include <sqlite3.h>

#include "NX_DAudioUtils.h"

//------------------------------------------------------------------------------
int32_t NX_SQLiteGetData( const char *pDatabase, const char *pTable, int32_t (*cbFunc)(void*, int32_t, char**, char**), void *pObj )
{
	int32_t iRet = 0;
	sqlite3 *pSqlite = NULL;

	if( access( pDatabase, F_OK ) )
		return -1;

	iRet = sqlite3_open( pDatabase, &pSqlite );
	if( SQLITE_OK != iRet )
	{
		printf("Fail, sqlite3_open().\n");
		return -1;
	}

	char qeury[256];
	char *pErrorMsg = NULL;
	snprintf( qeury, sizeof(qeury), "SELECT * FROM %s;", pTable );

	iRet = sqlite3_exec( pSqlite, qeury, cbFunc, pObj, &pErrorMsg );
	if( SQLITE_OK != iRet )
	{
		printf("Fail, sqlite3_exec(). ( %s )\n", pErrorMsg );
	}

	if( pSqlite )
	{
		sqlite3_close( pSqlite );
	}

	return (iRet != SQLITE_OK) ? -1 : 0;
}
