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
#include <unistd.h>

#include <CNX_Socket.h>

//------------------------------------------------------------------------------
int32_t main( int32_t argc, char *argv[] )
{
	if( argc != 2 )
	{
		printf("Usage: %s [PAYLOAD]\n", argv[0]);
		return -1;
	}

	int32_t iSock = -1;
	CNX_Socket *pSocket = new CNX_Socket();

	iSock = pSocket->Connect( "daudio.command" );
	pSocket->Write( iSock, (uint8_t*)argv[1], strlen(argv[1]) );
	close( iSock );

	delete pSocket;
	return 0;
}
