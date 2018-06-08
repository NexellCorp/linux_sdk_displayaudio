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
#include <pthread.h>
#include <sys/time.h>

#include "CNX_Base.h"
#include "NX_DAudioUtils.h"

//------------------------------------------------------------------------------
class NX_CPostProcess
{
public:
	static NX_CPostProcess* GetInstance();

	int32_t Run( void (*cbFunc)( void* ), int64_t iDelayTime, void *pObj = NULL );
	void	Cancel( void );

private:
	NX_CPostProcess();
	~NX_CPostProcess();

	static void *ThreadStub( void *pObj );
	void		ThreadProc( void );

	int64_t	GetSystemTick( void );

private:
	static NX_CPostProcess* m_pstInstance;

	CNX_Mutex	m_hLock;
	pthread_t	m_hThread;
	int32_t		m_bThreadRun;

	void		(*m_ProcessCallbackFunc)( void* );
	void*		m_pObj;
	int64_t		m_iDelayTime;

private:
	NX_CPostProcess (const NX_CPostProcess &Ref);
	NX_CPostProcess &operator=(const NX_CPostProcess &Ref);
};

NX_CPostProcess* NX_CPostProcess::m_pstInstance = NULL;

//------------------------------------------------------------------------------
NX_CPostProcess::NX_CPostProcess()
	: m_hThread		( 0x00 )
	, m_bThreadRun	( false )
{

}

//------------------------------------------------------------------------------
NX_CPostProcess::~NX_CPostProcess()
{
	if( true == m_bThreadRun )
	{
		m_bThreadRun = false;
		pthread_join( m_hThread, NULL );
		m_hThread = 0x00;
	}
}

//------------------------------------------------------------------------------
int32_t NX_CPostProcess::Run( void (*cbFunc)( void* ), int64_t iDelayTime, void *pObj )
{
	CNX_AutoLock lock( &m_hLock );
	if( true == m_bThreadRun )
	{
		m_bThreadRun = false;
		pthread_join( m_hThread, NULL );
		m_hThread = 0x00;
	}

	m_ProcessCallbackFunc = cbFunc;
	m_iDelayTime = iDelayTime;
	m_pObj       = pObj;

	m_bThreadRun = true;
	if( 0 != pthread_create( &m_hThread, NULL, this->ThreadStub, this ) )
	{
		printf( "Fail, Create Thread.\n" );
		return -1;
	}

	return 0;
}

//------------------------------------------------------------------------------
void NX_CPostProcess::Cancel()
{
	CNX_AutoLock lock( &m_hLock );
	if( true == m_bThreadRun )
	{
		m_bThreadRun = false;
		pthread_join( m_hThread, NULL );
		m_hThread = 0x00;
	}
}

//------------------------------------------------------------------------------
int64_t NX_CPostProcess::GetSystemTick()
{
	struct timeval	tv;
	struct timezone	zv;
	gettimeofday( &tv, &zv );
	return ((int64_t)tv.tv_sec) * 1000 + (int64_t)(tv.tv_usec / 1000);
}

//------------------------------------------------------------------------------
void *NX_CPostProcess::ThreadStub( void *pObj )
{
	if( NULL != pObj )
	{
		((NX_CPostProcess*)pObj)->ThreadProc();
	}

	return (void*)0xDEADDEAD;
}

//------------------------------------------------------------------------------
void NX_CPostProcess::ThreadProc()
{
	int64_t iStartTime = GetSystemTick();

	while( m_bThreadRun )
	{
		if( (iStartTime + m_iDelayTime) < GetSystemTick() )
		{
			if( m_ProcessCallbackFunc )
				m_ProcessCallbackFunc( m_pObj );

			break;
		}

		usleep( 10000 );
	}
}

//------------------------------------------------------------------------------
NX_CPostProcess* NX_CPostProcess::GetInstance()
{
	if( NULL == m_pstInstance )
	{
		m_pstInstance = new NX_CPostProcess();
	}
	return (NX_CPostProcess*)m_pstInstance;
}

//------------------------------------------------------------------------------
int32_t NX_PostProcessRun( void (*cbFunc)( void* ), int64_t iDelayTime, void *pObj )
{
	NX_CPostProcess *pInst = NX_CPostProcess::GetInstance();
	return pInst->Run( cbFunc, iDelayTime, pObj );
}

//------------------------------------------------------------------------------
void NX_PostProcessCancel( void )
{
	NX_CPostProcess *pInst = NX_CPostProcess::GetInstance();
	pInst->Cancel();
}
