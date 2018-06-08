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
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stddef.h>
#include <poll.h>
#include <errno.h>
#include <signal.h>
#include <sys/un.h>
#include <sys/socket.h>

#include "CNX_IpcManager.h"
#include "NX_IpcPacket.h"
#include "NX_Type.h"

#define MAX_TIMEOUT					3000 * 2
#define NX_SIGNAL_IGNORE_SIGPIPE	1

typedef struct NX_SOCK_INFO {
	CNX_IpcManager* pManager;
	int32_t			iSock;
} NX_SOCK_INFO;

//------------------------------------------------------------------------------
CNX_IpcManager::CNX_IpcManager()
	: m_pSem( new CNX_Semaphore(1, 0) )
	, m_pSock( NULL )
	, m_hThread( 0x00 )
	, m_bThreadRun( false )
{
}

//------------------------------------------------------------------------------
CNX_IpcManager::~CNX_IpcManager()
{
	m_pSem->Deinit();
	if( m_pSem )
		delete m_pSem;

	if( m_pSock )
		free( m_pSock );
}

//------------------------------------------------------------------------------
int32_t CNX_IpcManager::Write( int32_t iSock, uint8_t *pBuf, int32_t iSize )
{
	return write( iSock, pBuf, iSize );
}

//------------------------------------------------------------------------------
int32_t CNX_IpcManager::Read( int32_t iSock, uint8_t *pBuf, int32_t iSize )
{
	int32_t iRet;
	struct pollfd hPoll;

	hPoll.fd      = iSock;
	hPoll.events  = POLLIN | POLLERR;
	hPoll.revents = 0;

	iRet = poll( (struct pollfd*)&hPoll, 1, MAX_TIMEOUT );
	if( 0 < iRet )
	{
		return read( iSock, pBuf, iSize );
	}

	return -1;
}

//------------------------------------------------------------------------------
int32_t CNX_IpcManager::ReplyWait()
{
#ifndef QT_X11
	return m_pSem->Pend();
#else
	return 0;
#endif
}

//------------------------------------------------------------------------------
int32_t CNX_IpcManager::ReplyDone()
{
#ifndef QT_X11
	return m_pSem->Post();
#else
	return 0;
#endif
}

//------------------------------------------------------------------------------
int32_t CNX_IpcManager::StartServer( const char *pSock )
{
	if( m_pSock )
	{
		free( m_pSock );
		m_pSock = NULL;
	}

#if NX_SIGNAL_IGNORE_SIGPIPE
	signal( SIGPIPE, SIG_IGN );
#endif

	m_pSock = strdup( pSock );
	Start();

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_IpcManager::StopServer()
{
	Stop();

	if( m_pSock )
	{
		free( m_pSock );
		m_pSock = NULL;
	}

	return 0;
}

//------------------------------------------------------------------------------
void CNX_IpcManager::RegServerCallbackFunc( int32_t (*cbFunc)( int32_t, uint8_t*, uint8_t*, int32_t, void*), void *pObj )
{
	if( cbFunc )
	{
		m_ServerCallbackFunc = cbFunc;
		m_pObj = pObj;
	}
}

//------------------------------------------------------------------------------
int32_t CNX_IpcManager::SendCommand( const char *pSock, uint8_t *pSendBuf, int32_t iSendSize, uint8_t *pRecvBuf, int32_t iRecvMaxSize )
{
	int32_t iSock, iLen;
	struct sockaddr_un stAddr;

	if( 0 > (iSock = socket(AF_UNIX, SOCK_STREAM, 0)) )
	{
		printf( "Fail, socket(). ( %s )\n", strerror(errno) );
		return -1;
	}

	memset( &stAddr, 0x00, sizeof(stAddr) );
	stAddr.sun_family  = AF_UNIX;
	stAddr.sun_path[0] = '\0';	// for abstract namespace
	strcpy( stAddr.sun_path + 1, pSock );

	iLen = 1 + strlen(pSock) + offsetof(struct sockaddr_un, sun_path);

	if( 0 > connect(iSock, (struct sockaddr*)&stAddr, iLen) )
	{
		printf( "Fail, connect(). ( node: %s, reason: %s )\n", pSock, strerror(errno) );
		close( iSock );
		return -1;
	}

	if( 0 > Write(iSock, pSendBuf, iSendSize) )
	{
		printf( "Fail, write(). ( node: %s )\n", pSock );
		close( iSock );
		return -1;
	}

	int32_t iRecvSize = Read(iSock, pRecvBuf, iRecvMaxSize);
	if( 0 > iRecvSize )
	{
		printf( "Fail, read(). ( node: %s )\n", pSock );
		close( iSock );
		return -1;
	}

	close( iSock );
	return iRecvSize;
}

//------------------------------------------------------------------------------
int32_t CNX_IpcManager::SendCommand( const char *pSock, int32_t (*cbFunc)( int32_t iSock, uint8_t *pSendBuf, uint8_t *pRecvBuf, int32_t iMaxBufSize, void *pObj ), void *pObj )
{
	int32_t iRet = -1;
	int32_t iSock = -1, iLen;
	struct sockaddr_un stAddr;

	uint8_t sendBuf[MAX_PACKET_SIZE];
	uint8_t recvBuf[MAX_PACKET_SIZE];

	if( 0 > (iSock = socket(AF_UNIX, SOCK_STREAM, 0)) )
	{
		printf( "Fail, socket(). ( %s )\n", strerror(errno) );
		return -1;
	}

	memset( &stAddr, 0x00, sizeof(stAddr) );
	stAddr.sun_family  = AF_UNIX;
	stAddr.sun_path[0] = '\0';	// for abstract namespace
	strcpy( stAddr.sun_path + 1, pSock );

	iLen = 1 + strlen(pSock) + offsetof(struct sockaddr_un, sun_path);

	if( 0 > connect(iSock, (struct sockaddr*)&stAddr, iLen) )
	{
		printf( "Fail, connect(). ( node: %s, reason: %s )\n", pSock, strerror(errno) );
		close( iSock );
		return -1;
	}

	if( cbFunc )
		iRet = cbFunc( iSock, sendBuf, recvBuf, MAX_PACKET_SIZE, pObj );

	if( 0 <= iSock )
		close( iSock );

	return iRet;
}

//------------------------------------------------------------------------------
int32_t CNX_IpcManager::Start()
{
	if( false == m_bThreadRun )
	{
		m_bThreadRun = true;
		if( 0 != pthread_create( &m_hThread, NULL, CNX_IpcManager::ThreadStub, this) )
		{
			printf("Fail, pthread_create().\n");
			return -1;
		}
	}

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_IpcManager::Stop()
{
	if( true == m_bThreadRun )
	{
		m_bThreadRun = false;

		pthread_join( m_hThread, NULL );
		m_hThread = 0x00;
	}
	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_IpcManager::Accept( int32_t iSock, struct sockaddr *pAddr, socklen_t *pAddrLen )
{
	int32_t iRet;
	struct pollfd hPoll;

	hPoll.fd      = iSock;
	hPoll.events  = POLLIN | POLLERR;
	hPoll.revents = 0;

	iRet = poll( (struct pollfd*)&hPoll, 1, MAX_TIMEOUT );
	if( 0 < iRet )
	{
		return accept( iSock, pAddr, pAddrLen );
	}

	return -1;
}

//------------------------------------------------------------------------------
void *CNX_IpcManager::ThreadStub( void *pObj )
{
	if( NULL != pObj )
	{
		((CNX_IpcManager*)pObj)->ThreadProc();
	}

	return (void*)0xDEADDEAD;
}

//------------------------------------------------------------------------------
void CNX_IpcManager::ThreadProc()
{
	int32_t iSock, iLen;
	struct sockaddr_un stAddr;

	if( 0 > (iSock = socket(AF_UNIX, SOCK_STREAM, 0)) )
	{
		printf( "Fail, socket(). ( %s )\n", strerror(errno) );
		return ;
	}

	memset( &stAddr, 0x00, sizeof(stAddr) );
	stAddr.sun_family  = AF_UNIX;
	stAddr.sun_path[0] = '\0';	// for abstract namespace
	strcpy( stAddr.sun_path + 1, m_pSock );

	iLen = 1 + strlen(m_pSock) + offsetof(struct sockaddr_un, sun_path);

	if( 0 > bind(iSock, (struct sockaddr*)&stAddr, iLen) )
	{
		printf( "Fail, bind(). ( %s )\n", strerror(errno) );
		return ;
	}

	if( 0 > listen(iSock, 128) )
	{
		printf( "Fail, listen(). ( %s )\n", strerror(errno) );
		return ;
	}

	while( m_bThreadRun )
	{
		int32_t iClientSock = Accept( iSock, (struct sockaddr*)&stAddr, (socklen_t*)&iLen );
		if( 0 > iClientSock )
			continue;

		static uint8_t recvBuf[MAX_PACKET_SIZE];
		static uint8_t sendBuf[MAX_PACKET_SIZE];

		m_pSem->Init();

		if( m_ServerCallbackFunc )
		{
			m_ServerCallbackFunc( iClientSock, recvBuf, sendBuf, MAX_PACKET_SIZE, m_pObj );
		}

		if( 0 < iClientSock )
			close( iClientSock );

		m_pSem->Deinit();
	}

	close( iSock );
}

//------------------------------------------------------------------------------
CNX_IpcManager* CNX_IpcManager::m_pstInstance = NULL;

CNX_IpcManager* CNX_IpcManager::GetInstance()
{
	if( NULL == m_pstInstance )
	{
		m_pstInstance = new CNX_IpcManager();
	}

	return m_pstInstance;
}

//------------------------------------------------------------------------------
INX_IpcManager* GetIpcManagerHandle()
{
	return (INX_IpcManager*)CNX_IpcManager::GetInstance();
}

//------------------------------------------------------------------------------
INX_IpcManager* GetIpcManager()
{
	return new CNX_IpcManager();
}
