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

#ifndef __CNX_PROCESSMANAGER_H__
#define __CNX_PROCESSMANAGER_H__

#include "NX_Type.h"
#include "CNX_Base.h"

class CNX_ProcessManager
	: public CNX_Thread
{
public:
	CNX_ProcessManager();
	~CNX_ProcessManager();

public:
	int32_t Add( NX_PROCESS_INFO *pInfo );
	int32_t Remove( NX_PROCESS_INFO *pInfo );

	int32_t GetCount();
	int32_t GetAt( int32_t iIndex, NX_PROCESS_INFO **ppInfo );
	int32_t IndexOf( NX_PROCESS_INFO *pInfo );
	int32_t IndexOf( int32_t iFlags );
	int32_t Swap( int32_t iIndex1, int32_t iIndex2 );

	int32_t Request( int32_t (*cbFunc)( int32_t iIndex, NX_PROCESS_INFO*, NX_PROCESS_INFO*, void* ), NX_PROCESS_INFO* pReqInfo, void *pObj );
	int32_t Request( int32_t (*cbFunc)( int32_t iIndex, NX_PROCESS_INFO*, void*, int32_t, void* ), void* pPayload, int32_t iPayloadSize, void *pObj );
	int32_t Request( int32_t (*cbFunc)( NX_PROCESS_INFO*, void*, int32_t, void* ), NX_PROCESS_INFO* pReqInfo, void *pPayload, int32_t iPayloadSize, void *pObj );

	void	RegTerminateCallback( void (*cbTerminateFunc)( void* ), void *pObj );

	void	SetProcessInfoToPayload( void **ppPayload, int32_t *iPayloadSize );
	int32_t PrintInfo();

private:
	virtual void ThreadProc();

	int32_t	RemoveInvalidProcess();
	int32_t	TerminateProc();

private:
	CNX_Mutex			m_hLock;
	CNX_LinkedList*		m_pList;

	void (*m_cbTerminateFunc)( void *pObj );
	void *m_pObj;

private:
	CNX_ProcessManager (const CNX_ProcessManager &Ref);
	CNX_ProcessManager &operator=(const CNX_ProcessManager &Ref);
};

#endif	// __CNX_PROCESSMANAGER_H__
