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
#include <string.h>
#include <ctype.h>

#include "NX_IpcPacket.h"

//------------------------------------------------------------------------------
int32_t NX_IpcMakePacket( uint32_t iKey, void *pPayload, int32_t iPayloadSize, void *pOutBuf, int32_t iOutMaxSize )
{
	if( pOutBuf == NULL )
	{
		printf( "Fail, Output Buffer is NULL.\n");
		return -1;
	}

	if( iPayloadSize > MAX_PAYLOAD_SIZE )
	{
		printf( "Fail, Payload Size is too large. ( payload: %d, max: %d )\n",
			iPayloadSize, MAX_PAYLOAD_SIZE );
		return -1;
	}

	if( iOutMaxSize < iPayloadSize + MAX_PACKET_KEY_SIZE + MAX_PACKET_LENGTH_SIZE )
	{
		printf( "Fail, Output Buffer Size is too small. ( current: %d, required: %d )\n",
			iOutMaxSize, iPayloadSize + MAX_PACKET_KEY_SIZE + MAX_PACKET_LENGTH_SIZE );
		return -1;
	}

	uint8_t *pBuf = (uint8_t*)pOutBuf;

	*pBuf++		= ((iKey >> 24) & 0xFF);
	*pBuf++		= ((iKey >> 16) & 0xFF);
	*pBuf++		= ((iKey >>  8) & 0xFF);
	*pBuf++		= ((iKey >>  0) & 0xFF);

	*pBuf++		= ((iPayloadSize >> 8) & 0xFF);
	*pBuf++		= ((iPayloadSize >> 0) & 0xFF);

	if( pPayload != NULL && iPayloadSize > 0 )
	{
		memcpy( pBuf, pPayload, iPayloadSize );
	}

	return iPayloadSize + MAX_PACKET_KEY_SIZE + MAX_PACKET_LENGTH_SIZE;
}

//------------------------------------------------------------------------------
int32_t NX_IpcParsePacket( void *pInBuf, int32_t iInBufSize, uint32_t *iKey, void **ppPayload, int32_t *iPayloadSize )
{
	if( pInBuf == NULL )
	{
		printf( "Fail, Input Buffer is NULL.\n");
		return -1;
	}

	if( iInBufSize > MAX_PACKET_SIZE )
	{
		printf( "Fail, Input Buffer Size is too large. ( current: %d, max: %d )\n",
			iInBufSize, MAX_PACKET_SIZE );
		return -1;
	}

	uint8_t *pBuf = (uint8_t*)pInBuf;

	*iKey = (pBuf[0] << 24) | (pBuf[1] << 16) | (pBuf[2] <<  8) | (pBuf[3] <<  0);
	pBuf += MAX_PACKET_KEY_SIZE;

	*iPayloadSize = (pBuf[0] << 8) | (pBuf[1] << 0);
	pBuf += MAX_PACKET_LENGTH_SIZE;

	if( *iPayloadSize != iInBufSize - MAX_PACKET_KEY_SIZE - MAX_PACKET_LENGTH_SIZE )
	{
		printf( "Fail, Payload Size is not matched. ( current: %d, expected: %d )\n",
			iInBufSize - MAX_PACKET_KEY_SIZE - MAX_PACKET_LENGTH_SIZE, *iPayloadSize );
		return -1;
	}

	*ppPayload = pBuf;
	return 0;
}

//------------------------------------------------------------------------------
static void DumpHex( const void *pData, int32_t iSize )
{
	int32_t i=0, iOffset = 0;
	char tmp[32];
	static char lineBuf[1024];
	const uint8_t *_data = (const uint8_t*)pData;
	while( iOffset < iSize )
	{
		sprintf( lineBuf, "%08lx :  ", (unsigned long)iOffset );
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

		//     Add ACSII A~Z, & Number & String
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

//------------------------------------------------------------------------------
void NX_IpcDumpPacket( void *pInBuf, int32_t iInBufSize, const char *pFunc, int32_t iLine )
{
	int32_t iRet;

	uint32_t iKey;
	void *pPayload;
	int32_t iPayloadSize;

	iRet = NX_IpcParsePacket( pInBuf, iInBufSize, &iKey, &pPayload, &iPayloadSize );
	if( 0 > iRet )
	{
		printf( "Fail, NX_IpcParsePacket().\n" );
		return;
	}

	printf("================================================================================\n");
	if( pFunc != NULL && iLine != 0 ) printf(" %s(), %d\n", pFunc, iLine );
	printf(" pInBuf( %p ), iInBufSize( %d )\n", pInBuf, iInBufSize );
	printf("--------------------------------------------------------------------------------\n");
	printf("iKey : 0x%08x\n", iKey);
	printf("iPayloadSize : %d \n", iPayloadSize );
	printf("pPayload : \n");
	DumpHex( pPayload, iPayloadSize );
	printf("================================================================================\n");
}
