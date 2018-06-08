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

#include <stdio.h>
#include <unistd.h>
#include "NX_Utils.h"
#include "CNX_PostProcess.h"

#define NX_DTAG "[CNX_PostProcess]"
#include "NX_DbgMsg.h"

#define NX_ENABLE_TICK		0

//------------------------------------------------------------------------------
CNX_PostProcess::CNX_PostProcess()
	: m_hThread		( 0x00 )
	, m_bThreadRun	( false )
{

}

//------------------------------------------------------------------------------
CNX_PostProcess::~CNX_PostProcess()
{
	if( true == m_bThreadRun )
	{
		m_bThreadRun = false;
		pthread_join( m_hThread, NULL );
		m_hThread = 0x00;
	}
}

//------------------------------------------------------------------------------
int32_t CNX_PostProcess::Start( void (*cbFunc)(void*), int64_t iDelayTime, void *pObj )
{
	if( true == m_bThreadRun )
	{
		m_bThreadRun = false;
		pthread_join( m_hThread, NULL );
		m_hThread = 0x00;
	}

	m_ProcessCallbackFunc = cbFunc;
	m_iDelayTime = iDelayTime;
	m_pObj = pObj;

	m_bThreadRun = true;
	if( 0 != pthread_create( &m_hThread, NULL, this->ThreadStub, this ) )
	{
		NX_DbgMsg( NX_DBG_ERR, "Fail, Create Thread.\n" );
		return -1;
	}

	return 0;
}

//------------------------------------------------------------------------------
void CNX_PostProcess::Cancel()
{
	if( true == m_bThreadRun )
	{
		m_bThreadRun = false;
		pthread_join( m_hThread, NULL );
		m_hThread = 0x00;
	}
}

//------------------------------------------------------------------------------
void *CNX_PostProcess::ThreadStub( void *pObj )
{
	if( NULL != pObj )
	{
		((CNX_PostProcess*)pObj)->ThreadProc();
	}

	return (void*)0xDEADDEAD;
}

//------------------------------------------------------------------------------
void CNX_PostProcess::ThreadProc()
{
#if NX_ENABLE_TICK
	int64_t iStartTime = NX_GetSystemTick();
#endif

	while( m_bThreadRun )
	{
#if NX_ENABLE_TICK
		if( (iStartTime + m_iDelayTime) < NX_GetSystemTick() )
		{
			if( m_ProcessCallbackFunc )
				m_ProcessCallbackFunc( m_pObj );

			break;
		}

		usleep( 10000 );
#else
		if( 0 >= m_iDelayTime )
		{
			if( m_ProcessCallbackFunc )
				m_ProcessCallbackFunc( m_pObj );

			break;
		}

		usleep( 1000 );
		m_iDelayTime--;
#endif
	}
}
