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

#include <string.h>
#include <unistd.h>
#include <uevent.h>
#include <poll.h>

#include "CNX_UeventManager.h"

//------------------------------------------------------------------------------
CNX_UeventManager::CNX_UeventManager()
	: m_EventCallbackFunc( NULL )
	, m_pObj( NULL )
{
	Start();
}

//------------------------------------------------------------------------------
CNX_UeventManager::~CNX_UeventManager()
{
	Stop();
}

//------------------------------------------------------------------------------
void CNX_UeventManager::ThreadProc()
{
	char desc[1024];
	struct pollfd fds;

	uevent_init();

	fds.fd = uevent_get_fd();
	fds.events = POLLIN;

	while( m_bThreadRun )
	{
		int32_t err = poll( &fds, 1, 1000 );
		if( err > 0 )
		{
			if( fds.revents & POLLIN )
			{
				memset( desc, 0x00, sizeof(desc) );
				int32_t len = uevent_next_event(desc, sizeof(desc)-1);

				if( len < 1 ) continue;

				if( m_EventCallbackFunc )
				{
					m_EventCallbackFunc( (uint8_t*)desc, sizeof(desc)-1, m_pObj );
				}
			}
		}
	}
}

//------------------------------------------------------------------------------
void CNX_UeventManager::RegEventCallbackFunc( void (*cbFunc)(uint8_t*, uint32_t, void*), void *pObj )
{
	if( cbFunc )
	{
		m_EventCallbackFunc = cbFunc;
		m_pObj = pObj;
	}
}
