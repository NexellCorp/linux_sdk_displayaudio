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
#include <poll.h>
#include "uevent.h"

#include "CNX_UeventManager.h"

//------------------------------------------------------------------------------
CNX_UeventManager::CNX_UeventManager()
{
	m_bThreadRun = false;
}

//------------------------------------------------------------------------------
CNX_UeventManager::~CNX_UeventManager()
{
	Stop();
}

//------------------------------------------------------------------------------
void CNX_UeventManager::run()
{
	char desc[1024];
	struct pollfd fds;

	uevent_init();

	fds.fd = uevent_get_fd();
	fds.events = POLLIN;

	m_bThreadRun = true;

	while (m_bThreadRun)
	{
		int32_t err = poll( &fds, 1, 1000 );
		if( err > 0 )
		{
			if( fds.revents & POLLIN )
			{
				memset( desc, 0x00, sizeof(desc) );
				int32_t len = uevent_next_event(desc, sizeof(desc)-1);

				if( len < 1 ) continue;

				emit signalDetectUevent((uint8_t*)desc, sizeof(desc)-1);
			}
		}
	}
}

void CNX_UeventManager::Start()
{
	if (!m_bThreadRun)
		start();
}

void CNX_UeventManager::Stop()
{
	if (m_bThreadRun)
		m_bThreadRun = false;

	if (isRunning())
	{
		wait();
		quit();
	}
}
