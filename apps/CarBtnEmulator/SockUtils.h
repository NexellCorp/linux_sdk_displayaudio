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

#ifndef __SOCKUTIL_H__
#define __SOCKUTIL_H__

//	Local Socket Util
int32_t LS_Open( const char *pLocalFileName );
int32_t LS_Connect( const char *pLocalFileName );

//	TCP Socket Util
int32_t TCP_Open( short port );
int32_t TCP_Connect( const char *ipAddr, short port );

//	UD Socket Util
int32_t UDP_Open( short port );
int32_t UDP_Connect( void );

#endif	//	__SOCKUTIL_H__
