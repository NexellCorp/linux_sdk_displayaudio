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

#ifndef __NX_DBGMSG_H__
#define __NX_DBGMSG_H__

#include <stdint.h>

#define NX_ENABLE_SYSLOG

#define NX_DBG_VBS			2	// ANDROID_LOG_VERBOSE
#define NX_DBG_DEBUG		3	// ANDROID_LOG_DEBUG
#define	NX_DBG_INFO			4	// ANDROID_LOG_INFO
#define	NX_DBG_WARN			5	// ANDROID_LOG_WARN
#define	NX_DBG_ERR			6	// ANDROID_LOG_ERROR
#define NX_DBG_DISABLE		9

#ifndef NX_DTAG
#define	NX_DTAG		"[]"
#endif

#ifndef NX_DBG_OFF
#ifndef ANDROID

#ifndef NX_ENABLE_SYSLOG
#include <stdio.h>
#define DBG_PRINT			printf
#define NX_Trace(...)		do {										\
								DBG_PRINT(NX_DTAG);						\
								DBG_PRINT(" ");							\
								DBG_PRINT(__VA_ARGS__);					\
							} while(0)

#define NX_DbgMsg(A, ...)	do {										\
								if( gNxDebugLevel <= A ) {				\
									DBG_PRINT(NX_DTAG);					\
									DBG_PRINT(" ");						\
									DBG_PRINT(__VA_ARGS__);				\
								}										\
							} while(0)
#else
#include <stdio.h>
#include <syslog.h>

#define LOG_OPTION			LOG_CONS | LOG_PID | LOG_NDELAY
#define LOG_FACILITY		LOG_USER

#define LOG_STRING(A)		(A == NX_DBG_VBS)	? "/V " : (	\
							(A == NX_DBG_DEBUG)	? "/D " : (	\
							(A == NX_DBG_INFO)	? "/I " : (	\
							(A == NX_DBG_WARN)	? "/W " : (	\
							(A == NX_DBG_ERR)	? "/E " : ""\
							))))

#define LOG_LEVEL(A)		(A == NX_DBG_VBS)	? LOG_DEBUG :   (	\
							(A == NX_DBG_DEBUG)	? LOG_INFO :    (	\
							(A == NX_DBG_INFO)	? LOG_NOTICE :  (	\
							(A == NX_DBG_WARN)	? LOG_WARNING : (	\
							(A == NX_DBG_ERR)	? LOG_ERR :  		\
												  LOG_DEBUG ))))

#define NX_DbgMsg(A, ...)	do {										\
								if( gNxDebugLevel <= A ) {				\
									char szTag[32] = { 0x00, };			\
									snprintf( szTag, sizeof(szTag), "%s%s", NX_DTAG, LOG_STRING(A) ); \
									openlog( szTag, LOG_OPTION, LOG_FACILITY );	\
									syslog( LOG_DEBUG, ##__VA_ARGS__ );	\
									closelog();							\
								}										\
							} while(0)
#endif
#else

#include <android/log.h>
#define DBG_PRINT			__android_log_print
#define NX_Trace(...)		DBG_PRINT(ANDROID_LOG_VERBOSE, NX_DTAG, __VA_ARGS__);

#define NX_DbgMsg(A, ...)	do {										\
								if( gNxDebugLevel <= A ) {				\
									DBG_PRINT(A, NX_DTAG, __VA_ARGS__);	\
								}										\
							} while(0)

#endif
#else

#define NX_Trace(...)
#define NX_DbgMsg(A, ...)

#endif

extern	uint32_t gNxDebugLevel;

void	NX_ChangeDebugLevel( uint32_t iLevel );
void 	NX_DumpHex( const void *pData, int32_t iSize );

#endif	// __NX_DBGMSG_H__
