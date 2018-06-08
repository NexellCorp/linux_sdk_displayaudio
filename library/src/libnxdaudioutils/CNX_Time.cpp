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
#include <sys/time.h>

#include "CNX_Base.h"
#include "NX_DAudioUtils.h"

//------------------------------------------------------------------------------
class CNX_Time
{
public:
	static CNX_Time* GetInstance();

	void	Init( void );
	int32_t IsRepeat( int64_t iRepeatTime );

private:
	CNX_Time();
	~CNX_Time();

	int64_t	GetSystemTick( void );

private:
	static CNX_Time* m_pstInstance;

	CNX_Mutex m_hLock;
	int64_t	m_iPrevTime;

private:
	CNX_Time (const CNX_Time &Ref);
	CNX_Time &operator=(const CNX_Time &Ref);
};

CNX_Time* CNX_Time::m_pstInstance = NULL;

//------------------------------------------------------------------------------
CNX_Time::CNX_Time()
{

}

//------------------------------------------------------------------------------
CNX_Time::~CNX_Time()
{

}

//------------------------------------------------------------------------------
void CNX_Time::Init( void )
{
	CNX_AutoLock lock( &m_hLock );
	m_iPrevTime = GetSystemTick();
}

//------------------------------------------------------------------------------
int32_t CNX_Time::IsRepeat( int64_t iRepeatTime )
{
	CNX_AutoLock lock( &m_hLock );
	int64_t iCurTime = GetSystemTick();
	if( (m_iPrevTime + iRepeatTime) < iCurTime )
	{
		m_iPrevTime = iCurTime;
		return false;
	}
	return true;
}

//------------------------------------------------------------------------------
int64_t CNX_Time::GetSystemTick()
{
	struct timeval	tv;
	struct timezone	zv;
	gettimeofday( &tv, &zv );
	return ((int64_t)tv.tv_sec) * 1000 + (int64_t)(tv.tv_usec / 1000);
}

//------------------------------------------------------------------------------
CNX_Time* CNX_Time::GetInstance()
{
	if( NULL == m_pstInstance )
	{
		m_pstInstance = new CNX_Time();
	}
	return (CNX_Time*)m_pstInstance;
}

//------------------------------------------------------------------------------
void NX_TimeInit( void )
{
	CNX_Time *pInst = CNX_Time::GetInstance();
	pInst->Init();
}

//------------------------------------------------------------------------------
int32_t NX_TimeIsRepeat( int64_t iRepeatTime )
{
	CNX_Time *pInst = CNX_Time::GetInstance();
	return pInst->IsRepeat( iRepeatTime );
}
