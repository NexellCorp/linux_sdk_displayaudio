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

#ifndef __CNX_EVENTMANAGER_H__
#define __CNX_EVENTMANAGER_H__

#include "CNX_Base.h"

typedef struct NX_EVENT {
	uint32_t	iCode;
	uint32_t	iValue;
} NX_EVENT;

class CNX_EventManager
{
public:
	static CNX_EventManager* GetInstance();
	static void ReleaseInstance();

public:
	int32_t	PushEvent( NX_EVENT *pEvent );
	void	RegEventCallback( void (*cbFunc)(NX_EVENT *pEvent), void *pObj );

private:
	CNX_EventManager();
	~CNX_EventManager();

	void SendEvent();
	void WaitEvent();
	void ResetEvent();

	int32_t Start();
	int32_t Stop();

	static void* ThreadStub( void *pObj );
	void ThreadProc();

private:
	enum { MAX_EVENT_NUM = 1024 };
	static CNX_EventManager*	m_pstInstance;

	CNX_Mutex		m_hLock;
	pthread_t		m_hThread;
	int32_t			m_bThreadRun;

	pthread_cond_t	m_hCond;
	pthread_mutex_t m_hMutex;

	CNX_Queue*		m_pEventQueue;

	void 	(*m_CallbackFunc)( NX_EVENT *pEvent );
	void*	m_pObj;

private:
	CNX_EventManager (const CNX_EventManager &Ref);
	CNX_EventManager &operator=(const CNX_EventManager &Ref);
};

#endif	// __CNX_EVENTMANAGER_H__
