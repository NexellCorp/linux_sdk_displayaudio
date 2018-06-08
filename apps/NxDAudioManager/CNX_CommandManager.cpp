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

#include "CNX_Socket.h"
#include "CNX_CommandManager.h"

//------------------------------------------------------------------------------
CNX_CommandManager::CNX_CommandManager()
	: m_EventCallbackFunc( NULL )
	, m_pObj( NULL )
{
	Start();
}

//------------------------------------------------------------------------------
CNX_CommandManager::~CNX_CommandManager()
{
	Stop();
}

//------------------------------------------------------------------------------
void CNX_CommandManager::ThreadProc()
{
	CNX_Socket *pSocket = new CNX_Socket();
	pSocket->Open( "daudio.command" );

	while( m_bThreadRun )
	{
		int32_t iSock = pSocket->Accept();
		if( 0 >= iSock )
			continue;

		uint8_t szBuf[512] = { 0x00, };
		int32_t iReadSize = pSocket->Read(iSock, szBuf, sizeof(szBuf));
		if( 0 < iReadSize )
		{
			if( m_EventCallbackFunc )
				m_EventCallbackFunc( szBuf, iReadSize, m_pObj );
		}

		close( iSock );
	}

	if( pSocket )
	{
		pSocket->Close();
		delete pSocket;
	}
}

//------------------------------------------------------------------------------
void CNX_CommandManager::RegEventCallbackFunc( void (*cbFunc)(uint8_t*, int32_t, void*), void *pObj )
{
	if( cbFunc )
	{
		m_EventCallbackFunc = cbFunc;
		m_pObj = pObj;
	}
}
