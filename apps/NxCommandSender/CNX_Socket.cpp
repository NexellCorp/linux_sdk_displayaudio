//------------------------------------------------------------------------------
//
//	Copyright (C) 2018 Nexell Co. All Rights Reserved
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
#include <stddef.h>
#include <unistd.h>
#include <poll.h>
#include <errno.h>
#include <sys/un.h>
#include <sys/socket.h>

#include "CNX_Socket.h"

//------------------------------------------------------------------------------
CNX_Socket::CNX_Socket()
	: m_hServer( -1 )
	, m_iAddrLen( -1 )
{

}

//------------------------------------------------------------------------------
CNX_Socket::~CNX_Socket()
{

}

//------------------------------------------------------------------------------
int32_t CNX_Socket::Write( int32_t iSock, uint8_t *pBuf, int32_t iSize )
{
	return write( iSock, pBuf, iSize );
}

//------------------------------------------------------------------------------
int32_t CNX_Socket::Read( int32_t iSock, uint8_t *pBuf, int32_t iSize )
{
	int32_t iRet;
	struct pollfd hPoll;

	hPoll.fd      = iSock;
	hPoll.events  = POLLIN | POLLERR;
	hPoll.revents = 0;

	iRet = poll( (struct pollfd*)&hPoll, 1, 1000 );
	if( 0 < iRet )
	{
		return read( iSock, pBuf, iSize );
	}

	return -1;
}

//------------------------------------------------------------------------------
int32_t CNX_Socket::Open( const char *pSock )
{
	if( 0 > (m_hServer = socket(AF_UNIX, SOCK_STREAM, 0)) )
	{
		printf( "Fail, socket(). ( %s )\n", strerror(errno) );
		goto ERROR;
	}

	memset( &m_stAddr, 0x00, sizeof(m_stAddr) );
	m_stAddr.sun_family  = AF_UNIX;
	m_stAddr.sun_path[0] = '\0';	// for abstract namespace
	strcpy( m_stAddr.sun_path + 1, pSock );

	m_iAddrLen = 1 + strlen(pSock) + offsetof(struct sockaddr_un, sun_path);

	if( 0 > bind(m_hServer, (struct sockaddr*)&m_stAddr, m_iAddrLen) )
	{
		printf( "Fail, bind(). ( %s )\n", strerror(errno) );
		goto ERROR;
	}

	if( 0 > listen(m_hServer, 128) )
	{
		printf( "Fail, listen(). ( %s )\n", strerror(errno) );
		goto ERROR;
	}

	return 0;

ERROR:
	if( 0 < m_hServer )
	{
		close( m_hServer );
		m_hServer = -1;
		m_iAddrLen = -1;
	}

	return -1;
}

//------------------------------------------------------------------------------
void CNX_Socket::Close()
{
	if( 0 < m_hServer )
	{
		close( m_hServer );
		m_hServer = -1;
	}
}

//------------------------------------------------------------------------------
int32_t CNX_Socket::Accept()
{
	int32_t iRet;
	struct pollfd hPoll;

	hPoll.fd      = m_hServer;
	hPoll.events  = POLLIN | POLLERR;
	hPoll.revents = 0;

	iRet = poll( (struct pollfd*)&hPoll, 1, 1000 );
	if( 0 < iRet )
	{
		return accept( m_hServer, (struct sockaddr*)&m_stAddr, (socklen_t*)&m_iAddrLen );
	}

	return -1;

}

//------------------------------------------------------------------------------
int32_t CNX_Socket::Connect( const char *pSock )
{
	int32_t iSock = -1, iAddrLen;
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

	iAddrLen = 1 + strlen(pSock) + offsetof(struct sockaddr_un, sun_path);

	if( 0 > connect(iSock, (struct sockaddr*)&stAddr, iAddrLen) )
	{
		printf( "Fail, connect(). ( node: %s, reason: %s )\n", pSock, strerror(errno) );
		close( iSock );
		return -1;
	}

	return iSock;
}
