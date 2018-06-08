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

#ifndef __NX_PACPCLIENT_H__
#define __NX_PACPCLIENT_H__

#include <stdint.h>

void NX_PacpClientStart( void *pObj );
void NX_PacpClientStop();
void NX_PacpClientRequestRaise();
void NX_PacpClientRequestLower();

const char* NX_PacpClientGetVersion();

#endif	// __NX_PACPCLIENT_H__
