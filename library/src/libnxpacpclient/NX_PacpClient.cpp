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

#include "CNX_PacpClient.h"

//------------------------------------------------------------------------------
void NX_PacpClientStart( void *pObj )
{
	CNX_PacpClient *pClient = CNX_PacpClient::GetInstance();
	if( pClient )
		pClient->SetConfig( (QObject*)pObj );
}

//------------------------------------------------------------------------------
void NX_PacpClientStop()
{
	CNX_PacpClient *pClient = CNX_PacpClient::GetInstance();
	if( pClient )
		pClient->deleteLater();
}

//------------------------------------------------------------------------------
void NX_PacpClientRequestShow()
{
	CNX_PacpClient *pClient = CNX_PacpClient::GetInstance();
	if( pClient )
		pClient->Show();
}

//------------------------------------------------------------------------------
void NX_PacpClientRequestHide()
{
	CNX_PacpClient *pClient = CNX_PacpClient::GetInstance();
	if( pClient )
		pClient->Hide();
}

//------------------------------------------------------------------------------
void NX_PacpClientRequestRaise()
{
	CNX_PacpClient *pClient = CNX_PacpClient::GetInstance();
	if( pClient )
		pClient->Raise();
}

//------------------------------------------------------------------------------
void NX_PacpClientRequestLower()
{
	CNX_PacpClient *pClient = CNX_PacpClient::GetInstance();
	if( pClient )
		pClient->Lower();
}

//------------------------------------------------------------------------------
#include <NX_Type.h>

const char* NX_PacpClientGetVersion()
{
	return NX_VERSION_LIBPACPCLIENT;
}
