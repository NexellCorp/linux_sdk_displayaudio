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
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>

#include "NX_Utils.h"

#define ENABLE_CONSOLE_DUP		0
#define ENABLE_KILL_FORCE		1

//------------------------------------------------------------------------------
static int32_t IsDirectory( char *pFile )
{
	struct stat stStat;
	stat( pFile, &stStat );
	return S_ISDIR( stStat.st_mode ) ? true : false;
}

//------------------------------------------------------------------------------
int64_t NX_GetSystemTick( void )
{
	struct timeval	tv;
	struct timezone	zv;

	gettimeofday( &tv, &zv );

	return ((int64_t)tv.tv_sec) * 1000 + (int64_t)(tv.tv_usec / 1000);
}

//------------------------------------------------------------------------------
int32_t NX_GetConsole( char *pDevice )
{
	if( NULL == pDevice )
		return -1;

	FILE *pDev = fopen( "/proc/cmdline", "rb" );
	if( NULL == pDev )
		return -1;

	char cmdLine[256] = { 0x00, };
	fread( cmdLine, sizeof(cmdLine), 1, pDev );
	sscanf( cmdLine, "%*[^'=']=%[^','],%*[^',']", pDevice );

	fclose( pDev );
	return strlen(pDevice);
}

//------------------------------------------------------------------------------
int32_t NX_RunProcess( const char *pExec )
{
	if( NULL == pExec )
		return -1;

	struct sigaction sigchld_action;
	sigchld_action.sa_handler = SIG_DFL;
	sigchld_action.sa_flags   = SA_NOCLDWAIT;
	sigaction( SIGCHLD, &sigchld_action, NULL );

	int32_t pid = fork();
	if( 0 > pid )
	{
		printf("Fail, fork().\n");
		return -1;
	}
	else if( 0 == pid )
	{
#if ENABLE_CONSOLE_DUP
		char dev[256], tty[256];
		if( 0 > NX_GetConsole( tty ) )
			return -1;

		sprintf( dev, "/dev/%s", tty );
		int32_t fd = open( dev, O_RDWR );
		if( 0 > fd )
			return -1;

		dup2( fd, STDOUT_FILENO );
		dup2( fd, STDERR_ACKGROUND);
		close( fd );
#endif
		if( 0 > execl( pExec, pExec, "-platform", "wayland", NULL ) )
		{
			printf("Fail, execl(). ( %s )\n", pExec);
			return -1;
		}
	}

	return 0;
}

//------------------------------------------------------------------------------
int32_t NX_KillProcess( const char *pExec )
{
	DIR *pDir;
	struct dirent *pDirent;
	const char* pDirectory = "/proc";

	if( NULL == pExec || !strcmp(pExec, "") )
	{
		return -1;
	}

	if( NULL == (pDir = opendir( pDirectory )) )
	{
		return -1;
	}

	while( NULL != (pDirent = readdir(pDir)) )
	{
		if( !strcmp( pDirent->d_name, "." ) ||
			!strcmp( pDirent->d_name, ".." ) )
			continue;

		char szTemp[256];

		memset( szTemp, 0x00, sizeof(szTemp) );
		snprintf( szTemp, sizeof(szTemp), "%s/%s", pDirectory, pDirent->d_name );
		if( !IsDirectory(szTemp) )
			continue;

		memset( szTemp, 0x00, sizeof(szTemp) );
		snprintf( szTemp, sizeof(szTemp), "%s/%s/cmdline", pDirectory, pDirent->d_name );

		FILE *pFile = fopen( szTemp, "r" );
		if( NULL == pFile )
			continue;

		memset( szTemp, 0x00, sizeof(szTemp) );
		fscanf( pFile, "%s", szTemp );
		fclose( pFile );

		if( !strcmp( szTemp, "" ) )
			continue;

		if( NULL != strstr(szTemp, pExec) )
		{
#if ENABLE_KILL_FORCE
			if( 0 > kill( atoi(pDirent->d_name), SIGKILL ) )
#else
			if( 0 > kill( atoi(pDirent->d_name), SIGTERM ) )
#endif
			{
				printf("Fail, kill(). ( exec: %s, pid: %d )\n", szTemp, atoi(pDirent->d_name));
				continue;
			}

			printf("kill process. ( exec: %s, pid: %d )\n", szTemp, atoi(pDirent->d_name));
		}
	}

	return 0;
}

//------------------------------------------------------------------------------
int32_t NX_KillProcess( int32_t iPid )
{
	char szPath[256];
	snprintf( szPath, sizeof(szPath), "/proc/%d", iPid );

	if( 0 > access( szPath, F_OK ) )
	{
		printf( "Fail, Invalid pid. ( %d )\n", iPid );
		return -1;
	}

	char szTemp[256];
	snprintf( szTemp, sizeof(szTemp), "/proc/%d/cmdline", iPid );

	FILE *pFile = fopen( szTemp, "r" );
	if( NULL == pFile )
		return -1;

	fscanf( pFile, "%s", szTemp );
	fclose( pFile );

#if ENABLE_KILL_FORCE
	if( 0 > kill( iPid, SIGKILL ) )
#else
	if( 0 > kill( iPid, SIGTERM ) )
#endif
	{
		printf("Fail, kill(). ( exec: %s, pid: %d )\n", szTemp, iPid );
		return -1;
	}

	printf("kill process. ( exec: %s, pid: %d )\n", szTemp, iPid );
	return 0;
}

//------------------------------------------------------------------------------
int32_t NX_RearCamIsStop( void )
{
	char buf[64] = { 0x00, };
	int32_t bStoped = true;

	int32_t fd = open( "/sys/devices/platform/nx-rearcam/stop", O_RDONLY );
	if( 0 > fd )
		return -1;

	read( fd, buf, sizeof(buf) );
	if( !strncmp( buf, "0", sizeof(buf) ) )
	{
		bStoped = false;
	}

	close( fd );
	return bStoped;
}

//------------------------------------------------------------------------------
int32_t NX_RearCamSetStop( void )
{
	int32_t fd = open( "/sys/devices/platform/nx-rearcam/stop", O_RDWR );
	if( 0 > fd )
		return -1;

	write( fd, "1", 1 );
	close( fd );

	return 0;
}

//------------------------------------------------------------------------------
#include <NX_Type.h>

const char* NX_DAudioUtilGetVersion( void )
{
	return NX_VERSION_LIBDAUDIOUTILS;
}
