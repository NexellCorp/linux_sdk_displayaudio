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
#include <ctype.h>
#include <unistd.h>

#include "NX_IpcUtils.h"

//------------------------------------------------------------------------------
int32_t NX_GetProcessInfo( NX_PROCESS_INFO *pInfo )
{
	int32_t iReadSize = -1;
	memset( pInfo, 0x00, sizeof(NX_PROCESS_INFO) );

	char szPath[256];
	pInfo->iProcessId = getpid();
	snprintf( szPath, sizeof(szPath), "/proc/%d/cmdline", pInfo->iProcessId );

	FILE *pFile = fopen( szPath, "r" );
	if( pFile == NULL )
	{
		printf("Fail, Invalid Process.\n");
		return -1;
	}

	iReadSize = fscanf( pFile, "%s", pInfo->szAppName );
	fclose( pFile );

#if 0
	snprintf( pInfo->szSockName, sizeof(pInfo->szSockName), "nexell.daduio.%d", pInfo->iProcessId );
#else
	char pid[128];
	sprintf( pid, "%d", pInfo->iProcessId );

	strcpy( pInfo->szSockName, "nexell.daudio." );
	strcat( pInfo->szSockName, pid );
#endif

	// NX_DumpHex( pInfo, sizeof(NX_PROCESS_INFO) );

	printf("Process Information. ( name: %s, pid: %d, sock: %s )\n",
		pInfo->szAppName, pInfo->iProcessId, pInfo->szSockName );

	return (iReadSize > 0) ? 0 : -1;
}

//------------------------------------------------------------------------------
int32_t NX_DumpProcessInfo( void *pPayload, int32_t iPayloadSize )
{
	if( 0 != (iPayloadSize % sizeof(NX_PROCESS_INFO)) )
	{
		printf( "Fail, Mismatch payload size.\n" );
		return -1;
	}

	int32_t iNumOfInfo = (int32_t)(iPayloadSize / sizeof(NX_PROCESS_INFO));
	NX_PROCESS_INFO *pInfo = (NX_PROCESS_INFO*)pPayload;

	for( int32_t i = 0; i < iNumOfInfo; i++ )
	{
		printf("================================================================================\n");
		printf("szAppName      = %s\n", pInfo[i].szAppName);
		printf("szSockName     = %s\n", pInfo[i].szSockName);
		printf("iProcessId     = %d\n", pInfo[i].iProcessId);
		printf("iFlags         = 0x%08X\n", pInfo[i].iFlags);
		printf("iVideoPriority = %d\n", pInfo[i].iVideoPriority);
		printf("iAudioPriority = %d\n", pInfo[i].iAudioPriority);
		printf("iDisplayDevice = %d\n", pInfo[i].iDisplayDevice);
		printf("================================================================================\n");
	}
	return 0;
}

//------------------------------------------------------------------------------
int32_t NX_ReplyWait()
{
	return GetIpcManagerHandle()->ReplyWait();
}

//------------------------------------------------------------------------------
int32_t NX_ReplyDone()
{
	return GetIpcManagerHandle()->ReplyDone();
}

//------------------------------------------------------------------------------
int32_t NX_RequestCommand( int32_t iCommand )
{
	uint8_t	sendBuf[MAX_PACKET_SIZE], recvBuf[MAX_PACKET_SIZE];
	uint32_t iKey;
	void* pPayload;
	int32_t iPayloadSize;
	int32_t iSendSize, iRecvSize;

	NX_PROCESS_INFO info;
	NX_GetProcessInfo( &info );

	iSendSize = NX_IpcMakePacket( iCommand, (void*)&info, sizeof(NX_PROCESS_INFO), sendBuf, sizeof(sendBuf) );
	iRecvSize = GetIpcManagerHandle()->SendCommand( NX_SOCK_MANAGER, sendBuf, iSendSize, recvBuf, sizeof(recvBuf) );
	if( 0 > iRecvSize )
	{
		printf( "Fail, SendCommand(). ( cmd: 0x%04X )\n", iCommand );
		return -1;
	}

	if( 0 > NX_IpcParsePacket( recvBuf, iRecvSize, &iKey, &pPayload, &iPayloadSize ) )
	{
		printf( "Fail, NX_IpcParsePacket().\n" );
		return -1;
	}

	// printf( "Done, Reply %s.\n", (iKey == NX_REPLY_DONE) ? "Successful" : "Fail" );
	return iKey;	// Received Key Value.
}

//------------------------------------------------------------------------------
int32_t NX_RequestCommand( int32_t iCommand, NX_PROCESS_INFO *pInfo )
{
	uint8_t	sendBuf[MAX_PACKET_SIZE], recvBuf[MAX_PACKET_SIZE];
	uint32_t iKey;
	void* pPayload;
	int32_t iPayloadSize;
	int32_t iSendSize, iRecvSize;

	iSendSize = NX_IpcMakePacket( iCommand, (void*)pInfo, sizeof(NX_PROCESS_INFO), sendBuf, sizeof(sendBuf) );
	iRecvSize = GetIpcManagerHandle()->SendCommand( NX_SOCK_MANAGER, sendBuf, iSendSize, recvBuf, sizeof(recvBuf) );
	if( 0 > iRecvSize )
	{
		printf( "Fail, SendCommand(). ( cmd: 0x%04X )\n", iCommand );
		return -1;
	}

	if( 0 > NX_IpcParsePacket( recvBuf, iRecvSize, &iKey, &pPayload, &iPayloadSize ) )
	{
		printf( "Fail, NX_IpcParsePacket().\n" );
		return -1;
	}

	// printf( "Done, Reply %s.\n", (iKey == NX_REPLY_DONE) ? "Successful" : "Fail" );
	return iKey;	// Received Key Value.
}

//------------------------------------------------------------------------------
int32_t NX_RequestCommand( int32_t iCommand, void *pPayload, int32_t iPayloadSize )
{
	uint8_t	sendBuf[MAX_PACKET_SIZE], recvBuf[MAX_PACKET_SIZE];
	uint32_t iKey;
	void* pResult;
	int32_t iResultSize;
	int32_t iSendSize, iRecvSize;

	iSendSize = NX_IpcMakePacket( iCommand, pPayload, iPayloadSize, sendBuf, sizeof(sendBuf) );
	iRecvSize = GetIpcManagerHandle()->SendCommand( NX_SOCK_MANAGER, sendBuf, iSendSize, recvBuf, sizeof(recvBuf) );
	if( 0 > iRecvSize )
	{
		printf( "Fail, SendCommand(). ( cmd: 0x%04X )\n", iCommand );
		return -1;
	}

	if( 0 > NX_IpcParsePacket( recvBuf, iRecvSize, &iKey, &pResult, &iResultSize ) )
	{
		printf( "Fail, NX_IpcParsePacket().\n" );
		return -1;
	}

	// printf( "Done, Reply %s.\n", (iKey == NX_REPLY_DONE) ? "Successful" : "Fail" );
	return iKey;	// Received Key Value.
}

//------------------------------------------------------------------------------
void NX_DumpHex( const void *pData, int32_t iSize )
{
	int32_t i=0, iOffset = 0;
	char tmp[32];
	static char lineBuf[1024];
	const uint8_t *_data = (const uint8_t*)pData;
	while( iOffset < iSize )
	{
		sprintf( lineBuf, "%08lx : ", (unsigned long)iOffset );
		for( i=0 ; i<16 ; ++i )
		{
			if( i == 8 ){
				strcat( lineBuf, " " );
			}
			if( iOffset+i >= iSize )
			{
				strcat( lineBuf, "   " );
			}
			else{
				sprintf(tmp, "%02x ", _data[iOffset+i]);
				strcat( lineBuf, tmp );
			}
		}
		strcat( lineBuf, "   " );

		//	Add ACSII A~Z, & Number & String
		for( i=0 ; i<16 ; ++i )
		{
			if( iOffset+i >= iSize )
			{
				break;
			}
			else{
				if( isprint(_data[iOffset+i]) )
				{
					sprintf(tmp, "%c", _data[iOffset+i]);
					strcat(lineBuf, tmp);
				}
				else
				{
					strcat( lineBuf, "." );
				}
			}
		}

		strcat(lineBuf, "\n");
		printf( "%s", lineBuf );
		iOffset += 16;
	}
}
