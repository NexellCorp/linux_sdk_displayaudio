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

#ifndef __NX_UTILS_H__
#define __NX_UTILS_H__

#include <stdint.h>

//	These functions is not supported multiple process.
int32_t NX_RunProcess( const char* pAppName, const char* pAppArgs, int32_t bBackground = true );
int32_t NX_KillProcess( const char* pAppName );

int32_t NX_GetPid( const char *pAppName );
int32_t NX_GetApplicationName( int32_t iPid, char *pAppName );

int32_t NX_IsValidProcess( const char *pAppName, int32_t iPid );

int64_t NX_GetSystemTick();
int32_t NX_GetRandomValue( int32_t iStartNum, int32_t iEndNum );

#endif	// __NX_UTILS_H__
