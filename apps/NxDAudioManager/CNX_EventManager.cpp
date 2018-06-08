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

#include "CNX_EventManager.h"

//------------------------------------------------------------------------------
CNX_EventManager::CNX_EventManager()
	: m_hThread( 0x00 )
	, m_bThreadRun( false )
	, m_CallbackFunc( NULL )
	, m_pObj( NULL )
{
	m_pEventQueue = new CNX_Queue( MAX_EVENT_NUM );

	pthread_cond_init( &m_hCond, NULL );
	pthread_mutex_init( &m_hMutex, NULL );
}

//------------------------------------------------------------------------------
CNX_EventManager::~CNX_EventManager()
{
	pthread_cond_destroy( &m_hCond );
	pthread_mutex_destroy( &m_hMutex );

	if( m_pEventQueue )
	{
		delete m_pEventQueue;
		m_pEventQueue = NULL;
	}
}

//------------------------------------------------------------------------------
int32_t CNX_EventManager::PushEvent( NX_EVENT *pEvent )
{
	CNX_AutoLock lock( &m_hLock );
	NX_EVENT *pTemp = (NX_EVENT*)malloc( sizeof(NX_EVENT) );
	if( NULL == pTemp )
		return -1;

	*pTemp = *pEvent;
	m_pEventQueue->Push( (void*)pTemp );

	SendEvent();
	return 0;
}

//------------------------------------------------------------------------------
void CNX_EventManager::RegEventCallback( void (*cbFunc)(NX_EVENT *pEvent), void *pObj )
{
	if( cbFunc )
	{
		m_CallbackFunc = cbFunc;
		m_pObj = pObj;
	}
}

//------------------------------------------------------------------------------
void CNX_EventManager::SendEvent()
{
	pthread_mutex_lock( &m_hMutex );
	pthread_cond_signal( &m_hCond );
	pthread_mutex_unlock( &m_hMutex );
}

//------------------------------------------------------------------------------
void CNX_EventManager::WaitEvent()
{
	pthread_mutex_lock( &m_hMutex );
	pthread_cond_wait( &m_hCond, &m_hMutex );
	pthread_mutex_unlock( &m_hMutex );
}

//------------------------------------------------------------------------------
void CNX_EventManager::ResetEvent()
{
	pthread_mutex_lock( &m_hMutex );
	pthread_cond_broadcast( &m_hCond );
	pthread_mutex_unlock( &m_hMutex );
}

//------------------------------------------------------------------------------
int32_t CNX_EventManager::Start()
{
	if( m_bThreadRun == false )
	{
		m_bThreadRun = true;
		if( 0 != pthread_create( &m_hThread, NULL, CNX_EventManager::ThreadStub, this) )
			return -1;
	}
	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_EventManager::Stop()
{
	if( m_bThreadRun == true )
	{
		m_bThreadRun = false;
		ResetEvent();
		pthread_join( m_hThread, NULL );
		m_hThread = 0x00;
	}
	return 0;
}

//------------------------------------------------------------------------------
void* CNX_EventManager::ThreadStub( void *pObj )
{
	if( NULL != pObj ) {
		((CNX_EventManager*)pObj)->ThreadProc();
	}
	return (void*)0xDEADDEAD;
}

//------------------------------------------------------------------------------
void CNX_EventManager::ThreadProc()
{
	NX_EVENT *pEvent = NULL;

	while( m_bThreadRun )
	{
		WaitEvent();
		if( 0 > m_pEventQueue->Pop( (void**)&pEvent ) )
			continue;

		if( pEvent == NULL )
			continue;

		if( m_CallbackFunc )
			m_CallbackFunc( pEvent );

		free( pEvent );
	}

	while( 1 )
	{
		if( 0 > m_pEventQueue->Pop( (void**)&pEvent ) )
			break;

		free( pEvent );
	}
}

//------------------------------------------------------------------------------
CNX_EventManager* CNX_EventManager::m_pstInstance = NULL;

CNX_EventManager* CNX_EventManager::GetInstance()
{
	if( m_pstInstance == NULL )
	{
		m_pstInstance = new CNX_EventManager();
		if( 0 > m_pstInstance->Start() )
		{
			delete m_pstInstance;
			m_pstInstance = NULL;
		}
	}
	return m_pstInstance;
}

//------------------------------------------------------------------------------
void CNX_EventManager::ReleaseInstance()
{
	if( m_pstInstance )
	{
		m_pstInstance->Stop();
		delete m_pstInstance;
		m_pstInstance = NULL;
	}
}
