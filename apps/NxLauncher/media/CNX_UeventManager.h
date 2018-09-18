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

#ifndef __CNX_UEVENTMANAGER_H__
#define __CNX_UEVENTMANAGER_H__

#include <QThread>

class CNX_UeventManager : public QThread
{
	Q_OBJECT

signals:
	void signalDetectUevent(uint8_t* pDesc, uint32_t uiDescSize);

public:
	CNX_UeventManager();
	~CNX_UeventManager();

	void Start();

	void Stop();

protected:
	virtual void run();

private:
	bool m_bThreadRun;
};

#endif	// __CNX_UEVENTMANAGER_H__
