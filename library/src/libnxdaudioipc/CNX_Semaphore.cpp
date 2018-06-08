//------------------------------------------------------------------------------
//
//	Copyright (C) 2018 Nexell Co. All Rights Reserved
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
#include <time.h>
#include <errno.h>

#include "CNX_Semaphore.h"

//------------------------------------------------------------------------------
CNX_Semaphore::CNX_Semaphore( int32_t iMax, int32_t iInit )
	: m_iValue( iInit )
	, m_iMax( iMax )
	, m_iInit( iInit )
	, m_bReset( false )
{
	pthread_cond_init( &m_hCond, NULL );
	pthread_mutex_init( &m_hLock, NULL );
}

//------------------------------------------------------------------------------
CNX_Semaphore::~CNX_Semaphore()
{

}

//------------------------------------------------------------------------------
void CNX_Semaphore::Init()
{
	pthread_mutex_lock( &m_hLock );
	m_iValue = m_iInit;
	m_bReset = false;
	pthread_mutex_unlock( &m_hLock );
}

//------------------------------------------------------------------------------
void CNX_Semaphore::Deinit()
{
	pthread_mutex_lock( &m_hLock );
	m_bReset = true;
	for( int32_t i = 0; i < m_iMax; i++ )
		pthread_cond_signal( &m_hCond );
	pthread_mutex_unlock( &m_hLock );
}

//------------------------------------------------------------------------------
int32_t CNX_Semaphore::Post()
{
	int32_t iRet = 0;
	pthread_mutex_lock( &m_hLock );
	if( m_iMax <= m_iValue )
	{
		pthread_mutex_unlock( &m_hLock );
		return -1;
	}

	m_iValue++;

	pthread_cond_signal( &m_hCond );
	if( m_bReset || m_iValue <= 0 )
	{
		iRet = -1;
	}
	pthread_mutex_unlock( &m_hLock );
	return iRet;
}

//------------------------------------------------------------------------------
int32_t CNX_Semaphore::Pend( int32_t mSec )
{
	int32_t iRet = 0;
	pthread_mutex_lock( &m_hLock );
	if( m_iValue == 0 && !m_bReset ) {
		int32_t iErr = 0;
		struct timespec	timeSpec;
		do {
			int64_t sec  = (mSec / 1000);
			int64_t nsec = (mSec % 1000) * 1000000;

			clock_gettime( CLOCK_REALTIME, &timeSpec );

			if( 0 < sec )
			{
				timeSpec.tv_sec += sec;
			}

			if( timeSpec.tv_nsec > (1000000000 - nsec) )
			{
				timeSpec.tv_sec  += 1;
				timeSpec.tv_nsec -= (1000000000 - nsec);
			}

			iErr = pthread_cond_timedwait( &m_hCond, &m_hLock, &timeSpec );

			if( iErr == ETIMEDOUT )
			{
				iRet = -ETIMEDOUT;
				break;
			}

			if( m_bReset == true )
			{
				iRet = -1;
				break;
			}
		} while( ETIMEDOUT == iErr );
		m_iValue--;
	}
	else if( m_iValue < 0 || m_bReset )
	{
		iRet = -1;
	}
	else
	{
		m_iValue--;
	}
	pthread_mutex_unlock( &m_hLock );
	return iRet;
}
