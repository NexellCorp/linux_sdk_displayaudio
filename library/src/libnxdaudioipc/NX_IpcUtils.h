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

#ifndef __NX_IPCUTILS_H__
#define __NX_IPCUTILS_H__

#include <stdint.h>

#include <INX_IpcManager.h>
#include <NX_IpcPacket.h>
#include <NX_Type.h>

#define NX_SOCK_MANAGER		"nexell.daudio.manager"

int32_t NX_GetProcessInfo( NX_PROCESS_INFO *pInfo );
int32_t NX_DumpProcessInfo( void *pPayload, int32_t iPayloadSize );

int32_t NX_RequestCommand( int32_t iCommand );
int32_t NX_RequestCommand( int32_t iCommand, NX_PROCESS_INFO *pInfo );
int32_t NX_RequestCommand( int32_t iCommand, void *pPayload, int32_t iPayloadSize );

int32_t NX_ReplyWait();
int32_t NX_ReplyDone();

void NX_DumpHex( const void *pData, int32_t iSize );

#endif	// __NX_IPCUTILS_H__
