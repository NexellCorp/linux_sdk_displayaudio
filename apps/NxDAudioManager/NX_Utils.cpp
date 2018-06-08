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
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>

#include "NX_Utils.h"

//------------------------------------------------------------------------------
static int32_t IsDigit( const char *pStr )
{
	for( int32_t i = 0; pStr[i] != '\0'; i++ )
	{
		if( !isdigit(pStr[i]) )
		{
			return false;
		}
	}
	return true;
}

//------------------------------------------------------------------------------
int32_t NX_RunProcess( const char* pAppName, const char* pAppArgs, int32_t bBackground )
{
	if( NULL == pAppName )
		return -1;

	char* pCmd;
	int32_t iCmdLen = 0;

	iCmdLen += pAppName ? strlen( pAppName )   : 0;
	iCmdLen += pAppArgs ? strlen( pAppArgs )+1 : 0;
	iCmdLen += bBackground ? 2 : 0;
	iCmdLen += 1;

	pCmd = (char*)malloc( sizeof(char) * iCmdLen );

	int32_t iWrite = 0, iWritten = 0;
	char* pPtr = pCmd;
	memset( pPtr, 0x00, iCmdLen );

	if( pAppName != NULL )
	{
		iWrite = sprintf( pPtr,
			"%s%s",
			(iWritten != 0) ? " " : "",
			pAppName
		);
		pPtr     += iWrite;
		iWritten += iWrite;
	}

	if( pAppArgs != NULL )
	{
		iWrite = sprintf( pPtr,
			"%s%s",
			(iWritten != 0) ? " " : "",
			pAppArgs
		);
		pPtr     += iWrite;
		iWritten += iWrite;
	}

	if( bBackground )
	{
		iWrite = sprintf( pPtr,
			"%s%s",
			(iWritten != 0) ? " " : "",
			"&"
		);
		pPtr     += iWrite;
		iWritten += iWrite;
	}

	int32_t iRet = system( pCmd );
	printf("# sh %s ( ret = %d )\n", pCmd, iRet);

	if( pCmd ) free( pCmd );
	return iRet;
}

//------------------------------------------------------------------------------
int32_t NX_KillProcess( const char *pAppName )
{
	int32_t iPid = NX_GetPid( pAppName );
	if( 0 > iPid )
		return -1;

	kill( iPid, SIGKILL );
	return 0;
}

//------------------------------------------------------------------------------
int32_t NX_GetPid( const char *pAppName )
{
	int32_t iPid = -1;

	DIR *pDir;
	struct dirent *pDirent;

	pDir = opendir("/proc");
	if( NULL == pDir )
		return -1;

	while( NULL != (pDirent = readdir(pDir)) )
	{
		if( !IsDigit(pDirent->d_name) )
			continue;

		char path[256], name[256];
		snprintf( path, sizeof(path), "/proc/%s/cmdline", pDirent->d_name );

		FILE *pFile = fopen( path, "r" );
		if( pFile != NULL )
		{
			if( 0 < fscanf( pFile, "%s", name ) )
			{
				if( !strcmp(name, pAppName) )
				{
					iPid = atoi( pDirent->d_name );
					fclose( pFile );
					break;
				}
			}

			fclose( pFile );
		}
	}

	closedir( pDir );
	return iPid;
}

//------------------------------------------------------------------------------
int32_t NX_GetApplicationName( int32_t iPid, char *pAppName )
{
	char path[256];
	snprintf( path, sizeof(path), "/proc/%d/cmdline", iPid );

	FILE *pFile = fopen( path, "r" );
	if( pFile == NULL )
		return -1;

	int32_t iRet = -1;
	iRet = fscanf( pFile, "%s", pAppName );
	fclose( pFile );

	return (iRet > 0) ? 0 : -1;
}

//------------------------------------------------------------------------------
int32_t NX_IsValidProcess( const char *pAppName, int32_t iPid )
{
	char path[256], name[256];
	snprintf( path, sizeof(path), "/proc/%d/cmdline", iPid );

	FILE *pFile = fopen( path, "r" );
	if( pFile == NULL )
		return false;

	int32_t iRet = -1;
	iRet = fscanf( pFile, "%s", name );
	fclose( pFile );

	if( iRet <= 0  || strcmp( pAppName, name) )
		return false;

	return true;
}

//------------------------------------------------------------------------------
int64_t NX_GetSystemTick()
{
	struct timeval	tv;
	struct timezone	zv;

	gettimeofday( &tv, &zv );

	return ((int64_t)tv.tv_sec) * 1000 + (int64_t)(tv.tv_usec / 1000);
}

//------------------------------------------------------------------------------
int32_t NX_GetRandomValue( int32_t iStartNum, int32_t iEndNum )
{
    if( iStartNum >= iEndNum )
        return -1;

    srand( (uint32_t)NX_GetSystemTick() );
    return rand() % (iEndNum - iStartNum + 1) + iStartNum;
}

