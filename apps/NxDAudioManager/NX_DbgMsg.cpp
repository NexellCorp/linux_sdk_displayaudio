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
#include <ctype.h>

#define NX_DTAG	"[NX_DbgMsg]"
#include "NX_DbgMsg.h"

#ifdef NX_ENABLE_SYSLOG
uint32_t gNxDebugLevel = NX_DBG_VBS;
#else
uint32_t gNxDebugLevel = NX_DBG_ERR;
#endif

//------------------------------------------------------------------------------
void NX_ChangeDebugLevel( uint32_t iLevel )
{
	NX_DbgMsg( NX_DBG_VBS, "%s : Change debug level %d to %d.\n", __FUNCTION__, gNxDebugLevel, iLevel );
	gNxDebugLevel = iLevel;
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
