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

#ifndef __CNX_IPCMANAGER_H__
#define __CNX_IPCMANAGER_H__

#include <stdint.h>
#include <pthread.h>

#include <NX_Type.h>

#include "CNX_Base.h"
#include "CNX_Semaphore.h"
#include "INX_IpcManager.h"
#include "NX_IpcPacket.h"

class CNX_IpcManager
	: public INX_IpcManager
{
public:
	CNX_IpcManager();
	~CNX_IpcManager();

	static CNX_IpcManager*	GetInstance();

public:	// Common
	int32_t Write( int32_t iSock, uint8_t *pBuf, int32_t iSize );
	int32_t Read( int32_t iSock, uint8_t *pBuf, int32_t iSize );

	int32_t ReplyWait();
	int32_t ReplyDone();

	const char *GetVersion() { return NX_VERSION_LIBDAUDIOIPC; }

public:	// For Server
	int32_t StartServer( const char *pSock );
	int32_t StopServer();
	void	RegServerCallbackFunc( int32_t (*cbFunc)( int32_t iSock, uint8_t *pSendBuf, uint8_t*pRecvBuf, int32_t iMaxBufSize, void *pObj ), void *pObj );

public:	// For Client
	int32_t SendCommand( const char *pSock, uint8_t *pSendBuf, int32_t iSendSize, uint8_t *pRecvBuf, int32_t iRecvMaxSize );
	int32_t SendCommand( const char *pSock, int32_t (*cbFunc)( int32_t iSock, uint8_t *pSendBuf, uint8_t *pRecvBuf, int32_t iMaxBufSize, void *pObj ), void *pObj );

private:
	int32_t Start();
	int32_t Stop();
	int32_t Accept( int32_t iSock, struct sockaddr *pAddr, socklen_t *pAddrLen );

	static void *ThreadStub( void *pObj );
	void ThreadProc();

private:
	static CNX_IpcManager*	m_pstInstance;
	CNX_Semaphore*	m_pSem;

	char*		m_pSock;
	int32_t		(*m_ServerCallbackFunc)( int32_t iSock, uint8_t *pRecvBuf, uint8_t *pSendBuf, int32_t iMaxSendBufSize, void *pObj );
	void*		m_pObj;

	pthread_t	m_hThread;
	int32_t		m_bThreadRun;

private:
	CNX_IpcManager (const CNX_IpcManager &Ref);
	CNX_IpcManager &operator=(const CNX_IpcManager &Ref);
};

#endif	// __CNX_IPCMANAGER_H__
