//------------------------------------------------------------------------------
//
//	Copyright (C) 2016 Nexell Co. All Rights Reserved
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
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <errno.h>	//	errno

//
//	Server Part APIs
//
int32_t LS_Open( const char *pLocalFileName )
{
	int32_t svrSock = -1;
	struct sockaddr_un serverAddr;

	if ( 0 == access( pLocalFileName, F_OK ) )
		unlink( pLocalFileName);

	svrSock  = socket( PF_LOCAL, SOCK_STREAM, 0 );
	if( svrSock < 0 )
	{
		printf( "Error : server socket \n");
		goto ErrorExit;
	}

	memset( &serverAddr, 0, sizeof( serverAddr ) );
	serverAddr.sun_family  = AF_LOCAL;
	strcpy( serverAddr.sun_path, pLocalFileName );

	if( -1 == bind( svrSock, (struct sockaddr*)&serverAddr, sizeof( serverAddr ) ) )
	{
		printf( "Error : bind()\n");
		goto ErrorExit;
	}

	chmod( pLocalFileName, 0777 );
	return svrSock;

ErrorExit:
	if( svrSock > 0 )
	{
		close( svrSock );
	}
	return -1;
}



//
//	Client Part APIs
//
int32_t LS_Connect( const char *pLocalFileName )
{
	int   clntSock;
	struct sockaddr_un   svrAddr;

	clntSock  = socket( PF_LOCAL, SOCK_STREAM, 0);
	if( -1 == clntSock)
	{
		return -1;
	}

	memset( &svrAddr, 0, sizeof( svrAddr) );
	svrAddr.sun_family  = AF_LOCAL;
	strcpy( svrAddr.sun_path, pLocalFileName );

	if( -1 == connect( clntSock, (struct sockaddr*)&svrAddr, sizeof(svrAddr) ) )
	{
		printf( "Error : connect (err=%d)\n", errno);
		close( clntSock );
		return -1;
	}
	else
	{
		return clntSock;
	}
}


//////////////////////////////////////////////////////////////////////////////
//																			//
//						TCP Open / Connect Function							//
//																			//
//////////////////////////////////////////////////////////////////////////////

//
//	Server Part APIs
//
int32_t TCP_Open( short port )
{
	int32_t svrSock = -1;
	int32_t yes=1;
	struct sockaddr_in svrAddr;

	svrSock  = socket( AF_INET, SOCK_STREAM, 0 );
	if( svrSock < 0 )
	{
		printf( "Error : server socket \n");
		goto ErrorExit;
	}

	memset( &svrAddr, 0, sizeof( svrAddr ) );
	svrAddr.sin_family = AF_INET;
	svrAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	svrAddr.sin_port = htons( port );

	if( setsockopt(svrSock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1 )
	{
		printf( "Error : setsockopt()\n");
		goto ErrorExit;
	}
	if( -1 == bind( svrSock, (struct sockaddr*)&svrAddr, sizeof( svrAddr ) ) )
	{
		printf( "Error : bind()\n");
		goto ErrorExit;
	}
	return svrSock;

ErrorExit:
	if( svrSock > 0 )
	{
		close( svrSock );
	}
	return -1;
}


//
//	Client Part APIs
//
int32_t TCP_Connect( const char *ipAddr, short port )
{
	int32_t clntSock;
	struct sockaddr_in svrAddr;

	clntSock  = socket( AF_INET, SOCK_STREAM, 0);
	if( -1 == clntSock)
	{
		return -1;
	}

	memset( &svrAddr, 0, sizeof( svrAddr) );
	svrAddr.sin_family = AF_INET;
	svrAddr.sin_addr.s_addr = inet_addr(ipAddr);
	svrAddr.sin_port = htons( port );

	if( -1 == connect( clntSock, (struct sockaddr*)&svrAddr, sizeof(svrAddr) ) )
	{
		printf( "Error : connect (err=%d)\n", errno);
		close( clntSock );
		return -1;
	}
	else
	{
		return clntSock;
	}
}


//////////////////////////////////////////////////////////////////////////////
//																			//
//						UDP Open / Connect Function							//
//																			//
//////////////////////////////////////////////////////////////////////////////

//
//	UDP Server Part APIs
//
int32_t UDP_Open( short port )
{
	int32_t svrSock = -1;
	int32_t yes=1;
	struct sockaddr_in svrAddr;

	svrSock  = socket( AF_INET, SOCK_DGRAM, 0 );
	if( svrSock < 0 )
	{
		printf( "Error : server socket \n");
		goto ErrorExit;
	}

	memset( &svrAddr, 0, sizeof( svrAddr ) );
	svrAddr.sin_family = AF_INET;
	svrAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	svrAddr.sin_port = htons( port );

	if( setsockopt(svrSock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int32_t)) == -1 )
	{
		printf( "Error : setsockopt()\n");
		goto ErrorExit;
	}
	if( -1 == bind( svrSock, (struct sockaddr*)&svrAddr, sizeof( svrAddr ) ) )
	{
		printf( "Error : bind()\n");
		goto ErrorExit;
	}
	return svrSock;

ErrorExit:
	if( svrSock > 0 )
	{
		close( svrSock );
	}
	return -1;
}

//
//	Client Part APIs
//
int32_t UDP_Connect( void )
{
	int32_t clntSock;
	clntSock  = socket( AF_INET, SOCK_DGRAM, 0);
	if( -1 == clntSock)
	{
		return -1;
	}
	return clntSock;
}
