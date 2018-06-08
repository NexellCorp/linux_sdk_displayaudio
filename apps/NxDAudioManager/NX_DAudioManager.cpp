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
#include <unistd.h>
#include <string.h>
#include <signal.h>

#include <NX_DAudioUtils.h>

#include "CNX_Base.h"
#include "CNX_ProcessManager.h"
#include "CNX_UeventManager.h"
#include "CNX_EventManager.h"
#include "CNX_PostProcess.h"
#include "CNX_MediaScanner.h"
#include "CNX_VolumeManager.h"
#include "CNX_CommandManager.h"

#include "NX_Utils.h"
#include "INX_IpcManager.h"
#include "NX_IpcPacket.h"
#include "NX_IpcUtils.h"

#define NX_DTAG	"[NxDAudioManager]"
#include "NX_DbgMsg.h"

#define NX_ENABLE_MEDIA_SCANNER				1
#define NX_ENABLE_RETRY_LAUNCHER			0

//------------------------------------------------------------------------------
typedef struct NX_DAUDIO_MANAGER {
	CNX_ProcessManager*	pProcessManager;
	INX_IpcManager*		pIpcManager;
	CNX_UeventManager*	pUeventManager;
	CNX_VolumeManager*	pVolumeManager;
	CNX_PostProcess*	pPostProcess;
	CNX_MediaScanner*	pMediaScanner;
	CNX_CommandManager*	pCommandManager;
	INX_IpcManager*		pAvmIpcManager;

	uint32_t			iEventType;
} NX_DAUDIO_MANAGER;

static NX_DAUDIO_MANAGER stManager;

//------------------------------------------------------------------------------
static void signal_handler( int32_t signal )
{
	printf("Aborted by signal %s (%d)..\n", (char*)strsignal(signal), signal);

	switch( signal )
	{
		case SIGINT :
			printf("SIGINT..\n"); 	break;
		case SIGTERM :
			printf("SIGTERM..\n");	break;
		case SIGABRT :
			printf("SIGABRT..\n");	break;
		default :
			break;
	}

	if( stManager.pProcessManager )	delete stManager.pProcessManager;
	if( stManager.pIpcManager )		delete stManager.pIpcManager;
	if( stManager.pUeventManager )	delete stManager.pUeventManager;
	if( stManager.pVolumeManager )	delete stManager.pVolumeManager;
	if( stManager.pPostProcess )	delete stManager.pPostProcess;
	if( stManager.pMediaScanner )	delete stManager.pMediaScanner;
	if( stManager.pCommandManager ) delete stManager.pCommandManager;

	exit(EXIT_FAILURE);
}

//------------------------------------------------------------------------------
static void register_signal( void )
{
	signal( SIGINT,  signal_handler );
	signal( SIGTERM, signal_handler );
	signal( SIGABRT, signal_handler );
}

//------------------------------------------------------------------------------
static int32_t cbRequestIsProcessCheck( int32_t iIndex, NX_PROCESS_INFO *pInfo, NX_PROCESS_INFO *pReqInfo, void *pObj )
{
	if( !strcmp( pInfo->szAppName, pReqInfo->szAppName ) )
	{
		NX_DbgMsg( NX_DBG_DEBUG, "The Request Process is Exist in Process List.\n" );
		return -1;
	}

	return 0;
}

//------------------------------------------------------------------------------
static int32_t cbRequestCurrentProcessShow( int32_t iIndex, NX_PROCESS_INFO *pInfo, NX_PROCESS_INFO *pReqInfo, void *pObj )
{
	INX_IpcManager *pIpcManager = ((NX_DAUDIO_MANAGER*)pObj)->pIpcManager;

	uint8_t sendBuf[MAX_PACKET_SIZE], recvBuf[MAX_PACKET_SIZE];
	uint32_t iKey;
	void* pPayload;
	int32_t iPayloadSize;
	int32_t iRet, iSendSize, iRecvSize;

	if( !strcmp(pInfo->szAppName, pReqInfo->szAppName) )
	{
		NX_DbgMsg( NX_DBG_VBS, "%s(): Request Process is \"%s\"\n", __FUNCTION__, pReqInfo->szAppName );

		iSendSize = NX_IpcMakePacket( NX_REQUEST_PROCESS_SHOW, pReqInfo, sizeof(NX_PROCESS_INFO), sendBuf, sizeof(sendBuf) );
		if( 0 > iSendSize )
		{
			NX_DbgMsg( NX_DBG_ERR, "Fail, NX_IpcMakePacket().\n" );
			return -1;
		}

		iRecvSize = pIpcManager->SendCommand( pInfo->szSockName, sendBuf, iSendSize, recvBuf, sizeof(recvBuf) );
		if( 0 > iRecvSize )
		{
			NX_DbgMsg( NX_DBG_ERR, "Fail, SendCommand().\n" );
			return -1;
		}

		iRet = NX_IpcParsePacket( recvBuf, iRecvSize, &iKey, &pPayload, &iPayloadSize );
		if( 0 > iRet )
		{
			NX_DbgMsg( NX_DBG_ERR, "Fail, NX_IpcParsePacket().\n" );
			return -1;
		}

		return (iKey == NX_REPLY_DONE) ? 0 : -1;
	}

	return 0;
}

//------------------------------------------------------------------------------
static int32_t cbRequestCurrentProcessHide( int32_t iIndex, NX_PROCESS_INFO *pInfo, NX_PROCESS_INFO *pReqInfo, void *pObj )
{
	INX_IpcManager *pIpcManager = ((NX_DAUDIO_MANAGER*)pObj)->pIpcManager;

	uint8_t sendBuf[MAX_PACKET_SIZE], recvBuf[MAX_PACKET_SIZE];
	uint32_t iKey;
	void* pPayload;
	int32_t iPayloadSize;
	int32_t iRet, iSendSize, iRecvSize;

	if( !strcmp(pInfo->szAppName, pReqInfo->szAppName) )
	{
		NX_DbgMsg( NX_DBG_VBS, "%s(): Request Process is \"%s\"\n", __FUNCTION__, pReqInfo->szAppName );

		iSendSize = NX_IpcMakePacket( NX_REQUEST_PROCESS_HIDE, pReqInfo, sizeof(NX_PROCESS_INFO), sendBuf, sizeof(sendBuf) );
		if( 0 > iSendSize )
		{
			NX_DbgMsg( NX_DBG_ERR, "Fail, NX_IpcMakePacket().\n" );
			return -1;
		}

		iRecvSize = pIpcManager->SendCommand( pInfo->szSockName, sendBuf, iSendSize, recvBuf, sizeof(recvBuf) );
		if( 0 > iRecvSize )
		{
			NX_DbgMsg( NX_DBG_ERR, "Fail, SendCommand().\n" );
			return -1;
		}

		iRet = NX_IpcParsePacket( recvBuf, iRecvSize, &iKey, &pPayload, &iPayloadSize );
		if( 0 > iRet )
		{
			NX_DbgMsg( NX_DBG_ERR, "Fail, NX_IpcParsePacket().\n" );
			return -1;
		}

		return (iKey == NX_REPLY_DONE) ? 0 : -1;
	}

	return 0;
}

//------------------------------------------------------------------------------
static int32_t cbRequestLauncherShow( int32_t iIndex, NX_PROCESS_INFO *pInfo, NX_PROCESS_INFO *pReqInfo, void *pObj )
{
	INX_IpcManager *pIpcManager = ((NX_DAUDIO_MANAGER*)pObj)->pIpcManager;

	uint8_t sendBuf[MAX_PACKET_SIZE], recvBuf[MAX_PACKET_SIZE];
	uint32_t iKey;
	void* pPayload;
	int32_t iPayloadSize;
	int32_t iRet, iSendSize, iRecvSize;

	if( pInfo->iFlags & NX_PROCESS_FLAG_LAUNCHER )
	{
		NX_DbgMsg( NX_DBG_VBS, "%s(): Request Process is \"%s\"\n", __FUNCTION__, pReqInfo->szAppName );

		iSendSize = NX_IpcMakePacket( NX_REQUEST_PROCESS_SHOW, pReqInfo, sizeof(NX_PROCESS_INFO), sendBuf, sizeof(sendBuf) );
		if( 0 > iSendSize )
		{
			NX_DbgMsg( NX_DBG_ERR, "Fail, NX_IpcMakePacket().\n" );
			return -1;
		}

		iRecvSize = pIpcManager->SendCommand( pInfo->szSockName, sendBuf, iSendSize, recvBuf, sizeof(recvBuf) );
		if( 0 > iRecvSize )
		{
			NX_DbgMsg( NX_DBG_ERR, "Fail, SendCommand().\n" );
			return -1;
		}

		iRet = NX_IpcParsePacket( recvBuf, iRecvSize, &iKey, &pPayload, &iPayloadSize );
		if( 0 > iRet )
		{
			NX_DbgMsg( NX_DBG_ERR, "Fail, NX_IpcParsePacket().\n" );
			return -1;
		}

		return (iKey == NX_REPLY_DONE) ? 0 : -1;
	}

	return 0;
}

#if 0
//------------------------------------------------------------------------------
static int32_t cbRequestAnotherProcessHide( int32_t iIndex, NX_PROCESS_INFO *pInfo, NX_PROCESS_INFO *pReqInfo, void *pObj )
{
	INX_IpcManager *pIpcManager = ((NX_DAUDIO_MANAGER*)pObj)->pIpcManager;

	uint8_t sendBuf[MAX_PACKET_SIZE], recvBuf[MAX_PACKET_SIZE];
	uint32_t iKey;
	void* pPayload;
	int32_t iPayloadSize;
	int32_t iRet, iSendSize, iRecvSize;

	if( strcmp(pInfo->szAppName, pReqInfo->szAppName) )
	{
		NX_DbgMsg( NX_DBG_VBS, "%s(): Request Process is \"%s\"\n", __FUNCTION__, pReqInfo->szAppName );

		iSendSize = NX_IpcMakePacket( NX_REQUEST_PROCESS_HIDE, pReqInfo, sizeof(NX_PROCESS_INFO), sendBuf, sizeof(sendBuf) );
		if( 0 > iSendSize )
		{
			NX_DbgMsg( NX_DBG_ERR, "Fail, NX_IpcMakePacket().\n" );
			return -1;
		}

		iRecvSize = pIpcManager->SendCommand( pInfo->szSockName, sendBuf, iSendSize, recvBuf, sizeof(recvBuf) );
		if( 0 > iRecvSize )
		{
			NX_DbgMsg( NX_DBG_ERR, "Fail, SendCommand().\n" );
			return -1;
		}

		iRet = NX_IpcParsePacket( recvBuf, iRecvSize, &iKey, &pPayload, &iPayloadSize );
		if( 0 > iRet )
		{
			NX_DbgMsg( NX_DBG_ERR, "Fail, NX_IpcParsePacket().\n" );
			return -1;
		}

		return (iKey == NX_REPLY_DONE) ? 0 : -1;
	}

	return 0;
}
#endif

//------------------------------------------------------------------------------
static int32_t cbRequestFinalProcessShow( int32_t iIndex, NX_PROCESS_INFO *pInfo, NX_PROCESS_INFO *pReqInfo, void *pObj )
{
	INX_IpcManager *pIpcManager = ((NX_DAUDIO_MANAGER*)pObj)->pIpcManager;

	uint8_t sendBuf[MAX_PACKET_SIZE], recvBuf[MAX_PACKET_SIZE];
	uint32_t iKey;
	void* pPayload;
	int32_t iPayloadSize;
	int32_t iRet, iSendSize, iRecvSize;

	if( (1 == iIndex) &&
		((pInfo->iDisplayDevice == pReqInfo->iDisplayDevice) ||
		((pInfo->iFlags & NX_PROCESS_FLAG_CAMERA) == (pReqInfo->iFlags & NX_PROCESS_FLAG_CAMERA))) )
	{
		NX_DbgMsg( NX_DBG_VBS, "%s(): Request Process is \"%s\"\n", __FUNCTION__, pReqInfo->szAppName );

		iSendSize = NX_IpcMakePacket( NX_REQUEST_PROCESS_SHOW, pReqInfo, sizeof(NX_PROCESS_INFO), sendBuf, sizeof(sendBuf) );
		if( 0 > iSendSize )
		{
			NX_DbgMsg( NX_DBG_ERR, "Fail, NX_IpcMakePacket().\n" );
			return -1;
		}

		iRecvSize = pIpcManager->SendCommand( pInfo->szSockName, sendBuf, iSendSize, recvBuf, sizeof(recvBuf) );
		if( 0 > iRecvSize )
		{
			NX_DbgMsg( NX_DBG_ERR, "Fail, SendCommand().\n" );
			return -1;
		}

		iRet = NX_IpcParsePacket( recvBuf, iRecvSize, &iKey, &pPayload, &iPayloadSize );
		if( 0 > iRet )
		{
			NX_DbgMsg( NX_DBG_ERR, "Fail, NX_IpcParsePacket().\n" );
			return -1;
		}

		return (iKey == NX_REPLY_DONE) ? 0 : -1;
	}

	return 0;
}

//------------------------------------------------------------------------------
static int32_t cbRequestFinalProcessHide( int32_t iIndex, NX_PROCESS_INFO *pInfo, NX_PROCESS_INFO *pReqInfo, void *pObj )
{
	INX_IpcManager *pIpcManager = ((NX_DAUDIO_MANAGER*)pObj)->pIpcManager;

	uint8_t sendBuf[MAX_PACKET_SIZE], recvBuf[MAX_PACKET_SIZE];
	uint32_t iKey;
	void* pPayload;
	int32_t iPayloadSize;
	int32_t iRet, iSendSize, iRecvSize;

	if( (0 == iIndex) &&
		((pInfo->iDisplayDevice == pReqInfo->iDisplayDevice) ||
		((pInfo->iFlags & NX_PROCESS_FLAG_CAMERA) == (pReqInfo->iFlags & NX_PROCESS_FLAG_CAMERA))) )
	{
		NX_DbgMsg( NX_DBG_VBS, "%s(): Request Process is \"%s\"\n", __FUNCTION__, pReqInfo->szAppName );

		iSendSize = NX_IpcMakePacket( NX_REQUEST_PROCESS_HIDE, pReqInfo, sizeof(NX_PROCESS_INFO), sendBuf, sizeof(sendBuf) );
		if( 0 > iSendSize )
		{
			NX_DbgMsg( NX_DBG_ERR, "Fail, NX_IpcMakePacket().\n" );
			return -1;
		}

		iRecvSize = pIpcManager->SendCommand( pInfo->szSockName, sendBuf, iSendSize, recvBuf, sizeof(recvBuf) );
		if( 0 > iRecvSize )
		{
			NX_DbgMsg( NX_DBG_ERR, "Fail, SendCommand().\n" );
			return -1;
		}

		iRet = NX_IpcParsePacket( recvBuf, iRecvSize, &iKey, &pPayload, &iPayloadSize );
		if( 0 > iRet )
		{
			NX_DbgMsg( NX_DBG_ERR, "Fail, NX_IpcParsePacket().\n" );
			return -1;
		}

		return (iKey == NX_REPLY_DONE) ? 0 : -1;
	}

	return 0;
}

//------------------------------------------------------------------------------
static int32_t cbRequestVideoFocus( int32_t iIndex, NX_PROCESS_INFO *pInfo, NX_PROCESS_INFO *pReqInfo, void *pObj )
{
	INX_IpcManager *pIpcManager = ((NX_DAUDIO_MANAGER*)pObj)->pIpcManager;

	uint8_t sendBuf[MAX_PACKET_SIZE], recvBuf[MAX_PACKET_SIZE];
	uint32_t iKey;
	void* pPayload;
	int32_t iPayloadSize;
	int32_t iRet, iSendSize, iRecvSize;

	if( strcmp( pInfo->szAppName, pReqInfo->szAppName ) && (pInfo->iDisplayDevice == pReqInfo->iDisplayDevice) )
	{
		NX_DbgMsg( NX_DBG_VBS, "%s(): Request Process is \"%s\"\n", __FUNCTION__, pReqInfo->szAppName );

		iSendSize = NX_IpcMakePacket( NX_REQUEST_FOCUS_VIDEO, pReqInfo, sizeof(NX_PROCESS_INFO), sendBuf, sizeof(sendBuf) );
		if( 0 > iSendSize )
		{
			NX_DbgMsg( NX_DBG_ERR, "Fail, NX_IpcMakePacket().\n" );
			return -1;
		}

		iRecvSize = pIpcManager->SendCommand( pInfo->szSockName, sendBuf, iSendSize, recvBuf, sizeof(recvBuf) );
		if( 0 > iRecvSize )
		{
			NX_DbgMsg( NX_DBG_ERR, "Fail, SendCommand().\n" );
			return -1;
		}

		iRet = NX_IpcParsePacket( recvBuf, iRecvSize, &iKey, &pPayload, &iPayloadSize );
		if( 0 > iRet )
		{
			NX_DbgMsg( NX_DBG_ERR, "Fail, NX_IpcParsePacket().\n" );
			return -1;
		}

		return (iKey == NX_REPLY_DONE) ? 0 : -1;
	}

	return 0;
}

//------------------------------------------------------------------------------
static int32_t cbRequestAudioFocus( int32_t iIndex, NX_PROCESS_INFO *pInfo, NX_PROCESS_INFO *pReqInfo, void *pObj )
{
	INX_IpcManager *pIpcManager = ((NX_DAUDIO_MANAGER*)pObj)->pIpcManager;

	uint8_t sendBuf[MAX_PACKET_SIZE], recvBuf[MAX_PACKET_SIZE];
	uint32_t iKey;
	void* pPayload;
	int32_t iPayloadSize;
	int32_t iRet, iSendSize, iRecvSize;

	if( strcmp( pInfo->szAppName, pReqInfo->szAppName ) )
	{
		NX_DbgMsg( NX_DBG_VBS, "%s(): Request Process is \"%s\"\n", __FUNCTION__, pReqInfo->szAppName );

		iSendSize = NX_IpcMakePacket( NX_REQUEST_FOCUS_AUDIO, pReqInfo, sizeof(NX_PROCESS_INFO), sendBuf, sizeof(sendBuf) );
		if( 0 > iSendSize )
		{
			NX_DbgMsg( NX_DBG_ERR, "Fail, NX_IpcMakePacket().\n" );
			return -1;
		}

		iRecvSize = pIpcManager->SendCommand( pInfo->szSockName, sendBuf, iSendSize, recvBuf, sizeof(recvBuf) );
		if( 0 > iRecvSize )
		{
			NX_DbgMsg( NX_DBG_ERR, "Fail, SendCommand().\n" );
			return -1;
		}

		iRet = NX_IpcParsePacket( recvBuf, iRecvSize, &iKey, &pPayload, &iPayloadSize );
		if( 0 > iRet )
		{
			NX_DbgMsg( NX_DBG_ERR, "Fail, NX_IpcParsePacket().\n" );
			return -1;
		}

		return (iKey == NX_REPLY_DONE) ? 0 : -1;
	}

	return 0;
}

//------------------------------------------------------------------------------
static int32_t cbRequestVideoFocusTransient( int32_t iIndex, NX_PROCESS_INFO *pInfo, NX_PROCESS_INFO *pReqInfo, void *pObj )
{
	INX_IpcManager *pIpcManager = ((NX_DAUDIO_MANAGER*)pObj)->pIpcManager;

	uint8_t sendBuf[MAX_PACKET_SIZE], recvBuf[MAX_PACKET_SIZE];
	uint32_t iKey;
	void* pPayload;
	int32_t iPayloadSize;
	int32_t iRet, iSendSize, iRecvSize;

	if( strcmp( pInfo->szAppName, pReqInfo->szAppName ) && (pInfo->iDisplayDevice == pReqInfo->iDisplayDevice) )
	{
		NX_DbgMsg( NX_DBG_VBS, "%s(): Request Process is \"%s\"\n", __FUNCTION__, pReqInfo->szAppName );

		iSendSize = NX_IpcMakePacket( NX_REQUEST_FOCUS_VIDEO_TRANSIENT, pReqInfo, sizeof(NX_PROCESS_INFO), sendBuf, sizeof(sendBuf) );
		if( 0 > iSendSize )
		{
			NX_DbgMsg( NX_DBG_ERR, "Fail, NX_IpcMakePacket().\n" );
			return -1;
		}

		iRecvSize = pIpcManager->SendCommand( pInfo->szSockName, sendBuf, iSendSize, recvBuf, sizeof(recvBuf) );
		if( 0 > iRecvSize )
		{
			NX_DbgMsg( NX_DBG_ERR, "Fail, SendCommand().\n" );
			return -1;
		}

		iRet = NX_IpcParsePacket( recvBuf, iRecvSize, &iKey, &pPayload, &iPayloadSize );
		if( 0 > iRet )
		{
			NX_DbgMsg( NX_DBG_ERR, "Fail, NX_IpcParsePacket().\n" );
			return -1;
		}

		return (iKey == NX_REPLY_DONE) ? 0 : -1;
	}

	return 0;
}

//------------------------------------------------------------------------------
static int32_t cbRequestAudioFocusTransient( int32_t iIndex, NX_PROCESS_INFO *pInfo, NX_PROCESS_INFO *pReqInfo, void *pObj )
{
	INX_IpcManager *pIpcManager = ((NX_DAUDIO_MANAGER*)pObj)->pIpcManager;

	uint8_t sendBuf[MAX_PACKET_SIZE], recvBuf[MAX_PACKET_SIZE];
	uint32_t iKey;
	void* pPayload;
	int32_t iPayloadSize;
	int32_t iRet, iSendSize, iRecvSize;

	if( strcmp( pInfo->szAppName, pReqInfo->szAppName ) )
	{
		NX_DbgMsg( NX_DBG_VBS, "%s(): Request Process is \"%s\"\n", __FUNCTION__, pReqInfo->szAppName );

		iSendSize = NX_IpcMakePacket( NX_REQUEST_FOCUS_AUDIO_TRANSIENT, pReqInfo, sizeof(NX_PROCESS_INFO), sendBuf, sizeof(sendBuf) );
		if( 0 > iSendSize )
		{
			NX_DbgMsg( NX_DBG_ERR, "Fail, NX_IpcMakePacket().\n" );
			return -1;
		}

		iRecvSize = pIpcManager->SendCommand( pInfo->szSockName, sendBuf, iSendSize, recvBuf, sizeof(recvBuf) );
		if( 0 > iRecvSize )
		{
			NX_DbgMsg( NX_DBG_ERR, "Fail, SendCommand().\n" );
			return -1;
		}

		iRet = NX_IpcParsePacket( recvBuf, iRecvSize, &iKey, &pPayload, &iPayloadSize );
		if( 0 > iRet )
		{
			NX_DbgMsg( NX_DBG_ERR, "Fail, NX_IpcParsePacket().\n" );
			return -1;
		}

		return (iKey == NX_REPLY_DONE) ? 0 : -1;
	}

	return 0;
}

//------------------------------------------------------------------------------
static int32_t cbRequestVideoFocusLoss( int32_t iIndex, NX_PROCESS_INFO *pInfo, NX_PROCESS_INFO *pReqInfo, void *pObj )
{
	INX_IpcManager *pIpcManager = ((NX_DAUDIO_MANAGER*)pObj)->pIpcManager;

	uint8_t sendBuf[MAX_PACKET_SIZE], recvBuf[MAX_PACKET_SIZE];
	uint32_t iKey;
	void* pPayload;
	int32_t iPayloadSize;
	int32_t iRet, iSendSize, iRecvSize;

	if( strcmp( pInfo->szAppName, pReqInfo->szAppName ) && (pInfo->iDisplayDevice == pReqInfo->iDisplayDevice) )
	{
		NX_DbgMsg( NX_DBG_VBS, "%s(): Request Process is \"%s\"\n", __FUNCTION__, pReqInfo->szAppName );

		iSendSize = NX_IpcMakePacket( NX_REQUEST_FOCUS_VIDEO_LOSS, NULL, 0, sendBuf, sizeof(sendBuf) );
		if( 0 > iSendSize )
		{
			NX_DbgMsg( NX_DBG_ERR, "Fail, NX_IpcMakePacket().\n" );
			return -1;
		}

		iRecvSize = pIpcManager->SendCommand( pInfo->szSockName, sendBuf, iSendSize, recvBuf, sizeof(recvBuf) );
		if( 0 > iRecvSize )
		{
			NX_DbgMsg( NX_DBG_ERR, "Fail, SendCommand().\n" );
			return -1;
		}

		iRet = NX_IpcParsePacket( recvBuf, iRecvSize, &iKey, &pPayload, &iPayloadSize );
		if( 0 > iRet )
		{
			NX_DbgMsg( NX_DBG_ERR, "Fail, NX_IpcParsePacket().\n" );
			return -1;
		}

		return (iKey == NX_REPLY_DONE) ? 0 : -1;
	}

	return 0;
}

//------------------------------------------------------------------------------
static int32_t cbRequestAudioFocusLoss( int32_t iIndex, NX_PROCESS_INFO *pInfo, NX_PROCESS_INFO *pReqInfo, void *pObj )
{
	INX_IpcManager *pIpcManager = ((NX_DAUDIO_MANAGER*)pObj)->pIpcManager;

	uint8_t sendBuf[MAX_PACKET_SIZE], recvBuf[MAX_PACKET_SIZE];
	uint32_t iKey;
	void* pPayload;
	int32_t iPayloadSize;
	int32_t iRet, iSendSize, iRecvSize;

	if( strcmp( pInfo->szAppName, pReqInfo->szAppName ) )
	{
		NX_DbgMsg( NX_DBG_VBS, "%s(): Request Process is \"%s\"\n", __FUNCTION__, pReqInfo->szAppName );

		iSendSize = NX_IpcMakePacket( NX_REQUEST_FOCUS_AUDIO_LOSS, NULL, 0, sendBuf, sizeof(sendBuf) );
		if( 0 > iSendSize )
		{
			NX_DbgMsg( NX_DBG_ERR, "Fail, NX_IpcMakePacket().\n" );
			return -1;
		}

		iRecvSize = pIpcManager->SendCommand( pInfo->szSockName, sendBuf, iSendSize, recvBuf, sizeof(recvBuf) );
		if( 0 > iRecvSize )
		{
			NX_DbgMsg( NX_DBG_ERR, "Fail, SendCommand().\n" );
			return -1;
		}

		iRet = NX_IpcParsePacket( recvBuf, iRecvSize, &iKey, &pPayload, &iPayloadSize );
		if( 0 > iRet )
		{
			NX_DbgMsg( NX_DBG_ERR, "Fail, NX_IpcParsePacket().\n" );
			return -1;
		}

		return (iKey == NX_REPLY_DONE) ? 0 : -1;
	}

	return 0;
}

//------------------------------------------------------------------------------
static int32_t cbReqeustVolumeEvent( int32_t iIndex, NX_PROCESS_INFO *pInfo, void *pPayload, int32_t iPayloadSize, void *pObj )
{
	INX_IpcManager *pIpcManager = ((NX_DAUDIO_MANAGER*)pObj)->pIpcManager;
	uint32_t iEventType = ((NX_DAUDIO_MANAGER*)pObj)->iEventType;

	uint8_t sendBuf[MAX_PACKET_SIZE], recvBuf[MAX_PACKET_SIZE];
	uint32_t iKey;
	void* pResult;
	int32_t iResultSize;
	int32_t iRet, iSendSize, iRecvSize;

	iSendSize = NX_IpcMakePacket( iEventType, pPayload, iPayloadSize, sendBuf, sizeof(sendBuf) );
	if( 0 > iSendSize )
	{
		NX_DbgMsg( NX_DBG_ERR, "Fail, NX_IpcMakePacket().\n" );
		return -1;
	}

	iRecvSize = pIpcManager->SendCommand( pInfo->szSockName, sendBuf, iSendSize, recvBuf, sizeof(recvBuf) );
	if( 0 > iRecvSize )
	{
		NX_DbgMsg( NX_DBG_ERR, "Fail, SendCommand().\n" );
		return -1;
	}

	iRet = NX_IpcParsePacket( recvBuf, iRecvSize, &iKey, &pResult, &iResultSize );
	if( 0 > iRet )
	{
		NX_DbgMsg( NX_DBG_ERR, "Fail, NX_IpcParsePacket().\n" );
		return -1;
	}

	return (iKey == NX_REPLY_DONE) ? 0 : -1;
}

#if NX_ENABLE_MEDIA_SCANNER
//------------------------------------------------------------------------------
static int32_t cbRequestMediaScanDone( int32_t iIndex, NX_PROCESS_INFO *pInfo, void *pPayload, int32_t iPayloadSize, void *pObj )
{
	INX_IpcManager *pIpcManager = ((NX_DAUDIO_MANAGER*)pObj)->pIpcManager;

	uint8_t sendBuf[MAX_PACKET_SIZE], recvBuf[MAX_PACKET_SIZE];
	uint32_t iKey;
	void* pResult;
	int32_t iResultSize;
	int32_t iRet, iSendSize, iRecvSize;

	iSendSize = NX_IpcMakePacket( NX_EVENT_MEDIA_SCAN_DONE, pPayload, iPayloadSize, sendBuf, sizeof(sendBuf) );
	if( 0 > iSendSize )
	{
		NX_DbgMsg( NX_DBG_ERR, "Fail, NX_IpcMakePacket().\n" );
		return -1;
	}

	iRecvSize = pIpcManager->SendCommand( pInfo->szSockName, sendBuf, iSendSize, recvBuf, sizeof(recvBuf) );
	if( 0 > iRecvSize )
	{
		NX_DbgMsg( NX_DBG_ERR, "Fail, SendCommand().\n" );
		return -1;
	}

	iRet = NX_IpcParsePacket( recvBuf, iRecvSize, &iKey, &pResult, &iResultSize );
	if( 0 > iRet )
	{
		NX_DbgMsg( NX_DBG_ERR, "Fail, NX_IpcParsePacket().\n" );
		return -1;
	}

	return (iKey == NX_REPLY_DONE) ? 0 : -1;
}
#endif

#if NX_ENABLE_MEDIA_SCANNER
//------------------------------------------------------------------------------
static void cbMediaScanDone( void *pObj )
{
	NX_DbgMsg( NX_DBG_INFO, "Media Scan Done.\n" );
	CNX_ProcessManager*	pProcessManager = ((NX_DAUDIO_MANAGER*)pObj)->pProcessManager;
	pProcessManager->Request( cbRequestMediaScanDone, NULL, 0, pObj );
}
#endif

//------------------------------------------------------------------------------
static void cbMediaScanEvent( void *pObj )
{
	CNX_VolumeManager*	pVolumeManager  = ((NX_DAUDIO_MANAGER*)pObj)->pVolumeManager;

#if NX_ENABLE_MEDIA_SCANNER
	CNX_MediaScanner* pMediaScanner = ((NX_DAUDIO_MANAGER*)pObj)->pMediaScanner;

	NX_VOLUME_INFO *pVolumeInfo = NULL;
	int32_t iVolumeNum = 0;

	pVolumeManager->GetMount( &pVolumeInfo, &iVolumeNum );

	char *pDirectory[iVolumeNum];
	for( int32_t i = 0; i < iVolumeNum; i++ )
	{
		pDirectory[i] = (char*)malloc( sizeof(char) * 256 );
		sprintf( pDirectory[i], "%s", pVolumeInfo[i].szVolume );
	}

	pMediaScanner->Scan( pDirectory, iVolumeNum, cbMediaScanDone, &stManager );

	for( int32_t i = 0; i < iVolumeNum; i++ )
	{
		free( pDirectory[i] );
	}
#endif
}

//------------------------------------------------------------------------------
static int32_t cbRequestEventBroadcast( int32_t iIndex, NX_PROCESS_INFO *pInfo, void *pPayload, int32_t iPayloadSize, void *pObj )
{
	INX_IpcManager *pIpcManager = ((NX_DAUDIO_MANAGER*)pObj)->pIpcManager;

	uint8_t sendBuf[MAX_PACKET_SIZE], recvBuf[MAX_PACKET_SIZE];
	uint32_t iKey;
	void* pResult;
	int32_t iResultSize;
	int32_t iRet, iSendSize, iRecvSize;

	iSendSize = NX_IpcMakePacket( NX_EVENT_BROADCAST, pPayload, iPayloadSize, sendBuf, sizeof(sendBuf) );
	if( 0 > iSendSize )
	{
		NX_DbgMsg( NX_DBG_ERR, "Fail, NX_IpcMakePacket().\n" );
		return -1;
	}

	iRecvSize = pIpcManager->SendCommand( pInfo->szSockName, sendBuf, iSendSize, recvBuf, sizeof(recvBuf) );
	if( 0 > iRecvSize )
	{
		NX_DbgMsg( NX_DBG_ERR, "Fail, SendCommand().\n" );
		return -1;
	}

	iRet = NX_IpcParsePacket( recvBuf, iRecvSize, &iKey, &pResult, &iResultSize );
	if( 0 > iRet )
	{
		NX_DbgMsg( NX_DBG_ERR, "Fail, NX_IpcParsePacket().\n" );
		return -1;
	}

	return (iKey == NX_REPLY_DONE) ? 0 : -1;
}

//------------------------------------------------------------------------------
static int32_t cbRequestProcessInfo( NX_PROCESS_INFO *pReqInfo, void *pPayload, int32_t iPayloadSize, void *pObj )
{
	INX_IpcManager *pIpcManager = ((NX_DAUDIO_MANAGER*)pObj)->pIpcManager;

	uint8_t sendBuf[MAX_PACKET_SIZE], recvBuf[MAX_PACKET_SIZE];
	uint32_t iKey;
	void* pResult;
	int32_t iResultSize;
	int32_t iRet, iSendSize, iRecvSize;

	NX_DbgMsg( NX_DBG_VBS, "%s(): Request Process is \"%s\"\n", __FUNCTION__, pReqInfo->szAppName );

	iSendSize = NX_IpcMakePacket( NX_REQUEST_PROCESS_INFO, pPayload, iPayloadSize, sendBuf, sizeof(sendBuf) );
	if( 0 > iSendSize )
	{
		NX_DbgMsg( NX_DBG_ERR, "Fail, NX_IpcMakePacket().\n" );
		return -1;
	}

	iRecvSize = pIpcManager->SendCommand( pReqInfo->szSockName, sendBuf, iSendSize, recvBuf, sizeof(recvBuf) );
	if( 0 > iRecvSize )
	{
		NX_DbgMsg( NX_DBG_ERR, "Fail, SendCommand().\n" );
		return -1;
	}

	iRet = NX_IpcParsePacket( recvBuf, iRecvSize, &iKey, &pResult, &iResultSize );
	if( 0 > iRet )
	{
		NX_DbgMsg( NX_DBG_ERR, "Fail, NX_IpcParsePacket().\n" );
		return -1;
	}

	return (iKey == NX_REPLY_DONE) ? 0 : -1;
}

//------------------------------------------------------------------------------
static int32_t cbServerData( int32_t iSock, uint8_t *pSendBuf, uint8_t *pRecvBuf, int32_t iMaxBufSize, void *pObj )
{
	CNX_ProcessManager *pProcessManager = ((NX_DAUDIO_MANAGER*)pObj)->pProcessManager;
	INX_IpcManager *pIpcManager = ((NX_DAUDIO_MANAGER*)pObj)->pIpcManager;

	int32_t iRet, iReply = NX_REPLY_DONE;
	uint32_t iKey;
	void* pPayload;
	int32_t iPayloadSize;
	int32_t iSendSize = 0, iRecvSize = 0;

	iRecvSize = pIpcManager->Read( iSock, pRecvBuf, iMaxBufSize );
	if( 0 > iRecvSize )
	{
		NX_DbgMsg( NX_DBG_ERR, "Fail, Read().\n" );
		return -1;
	}

	NX_IpcParsePacket( pRecvBuf, iRecvSize, &iKey, &pPayload, &iPayloadSize );

	switch( iKey )
	{
	case NX_REQUEST_PROCESS_ADD:
	{
		NX_DbgMsg( NX_DBG_VBS, "NX_REQUEST_PROCESS_ADD.\n");
		NX_DbgMsg( NX_DBG_VBS, ">>>>> Request Process: %s\n", ((NX_PROCESS_INFO*)pPayload)->szAppName );

		int32_t iCount = 0;

		// -1: Requested Process is exist in process list.
		//  0: Requested Process is not exist in process list.
		NX_DbgMsg( NX_DBG_VBS, "#%d Process Check.\n", iCount++ );
		iRet = pProcessManager->Request( cbRequestIsProcessCheck, (NX_PROCESS_INFO*)pPayload, pObj );
		if( 0 > iRet ) {
			iReply = NX_REPLY_FAIL;
			break;
		}

		NX_DbgMsg( NX_DBG_VBS, "#%d Process Add.\n", iCount++ );
		pProcessManager->Add( (NX_PROCESS_INFO*)pPayload );

		if( ((NX_PROCESS_INFO*)pPayload)->iFlags & NX_PROCESS_FLAG_BACKGROUND )
		{
			int32_t iIndex = pProcessManager->IndexOf( (NX_PROCESS_INFO*)pPayload );
			if( 0 <= iIndex )
			{
				for( int32_t i = iIndex; i < pProcessManager->GetCount() - 1; i++ )
				{
					pProcessManager->Swap( i, i + 1 );
				}
			}
		}
		pProcessManager->PrintInfo();
		break;
	}

	case NX_REQUEST_PROCESS_REMOVE:
	case NX_REQUEST_FOCUS_VIDEO_LOSS | NX_REQUEST_PROCESS_REMOVE:
	case NX_REQUEST_FOCUS_AUDIO_LOSS | NX_REQUEST_PROCESS_REMOVE:
	case NX_REQUEST_FOCUS_AV_LOSS | NX_REQUEST_PROCESS_REMOVE:
	{
		NX_DbgMsg( NX_DBG_VBS, "NX_REQUEST_PROCESS_REMOVE.\n");
		NX_DbgMsg( NX_DBG_VBS, ">>>>> Request Process: %s\n", ((NX_PROCESS_INFO*)pPayload)->szAppName );

		NX_PROCESS_INFO *pReqInfo = NULL;
		int32_t iReqIndex = pProcessManager->IndexOf( (NX_PROCESS_INFO*)pPayload );
		if( 0 > pProcessManager->GetAt( iReqIndex, &pReqInfo ) )
		{
			NX_DbgMsg( NX_DBG_ERR, "Fail, Request Process is not valid.\n" );
			iReply = NX_REPLY_FAIL;
			break;
		}

		int32_t iCmd = NX_REQUEST_FOCUS_MASK & iKey;
		int32_t iCount = 0;

		NX_DbgMsg( NX_DBG_VBS, "#%d Request Final Process Show.\n", iCount++ );
		pProcessManager->Request( cbRequestFinalProcessShow, (NX_PROCESS_INFO*)pReqInfo, pObj );

		NX_DbgMsg( NX_DBG_VBS, "#%d Process Remove.\n", iCount++ );
		pProcessManager->Remove( (NX_PROCESS_INFO*)pReqInfo );
		pProcessManager->PrintInfo();

		if( iCmd & NX_REQUEST_FOCUS_VIDEO_LOSS )
		{
			NX_DbgMsg( NX_DBG_VBS, "#%d Request Video Focus Loss.\n", iCount++ );
			pProcessManager->Request( cbRequestVideoFocusLoss, (NX_PROCESS_INFO*)pReqInfo, pObj );
		}

		if( iCmd & NX_REQUEST_FOCUS_AUDIO_LOSS )
		{
			NX_DbgMsg( NX_DBG_VBS, "#%d Request Audio Focus Loss.\n", iCount++ );
			pProcessManager->Request( cbRequestAudioFocusLoss, (NX_PROCESS_INFO*)pReqInfo, pObj );
		}
		break;
	}

	case NX_REQUEST_PROCESS_CHECK:
	{
		NX_DbgMsg( NX_DBG_VBS, "NX_REQUEST_PROCESS_CHECK.\n");
		NX_DbgMsg( NX_DBG_VBS, ">>>>> Request Process: %s\n", ((NX_PROCESS_INFO*)pPayload)->szAppName );

		// -1: Requested Process is exist in process list.
		//  0: Requested Process is not exist in process list.
		iRet = pProcessManager->Request( cbRequestIsProcessCheck, (NX_PROCESS_INFO*)pPayload, pObj );
		if( 0 > iRet ) iReply = NX_REPLY_FAIL;
		break;
	}

	case NX_REQUEST_FOCUS_AV:
	case NX_REQUEST_FOCUS_VIDEO:
	case NX_REQUEST_FOCUS_AUDIO:
	case NX_REQUEST_FOCUS_AV_TRANSIENT:
	case NX_REQUEST_FOCUS_VIDEO_TRANSIENT:
	case NX_REQUEST_FOCUS_AUDIO_TRANSIENT:
	{
		NX_DbgMsg( NX_DBG_VBS, "NX_REQUEST_FOCUS.\n");
		NX_DbgMsg( NX_DBG_VBS, ">>>>> Request Process: %s\n", ((NX_PROCESS_INFO*)pPayload)->szAppName );

		NX_PROCESS_INFO *pReqInfo = NULL;
		int32_t iReqIndex = pProcessManager->IndexOf( (NX_PROCESS_INFO*)pPayload );
		if( 0 > pProcessManager->GetAt( iReqIndex, &pReqInfo ) )
		{
			NX_DbgMsg( NX_DBG_ERR, "Fail, Request Process is not valid.\n" );
			iReply = NX_REPLY_FAIL;
			break;
		}

		int32_t iCmd = NX_REQUEST_FOCUS_MASK & iKey;
		int32_t iCount = 0;

		if( iCmd & NX_REQUEST_FOCUS_VIDEO )
		{
			NX_DbgMsg( NX_DBG_VBS, "#%d Reqeust Video Focus.\n", iCount++ );
			iRet = pProcessManager->Request( cbRequestVideoFocus, pReqInfo, pObj );
			if( 0 > iRet ) {
				NX_DbgMsg( NX_DBG_VBS, "Don't Get Video Focus.\n" );
				iReply = NX_REPLY_FAIL;
				break;
			}
		}

		if( iCmd & NX_REQUEST_FOCUS_AUDIO )
		{
			NX_DbgMsg( NX_DBG_VBS, "#%d Reqeust Audio Focus.\n", iCount++ );
			iRet = pProcessManager->Request( cbRequestAudioFocus, pReqInfo, pObj );
			if( 0 > iRet ) {
				NX_DbgMsg( NX_DBG_VBS, "Don't Get Audio Focus.\n" );
				iReply = NX_REPLY_FAIL;
				break;
			}
		}

		if( iCmd & NX_REQUEST_FOCUS_VIDEO_TRANSIENT )
		{
			NX_DbgMsg( NX_DBG_VBS, "#%d Request Video Focus Transient.\n", iCount++ );
			iRet = pProcessManager->Request( cbRequestVideoFocusTransient, pReqInfo, pObj );
			if( 0 > iRet ) {
				NX_DbgMsg( NX_DBG_VBS, "Don't Get Video Focus Transient.\n" );
				iReply = NX_REPLY_FAIL;
				break;
			}
		}

		if( iCmd & NX_REQUEST_FOCUS_AUDIO_TRANSIENT )
		{
			NX_DbgMsg( NX_DBG_VBS, "#%d Request Audio Focus Transient.\n", iCount++ );
			iRet = pProcessManager->Request( cbRequestAudioFocusTransient, pReqInfo, pObj );
			if( 0 > iRet ) {
				NX_DbgMsg( NX_DBG_VBS, "Don't Get Audio Focus Transient.\n" );
				iReply = NX_REPLY_FAIL;
				break;
			}
		}
		break;
	}

	case NX_REQUEST_FOCUS_VIDEO_LOSS:
	case NX_REQUEST_FOCUS_AUDIO_LOSS:
	case NX_REQUEST_FOCUS_AV_LOSS:
	{
		NX_DbgMsg( NX_DBG_VBS, "NX_REQUEST_FOCUS_LOSS.\n");
		NX_DbgMsg( NX_DBG_VBS, ">>>>> Request Process: %s\n", ((NX_PROCESS_INFO*)pPayload)->szAppName );

		NX_PROCESS_INFO *pReqInfo = NULL;
		int32_t iReqIndex = pProcessManager->IndexOf( (NX_PROCESS_INFO*)pPayload );
		if( 0 > pProcessManager->GetAt( iReqIndex, &pReqInfo ) )
		{
			NX_DbgMsg( NX_DBG_ERR, "Fail, Request Process is not valid.\n" );
			iReply = NX_REPLY_FAIL;
			break;
		}

		int32_t iCmd = NX_REQUEST_FOCUS_MASK & iKey;
		int32_t iCount = 0;

		if( iCmd & NX_REQUEST_FOCUS_VIDEO_LOSS )
		{
			NX_DbgMsg( NX_DBG_VBS, "#%d Request Video Focus Loss.\n", iCount++);
			pProcessManager->Request( cbRequestVideoFocusLoss, pReqInfo, pObj );
		}

		if( iCmd & NX_REQUEST_FOCUS_AUDIO_LOSS )
		{
			NX_DbgMsg( NX_DBG_VBS, "#%d Request Audio Focus Loss.\n", iCount++);
			pProcessManager->Request( cbRequestAudioFocusLoss, pReqInfo, pObj );
		}
		break;
	}

	case NX_REQUEST_PROCESS_SHOW:
	case NX_REQUEST_FOCUS_VIDEO | NX_REQUEST_PROCESS_SHOW:
	case NX_REQUEST_FOCUS_AUDIO	| NX_REQUEST_PROCESS_SHOW:
	case NX_REQUEST_FOCUS_AV | NX_REQUEST_PROCESS_SHOW:
	case NX_REQUEST_FOCUS_VIDEO_TRANSIENT | NX_REQUEST_PROCESS_SHOW:
	case NX_REQUEST_FOCUS_AUDIO_TRANSIENT | NX_REQUEST_PROCESS_SHOW:
	case NX_REQUEST_FOCUS_AV_TRANSIENT | NX_REQUEST_PROCESS_SHOW:
	{
		NX_DbgMsg( NX_DBG_VBS, "NX_REQUEST_PROCESS_SHOW.\n");
		NX_DbgMsg( NX_DBG_VBS, ">>>>> Request Process: %s\n", ((NX_PROCESS_INFO*)pPayload)->szAppName );

		NX_PROCESS_INFO *pReqInfo = NULL;
		int32_t iReqIndex = pProcessManager->IndexOf( (NX_PROCESS_INFO*)pPayload );
		if( 0 == iReqIndex )
		{
			NX_DbgMsg( NX_DBG_WARN, "Fail, Already Show.\n");
			iReply = NX_REPLY_FAIL;
			break;
		}

		if( 0 > pProcessManager->GetAt( iReqIndex, &pReqInfo ) )
		{
			NX_DbgMsg( NX_DBG_ERR, "Fail, Request Process is not valid.\n" );
			iReply = NX_REPLY_FAIL;
			break;
		}

		int32_t iCmd = NX_REQUEST_FOCUS_MASK & iKey;
		int32_t iCount = 0;

		if( iCmd & NX_REQUEST_FOCUS_VIDEO )
		{
			NX_DbgMsg( NX_DBG_VBS, "#%d Reqeust Video Focus.\n", iCount++ );
			iRet = pProcessManager->Request( cbRequestVideoFocus, pReqInfo, pObj );
			if( 0 > iRet ) {
				NX_DbgMsg( NX_DBG_VBS, "Don't Get Video Focus.\n" );
				iReply = NX_REPLY_FAIL;
				break;
			}
		}

		if( iCmd & NX_REQUEST_FOCUS_AUDIO )
		{
			NX_DbgMsg( NX_DBG_VBS, "#%d Reqeust Audio Focus.\n", iCount++ );
			iRet = pProcessManager->Request( cbRequestAudioFocus, pReqInfo, pObj );
			if( 0 > iRet ) {
				NX_DbgMsg( NX_DBG_VBS, "Don't Get Audio Focus.\n" );
				iReply = NX_REPLY_FAIL;
				break;
			}
		}

		if( iCmd & NX_REQUEST_FOCUS_VIDEO_TRANSIENT )
		{
			NX_DbgMsg( NX_DBG_VBS, "#%d Request Video Focus Transient.\n", iCount++ );
			iRet = pProcessManager->Request( cbRequestVideoFocusTransient, pReqInfo, pObj );
			if( 0 > iRet ) {
				NX_DbgMsg( NX_DBG_VBS, "Don't Get Video Focus Transient.\n" );
				iReply = NX_REPLY_FAIL;
				break;
			}
		}

		if( iCmd & NX_REQUEST_FOCUS_AUDIO_TRANSIENT )
		{
			NX_DbgMsg( NX_DBG_VBS, "#%d Request Audio Focus Transient.\n", iCount++ );
			iRet = pProcessManager->Request( cbRequestAudioFocusTransient, pReqInfo, pObj );
			if( 0 > iRet ) {
				NX_DbgMsg( NX_DBG_VBS, "Don't Get Audio Focus Transient.\n" );
				iReply = NX_REPLY_FAIL;
				break;
			}
		}

		NX_DbgMsg( NX_DBG_VBS, "#%d Request Process Show.\n", iCount++ );
		pProcessManager->Request( cbRequestCurrentProcessShow, pReqInfo, pObj );

		NX_DbgMsg( NX_DBG_VBS, "#%d Request Final Process Hide.\n", iCount++ );
		pProcessManager->Request( cbRequestFinalProcessHide, pReqInfo, pObj );

		NX_DbgMsg( NX_DBG_VBS, "#%d Change Z-Order.\n", iCount++ );
		if( 0 <= iReqIndex )
		{
			for( int32_t i = iReqIndex; i > 0; i-- )
			{
				pProcessManager->Swap( i, i - 1 );
			}
		}
		pProcessManager->PrintInfo();
		break;
	}

	case NX_REQUEST_PROCESS_HIDE:
	case NX_REQUEST_FOCUS_VIDEO_LOSS | NX_REQUEST_PROCESS_HIDE:
	case NX_REQUEST_FOCUS_AUDIO_LOSS | NX_REQUEST_PROCESS_HIDE:
	case NX_REQUEST_FOCUS_AV_LOSS | NX_REQUEST_PROCESS_HIDE:
	{
		NX_DbgMsg( NX_DBG_VBS, "NX_REQUEST_PROCESS_HIDE.\n");
		NX_DbgMsg( NX_DBG_VBS, ">>>>> Request Process: %s\n", ((NX_PROCESS_INFO*)pPayload)->szAppName );

		NX_PROCESS_INFO *pReqInfo = NULL;
		int32_t iReqIndex = pProcessManager->IndexOf( (NX_PROCESS_INFO*)pPayload );
		if( 0 < iReqIndex )
		{
			NX_DbgMsg( NX_DBG_WARN, "Fail, Already Hide.\n");
			iReply = NX_REPLY_FAIL;
			break;
		}

		if( 0 > pProcessManager->GetAt( iReqIndex, &pReqInfo ) )
		{
			NX_DbgMsg( NX_DBG_ERR, "Fail, Request Process is not valid.\n" );
			iReply = NX_REPLY_FAIL;
			break;
		}

		int32_t iCmd = NX_REQUEST_FOCUS_MASK & iKey;
		int32_t iCount = 0;

		NX_DbgMsg( NX_DBG_VBS, "#%d Request Final Process Show.\n", iCount++ );
		pProcessManager->Request( cbRequestFinalProcessShow, (NX_PROCESS_INFO*)pReqInfo, pObj );

		NX_DbgMsg( NX_DBG_VBS, "#%d Request Process Hide.\n", iCount++ );
		pProcessManager->Request( cbRequestCurrentProcessHide, (NX_PROCESS_INFO*)pReqInfo, pObj );

		if( iCmd & NX_REQUEST_FOCUS_VIDEO_LOSS )
		{
			NX_DbgMsg( NX_DBG_VBS, "#%d Request Video Focus Loss.\n", iCount++ );
			pProcessManager->Request( cbRequestVideoFocusLoss, (NX_PROCESS_INFO*)pReqInfo, pObj );
		}

		if( iCmd & NX_REQUEST_FOCUS_AUDIO_LOSS )
		{
			NX_DbgMsg( NX_DBG_VBS, "#%d Request Audio Focus Loss.\n", iCount++ );
			pProcessManager->Request( cbRequestAudioFocusLoss, (NX_PROCESS_INFO*)pReqInfo, pObj );
		}

		NX_DbgMsg( NX_DBG_VBS, "#%d Change Z-Order.\n", iCount++ );
		if( 0 <= iReqIndex )
		{
			for( int32_t i = iReqIndex; i < pProcessManager->GetCount() - 1; i++ )
			{
				pProcessManager->Swap( i, i + 1 );
			}
		}
		pProcessManager->PrintInfo();
		break;
	}

	case NX_REQUEST_LAUNCHER_SHOW:
	case NX_REQUEST_FOCUS_VIDEO_LOSS | NX_REQUEST_LAUNCHER_SHOW:
	case NX_REQUEST_FOCUS_AUDIO_LOSS | NX_REQUEST_LAUNCHER_SHOW:
	case NX_REQUEST_FOCUS_AV_LOSS | NX_REQUEST_LAUNCHER_SHOW:
	{
		NX_DbgMsg( NX_DBG_VBS, "NX_REQUEST_LAUNCHER_SHOW.\n");
		NX_DbgMsg( NX_DBG_VBS, ">>>>> Request Process: %s\n", ((NX_PROCESS_INFO*)pPayload)->szAppName );

		NX_PROCESS_INFO *pReqInfo = NULL;
		int32_t iReqIndex = pProcessManager->IndexOf( (NX_PROCESS_INFO*)pPayload );
		if( 0 > pProcessManager->GetAt( iReqIndex, &pReqInfo ) )
		{
			NX_DbgMsg( NX_DBG_ERR, "Fail, Request Process is not valid.\n" );
			iReply = NX_REPLY_FAIL;
			break;
		}

		int32_t iCmd = NX_REQUEST_FOCUS_MASK & iKey;
		int32_t iCount = 0;

		NX_DbgMsg( NX_DBG_VBS, "#%d Process Show. ( Force Launcher Show )\n", iCount++ );
		pProcessManager->Request( cbRequestLauncherShow, pReqInfo, pObj );

		NX_DbgMsg( NX_DBG_VBS, "#%d Current Process Hide.\n", iCount++ );
		pProcessManager->Request( cbRequestCurrentProcessHide, pReqInfo, pObj );

		if( iCmd & NX_REQUEST_FOCUS_VIDEO_LOSS )
		{
			NX_DbgMsg( NX_DBG_VBS, "#%d Request Video Focus Loss.\n", iCount++ );
			pProcessManager->Request( cbRequestVideoFocusLoss, pReqInfo, pObj );
		}

		if( iCmd & NX_REQUEST_FOCUS_AUDIO_LOSS )
		{
			NX_DbgMsg( NX_DBG_VBS, "#%d Request Audio Focus Loss.\n", iCount++ );
			pProcessManager->Request( cbRequestAudioFocusLoss, pReqInfo, pObj );
		}

		NX_DbgMsg( NX_DBG_VBS, "#%d Change Z-Order.\n", iCount++ );

		NX_PROCESS_INFO *pLauncherInfo = NULL;
		int32_t iLauncherIndex = pProcessManager->IndexOf( NX_PROCESS_FLAG_LAUNCHER );
		if( 0 > pProcessManager->GetAt( iLauncherIndex, &pLauncherInfo ) )
		{
			NX_DbgMsg( NX_DBG_ERR, "Fail, Launcher is not valid.\n" );
			iReply = NX_REPLY_FAIL;
			break;
		}

		if( 0 <= iLauncherIndex )
		{
			for( int32_t i = iLauncherIndex; i > 0; i-- )
			{
				pProcessManager->Swap( i, i - 1 );
			}
		}
		pProcessManager->PrintInfo();
		break;
	}

	case NX_EVENT_BROADCAST:
	{
		NX_DbgMsg( NX_DBG_VBS, "NX_EVENT_BROADCAST.\n");
		pProcessManager->Request( cbRequestEventBroadcast, pPayload, iPayloadSize, pObj );
		break;
	}

	case NX_REQUEST_PROCESS_INFO:
	{
		NX_DbgMsg( NX_DBG_VBS, "NX_REQUEST_PROCESS_INFO.\n");
		NX_DbgMsg( NX_DBG_VBS, ">>>>> Request Process: %s\n", ((NX_PROCESS_INFO*)pPayload)->szAppName );

		NX_PROCESS_INFO *pReqInfo = NULL;
		int32_t iReqIndex = pProcessManager->IndexOf( (NX_PROCESS_INFO*)pPayload );
		if( 0 > pProcessManager->GetAt( iReqIndex, &pReqInfo ) )
		{
			NX_DbgMsg( NX_DBG_ERR, "Fail, Request Process is not valid.\n" );
			iReply = NX_REPLY_FAIL;
			break;
		}

		void *pResult = NULL;
		int32_t iResultSize = 0;
		pProcessManager->SetProcessInfoToPayload( &pResult, &iResultSize );
		// pProcessManager->DumpPayloadToProcessInfo( pResult, iResultSize );

		pProcessManager->Request( cbRequestProcessInfo, pReqInfo, pResult, iResultSize, pObj );
		if( pResult ) free( pResult );
		break;
	}

	default:
		break;
	}

	NX_DbgMsg( NX_DBG_VBS, ">>>>> Reply Result: %s( 0x%08X )\n\n",
		(iReply == NX_REPLY_DONE) ? "NX_REPLY_DONE" : "NX_REPLY_FAIL", iReply );

	iSendSize = NX_IpcMakePacket( iReply, NULL, 0, pSendBuf, iMaxBufSize );

	return pIpcManager->Write( iSock, pSendBuf, iSendSize );
}

//------------------------------------------------------------------------------
#define NX_UEVENT_STORAGE_ADD		"add@/devices/platform"
#define NX_UEVENT_STORAGE_REMOVE	"remove@/devices/platform"

const char *pstDeviceReserved[] = {
	"mmcblk0"
};

const char *pstMountPosition[] = {
	"/tmp/media",
};

#if NX_ENABLE_VOLUME_MANAGER
static void cbVolumeEvent( uint32_t iEventType, uint8_t *pDevice, uint8_t *pMountPos, void *pObj )
{
	CNX_PostProcess*	pPostProcess    = ((NX_DAUDIO_MANAGER*)pObj)->pPostProcess;
	CNX_ProcessManager*	pProcessManager = ((NX_DAUDIO_MANAGER*)pObj)->pProcessManager;

	if( iEventType == NX_EVENT_SDCARD_INSERT )			NX_DbgMsg( NX_DBG_DEBUG, "Insert SD Card.\n" );
	else if( iEventType == NX_EVENT_SDCARD_REMOVE )		NX_DbgMsg( NX_DBG_DEBUG, "Remove SD Card.\n" );
	else if( iEventType == NX_EVENT_USB_INSERT )		NX_DbgMsg( NX_DBG_DEBUG, "Insert USB Stroage.\n" );
	else if( iEventType == NX_EVENT_USB_REMOVE )		NX_DbgMsg( NX_DBG_DEBUG, "Remove USB Stroage.\n" );

	((NX_DAUDIO_MANAGER*)pObj)->iEventType = iEventType;
	int32_t iRet = pProcessManager->Request( cbReqeustVolumeEvent, pDevice, strlen((char*)pDevice), pObj );
	if( 0 > iRet ) {
		NX_DbgMsg( NX_DBG_ERR, "Fail, cbReqeustVolumeEvent().\n" );
	}

	pPostProcess->Start( cbMediaScanEvent, 3000, pObj );
}
#else
static void cbVolumeEvent( uint8_t *pDesc, uint32_t iDescSize, void *pObj )
{
	uint32_t iEventType = 0;
	CNX_PostProcess*	pPostProcess    = ((NX_DAUDIO_MANAGER*)pObj)->pPostProcess;
	CNX_ProcessManager*	pProcessManager = ((NX_DAUDIO_MANAGER*)pObj)->pProcessManager;

	for( uint32_t i = 0; i < sizeof(pstDeviceReserved) / sizeof(pstDeviceReserved[0]); i++ )
	{
		if( strstr( (char*)pDesc, pstDeviceReserved[i]) )
		return;
	}

	if( !strncmp( (char*)pDesc, NX_UEVENT_STORAGE_ADD, strlen(NX_UEVENT_STORAGE_ADD) ) &&
		strstr( (char*)pDesc, "mmcblk") )
	{
		iEventType = NX_EVENT_SDCARD_INSERT;
	}

	if( !strncmp( (char*)pDesc, NX_UEVENT_STORAGE_REMOVE, strlen(NX_UEVENT_STORAGE_REMOVE) ) &&
		strstr( (char*)pDesc, "mmcblk") )
	{
		iEventType = NX_EVENT_SDCARD_REMOVE;
	}

	if( !strncmp( (char*)pDesc, NX_UEVENT_STORAGE_ADD, strlen(NX_UEVENT_STORAGE_ADD) ) &&
		strstr( (char*)pDesc, "scsi_disk") )
	{
		iEventType = NX_EVENT_USB_INSERT;
	}

	if( !strncmp( (char*)pDesc, NX_UEVENT_STORAGE_REMOVE, strlen(NX_UEVENT_STORAGE_REMOVE) ) &&
		strstr( (char*)pDesc, "scsi_disk") )
	{
		iEventType = NX_EVENT_USB_REMOVE;
	}

	if( 0 == iEventType )
		return;

#if 1	// Enable duplicated iEventType
	if( ((NX_DAUDIO_MANAGER*)pObj)->iEventType == iEventType )
		return;
#endif

	if( iEventType == NX_EVENT_SDCARD_INSERT )			NX_DbgMsg( NX_DBG_DEBUG, "Insert SD Card.\n" );
	else if( iEventType == NX_EVENT_SDCARD_REMOVE )		NX_DbgMsg( NX_DBG_DEBUG, "Remove SD Card.\n" );
	else if( iEventType == NX_EVENT_USB_INSERT )		NX_DbgMsg( NX_DBG_DEBUG, "Insert USB Stroage.\n" );
	else if( iEventType == NX_EVENT_USB_REMOVE )		NX_DbgMsg( NX_DBG_DEBUG, "Remove USB Stroage.\n" );

	NX_DbgMsg( NX_DBG_DEBUG, ">>>>> pDesc( %zu ) : %s\n", strlen((char*)pDesc), pDesc );

	((NX_DAUDIO_MANAGER*)pObj)->iEventType = iEventType;
	int32_t iRet = pProcessManager->Request( cbReqeustVolumeEvent, pDesc, strlen((char*)pDesc), pObj );
	if( 0 > iRet ) {
		NX_DbgMsg( NX_DBG_ERR, "Fail, cbReqeustVolumeEvent().\n" );
	}

	pPostProcess->Start( cbMediaScanEvent, 3000, pObj );
}
#endif

//------------------------------------------------------------------------------
#define NX_APP_EXEC 	"/usr/bin/NxTestApp"

static int32_t cbAvmServerData( int32_t iSock, uint8_t *pSendBuf, uint8_t *pRecvBuf, int32_t iMaxBufSize, void *pObj )
{
	INX_IpcManager *pIpcManager = (INX_IpcManager*)pObj;

	uint32_t iKey;
	void* pPayload;
	int32_t iPayloadSize;
	int32_t iRecvSize, iSendSize = 0;

	iRecvSize = pIpcManager->Read( iSock, pRecvBuf, iMaxBufSize );
	iSendSize = NX_IpcMakePacket( NX_REPLY_DONE, NULL, 0, pSendBuf, iMaxBufSize );

	NX_IpcParsePacket( pRecvBuf, iRecvSize, &iKey, &pPayload, &iPayloadSize );

	switch( iKey )
	{
	case NX_REQUEST_PROCESS_SHOW:
	{
		break;
	}

	case NX_REQUEST_PROCESS_HIDE:
	{
		break;
	}

	case NX_REQUEST_FOCUS_VIDEO:
	case NX_REQUEST_FOCUS_VIDEO_TRANSIENT:
	{
		if( ((NX_PROCESS_INFO*)pPayload)->iVideoPriority < NX_FOCUS_PRIORITY_MOST )
		{
			iSendSize = NX_IpcMakePacket( NX_REPLY_FAIL, NULL, 0, pSendBuf, iMaxBufSize );
		}
		break;
	}

	default:
		break;
	}

	iSendSize = pIpcManager->Write( iSock, pSendBuf, iSendSize );
	return iSendSize;
}

//------------------------------------------------------------------------------
static void cbCommandCallback( uint8_t *pBuf, int32_t iBufSize, void *pObj )
{
	CNX_ProcessManager*	pProcessManager = ((NX_DAUDIO_MANAGER*)pObj)->pProcessManager;

	int32_t iRet;
	NX_PROCESS_INFO info;
	memset( &info, 0x00, sizeof(info) );
	sprintf( info.szAppName, NX_APP_EXEC );

	if( !strcmp((char*)pBuf, "1") )
	{
		// -1: Requested Process is exist in process list.
		//  0: Requested Process is not exist in process list.
		iRet = pProcessManager->Request( cbRequestIsProcessCheck, &info, pObj );
		if( !iRet )
		{
			NX_PROCESS_INFO* pReqInfo;
			pProcessManager->GetAt( 0, &pReqInfo );
			if( pReqInfo->iFlags & NX_PROCESS_FLAG_CAMERA )
			{
				iRet = NX_RequestCommand( NX_REQUEST_PROCESS_HIDE, pReqInfo );
				if( NX_REPLY_DONE != iRet )
				{
					printf( "Fail, NX_RequestCommand(). ( cmd: 0x%04X, iRet: %d )\n", NX_REQUEST_PROCESS_HIDE, iRet );
				}
			}

			NX_RunProcess( NX_APP_EXEC, NULL, true );
			usleep(1000000);

			INX_IpcManager* pIpcManager = GetIpcManager();
			pIpcManager->RegServerCallbackFunc( &cbAvmServerData, (void*)pIpcManager );

			info.iProcessId = NX_GetPid( NX_APP_EXEC );
			info.iVideoPriority = NX_FOCUS_PRIORITY_MOST;
			info.iDisplayDevice = NX_DISPLAY_SECONDARY;
			info.iFlags |= NX_PROCESS_FLAG_CAMERA;

			sprintf( info.szSockName, "nexell.daudio.%d", info.iProcessId );
			pIpcManager->StartServer( info.szSockName );

			((NX_DAUDIO_MANAGER*)pObj)->pAvmIpcManager = pIpcManager;

			iRet = NX_RequestCommand( NX_REQUEST_PROCESS_ADD, &info );
			if( NX_REPLY_DONE != iRet )
			{
				printf( "Fail, NX_RequestCommand(). ( cmd: 0x%04X, iRet: %d )\n", NX_REQUEST_PROCESS_ADD, iRet );
				return ;
			}
		}
	}
	else
	{
		// -1: Requested Process is exist in process list.
		//  0: Requested Process is not exist in process list.
		iRet = pProcessManager->Request( cbRequestIsProcessCheck, &info, pObj );
		if( 0 > iRet )
		{
			iRet = NX_RequestCommand( NX_REQUEST_PROCESS_REMOVE, &info );
			if( NX_REPLY_DONE != iRet )
			{
				printf( "Fail, NX_RequestCommand(). ( cmd: 0x%04X, iRet: %d )\n", NX_REQUEST_PROCESS_REMOVE, iRet );
				return ;
			}

			INX_IpcManager* pIpcManager = ((NX_DAUDIO_MANAGER*)pObj)->pAvmIpcManager;
			if( pIpcManager )
			{
				pIpcManager->StopServer();
				delete pIpcManager;
				((NX_DAUDIO_MANAGER*)pObj)->pAvmIpcManager = NULL;
			}

			NX_KillProcess( NX_APP_EXEC );

			NX_PROCESS_INFO* pReqInfo;
			if( 0 > pProcessManager->GetAt( 0, &pReqInfo ) )
				return ;

			if( !(pReqInfo->iFlags & NX_PROCESS_FLAG_LAUNCHER) )
				return ;

			if( 0 > pProcessManager->GetAt( 1, &pReqInfo ) )
				return ;

			if( pReqInfo->iFlags & NX_PROCESS_FLAG_CAMERA )
			{
				iRet = NX_RequestCommand( NX_REQUEST_PROCESS_SHOW, pReqInfo );
				if( NX_REPLY_DONE != iRet )
				{
					printf( "Fail, NX_RequestCommand(). ( cmd: 0x%04X, iRet: %d )\n", NX_REQUEST_PROCESS_SHOW, iRet );
				}
			}
		}
	}
}

//------------------------------------------------------------------------------
static int32_t cbRequestIsLauncher( int32_t iIndex, NX_PROCESS_INFO *pInfo, NX_PROCESS_INFO *pReqInfo, void *pObj )
{
	if( pInfo->iFlags & NX_PROCESS_FLAG_LAUNCHER )
	{
		NX_DbgMsg( NX_DBG_DEBUG, "The Launcher is exist.\n" );
		return -1;
	}
	return 0;
}

//------------------------------------------------------------------------------
static void cbTerminateFunc( void *pObj )
{
	int32_t iRet = 0;
	int32_t iCount = 0;

	CNX_ProcessManager*	pProcessManager = ((NX_DAUDIO_MANAGER*)pObj)->pProcessManager;;

	NX_DbgMsg( NX_DBG_VBS, "#%d Request Is Launcher!?\n", iCount++ );
	iRet = pProcessManager->Request( cbRequestIsLauncher, NULL, pObj );
	if( 0 == iRet ) {
		NX_DbgMsg( NX_DBG_VBS, ">>> Launcher is not running.\n" );
		//
		//	Retry to run NxLauncher.
		//
#if NX_ENABLE_RETRY_LAUNCHER
		NX_RunProcess( NX_LAUNCHER_EXEC );
#endif
		return ;
	}

	// NX_DbgMsg( NX_DBG_VBS, "#%d Request is Launcher Show!?\n", iCount++ );
	// iRet = pProcessManager->Request( cbRequestIsLauncherShow, NULL, pObj );
	// if( 0 > iRet ) {
	// 	printf("Launcher is already showing.\n");
	// 	return ;
	// }

	NX_DbgMsg( NX_DBG_VBS, "#%d Launcher Show.\n", iCount++ );
	iRet = pProcessManager->Request( cbRequestLauncherShow, NULL, pObj );
	if( 0 > iRet ) {
		printf("Fail, Launcher Show.\n");
		return ;
	}

	return ;
}

//------------------------------------------------------------------------------
int32_t main( int32_t argc, char *argv[] )
{
	int32_t opt;

	while( -1 != (opt=getopt(argc, argv, "d:")))
	{
		switch( opt )
		{
		case 'd' :
			// VBS(0), DEBUG(1), INFO(2), WARN(3), ERR(4), DISABLE(5)
			if( 0 > atoi(optarg) || 5 < atoi(optarg) )
			{
				printf("Unknown Debug Level. ( %d )", atoi(optarg) );
				break;
			}

			NX_ChangeDebugLevel( (5 == atoi(optarg)) ? NX_DBG_DISABLE : atoi(optarg)+2 );
		 	break;
		default :
			break;
		}
	}

	register_signal();

	CNX_ProcessManager* pProcessManager = new CNX_ProcessManager();
	INX_IpcManager*		pIpcManager     = GetIpcManagerHandle();
	CNX_UeventManager*	pUeventManager	= new CNX_UeventManager();
	CNX_VolumeManager*  pVolumeManager  = new CNX_VolumeManager();
	CNX_PostProcess*	pPostProcess	= new CNX_PostProcess();
	CNX_MediaScanner* 	pMediaScanner	= new CNX_MediaScanner();
	CNX_CommandManager*	pCommandManager = new CNX_CommandManager();

	memset( &stManager, 0x00, sizeof(stManager) );
	stManager.pProcessManager = pProcessManager;
	stManager.pIpcManager     = pIpcManager;
	stManager.pUeventManager  = pUeventManager;
	stManager.pVolumeManager  = pVolumeManager;
	stManager.pPostProcess    = pPostProcess;
	stManager.pMediaScanner   = pMediaScanner;
	stManager.pCommandManager = pCommandManager;

	pProcessManager->RegTerminateCallback( cbTerminateFunc, (void*)&stManager );

	pIpcManager->RegServerCallbackFunc( &cbServerData, (void*)&stManager );
	pIpcManager->StartServer( "nexell.daudio.manager" );

	pVolumeManager->SetDeviceReserved( pstDeviceReserved, sizeof(pstDeviceReserved) / sizeof(pstDeviceReserved[0]) );
	pVolumeManager->SetMountPosition( pstMountPosition, sizeof(pstMountPosition) / sizeof(pstMountPosition[0]) );
#if NX_ENABLE_VOLUME_MANAGER
	pVolumeManager->RegEventCallbackFunc( &cbVolumeEvent, (void*)&stManager );
	pVolumeManager->Start();
#else
	pUeventManager->RegEventCallbackFunc( &cbVolumeEvent, (void*)&stManager );
#endif

	pCommandManager->RegEventCallbackFunc( &cbCommandCallback, (void*)&stManager );

#if NX_ENABLE_MEDIA_SCANNER
	stManager.iEventType = 0;
	pPostProcess->Start( cbMediaScanEvent, 3000, &stManager );
#endif

	while(1)
	{
		usleep( 100000 );
	}

	return 0;
}

