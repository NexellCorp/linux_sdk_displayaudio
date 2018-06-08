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

#ifndef __NX_IPCPACKET_H__
#define __NX_IPCPACKET_H__

#include <stdint.h>

#define MAX_PAYLOAD_SIZE		65535
#define MAX_PACKET_KEY_SIZE		4
#define MAX_PACKET_LENGTH_SIZE	2
#define MAX_PACKET_SIZE			MAX_PAYLOAD_SIZE + MAX_PACKET_KEY_SIZE + MAX_PACKET_LENGTH_SIZE

//
//	IPC Packet Struct
//	+-----------------+-------------------------+--------------------------+
//	| iKey ( 4bytes ) | iPayloadSize ( 2bytes ) | pPayload[0]..pPayload[n] |
//	+-----------------+-------------------------+--------------------------+
//

//
//	Parameters
//		Input	: iKey, pPayload, iPayloadSize, iOutMaxSize
//		Output	: pOutBuf
//	Return
//		On success, the number of bytes is returned.
//		Otherwise, -1 is returned.
//
int32_t NX_IpcMakePacket( uint32_t iKey, void *pPayload, int32_t iPayloadSize, void *pOutBuf, int32_t iOutMaxSize );


//
//	Parameters
//		Input	: pInBuf, iInBufSize
//		Output	: iKey, ppPayload, iPayloadSize
//	Return
//		On success, 0 is returned.
//		Otherwise, -1 is returned.
//
int32_t NX_IpcParsePacket( void *pInBuf, int32_t iInBufSize, uint32_t *iKey, void **ppPayload, int32_t *iPayloadSize );


//
//
//
void NX_IpcDumpPacket( void *pInBuf, int32_t iInBufSize, const char *pFunc = NULL, int32_t iLine = 0 );

#endif	// __NX_IPCPACKET_H__
