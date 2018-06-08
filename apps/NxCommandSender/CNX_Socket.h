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

#ifndef __CNX_SOCKET_H__
#define __CNX_SOCKET_H__

#include <stdint.h>
#include <sys/un.h>

class CNX_Socket
{
public:
	CNX_Socket();
	~CNX_Socket();

public:
	//	Common
	int32_t Write( int32_t iSock, uint8_t *pBuf, int32_t iSize );
	int32_t Read( int32_t iSock, uint8_t *pBuf, int32_t iSize );

public:
	//	Server
	int32_t Open( const char *pSock );
	void	Close();
	int32_t	Accept();

public:
	//	Client
	int32_t Connect( const char *pSock );

private:
	int32_t m_hServer;

	struct sockaddr_un m_stAddr;
	int32_t m_iAddrLen;

private:
	CNX_Socket (const CNX_Socket &Ref);
	CNX_Socket &operator=(const CNX_Socket &Ref);
};

#endif	// __CNX_SOCKET_H__
