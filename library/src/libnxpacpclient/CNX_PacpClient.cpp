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

#include <unistd.h>
#ifndef QT_X11
#include <pacpclient.h>
#else
#include <QWidget>
#endif

#include "CNX_PacpClient.h"

//------------------------------------------------------------------------------
CNX_PacpClient::CNX_PacpClient(QObject *pParent)
	: QObject(pParent)
	, m_pParent( NULL )
	, m_pClient( NULL )
	, m_SockName( "nexell.pacp." + QString::number(getpid()) )
{
}

//------------------------------------------------------------------------------
CNX_PacpClient::~CNX_PacpClient()
{
#ifndef QT_X11
	PACPClient* pClient = (PACPClient*)m_pClient;
	if( pClient ) {
		pClient->deleteLater();
		m_pClient = NULL;
	}
#endif
}

//------------------------------------------------------------------------------
void CNX_PacpClient::SetConfig(QObject *pObj)
{
#ifndef QT_X11
	PACPClient* pClient = new PACPClient(pObj);
	if( pClient ) {
		pClient->setName(m_SockName);
		pClient->connectToServer();

		connect( pClient, SIGNAL(requestShow(QString)), pObj, SLOT(show()) );
		connect( pClient, SIGNAL(requestHide(QString)), pObj, SLOT(hide()) );
		connect( pClient, SIGNAL(requestRaise(QString)), pObj, SLOT(raise()) );
		connect( pClient, SIGNAL(requestLower(QString)), pObj, SLOT(lower()) );

		m_pClient = (void*)pClient;
	}
#else
	m_pParent = pObj;
#endif
}

//------------------------------------------------------------------------------
void CNX_PacpClient::Show()
{
#ifndef QT_X11
	PACPClient* pClient = (PACPClient*)m_pClient;
	if( pClient )
		pClient->requireShow(m_SockName);
#else
	QWidget* pWidget = (QWidget*)m_pParent;
	if( pWidget )
		pWidget->show();
#endif
}

//------------------------------------------------------------------------------
void CNX_PacpClient::Hide()
{
#ifndef QT_X11
	PACPClient* pClient = (PACPClient*)m_pClient;
	if( pClient )
		pClient->requireHide(m_SockName);
#else
	QWidget* pWidget = (QWidget*)m_pParent;
	if( pWidget )
		pWidget->hide();
#endif
}

//------------------------------------------------------------------------------
void CNX_PacpClient::Raise()
{
#ifndef QT_X11
	PACPClient* pClient = (PACPClient*)m_pClient;
	if( pClient )
		pClient->requireRaise(m_SockName);
#else
	QWidget* pWidget = (QWidget*)m_pParent;
	if( pWidget )
		pWidget->raise();
#endif
}

//------------------------------------------------------------------------------
void CNX_PacpClient::Lower()
{
#ifndef QT_X11
	PACPClient* pClient = (PACPClient*)m_pClient;
	if( pClient )
		pClient->requireLower(m_SockName);
#else
	QWidget* pWidget = (QWidget*)m_pParent;
	if( pWidget )
		pWidget->lower();
#endif
}

//------------------------------------------------------------------------------
CNX_PacpClient* CNX_PacpClient::m_psInstance = NULL;

CNX_PacpClient* CNX_PacpClient::GetInstance()
{
	if( NULL == m_psInstance )
	{
		m_psInstance = new CNX_PacpClient();
	}
	return m_psInstance;
}
