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

#ifndef __CNX_PACPCLIENT_H__
#define __CNX_PACPCLIENT_H__

#include <QObject>
#include <QString>

class CNX_PacpClient : public QObject
{
    Q_OBJECT

public:
	static CNX_PacpClient* GetInstance();

private:
    explicit CNX_PacpClient(QObject *pParent = Q_NULLPTR);
    ~CNX_PacpClient();

public:
	void SetConfig( QObject *pObj );
	void Show();
	void Hide();
	void Raise();
	void Lower();

private:
	QObject*	m_pParent;
	void* 		m_pClient;
	QString		m_SockName;

	static CNX_PacpClient* m_psInstance;
};

#endif	// __CNX_PACPCLIENT_H__
