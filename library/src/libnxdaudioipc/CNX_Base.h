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

#ifndef __CNX_BASE_H__
#define __CNX_BASE_H__

#include <stdio.h>
#include <pthread.h>

//------------------------------------------------------------------------------
//
//	CNX_Mutex
//
class CNX_Mutex
{
public:
	CNX_Mutex()
	{
		pthread_mutex_init( &m_hLock, NULL );
	}

	~CNX_Mutex()
	{
		pthread_mutex_destroy( &m_hLock );
	}

public:
	void Lock()
	{
		pthread_mutex_lock( &m_hLock );
	}

	void Unlock()
	{
		pthread_mutex_unlock( &m_hLock );
	}

private:
	pthread_mutex_t m_hLock;

private:
	CNX_Mutex (const CNX_Mutex &Ref);
	CNX_Mutex &operator=(const CNX_Mutex &Ref);
};


//------------------------------------------------------------------------------
//
//	CNX_AutoLock
//
class CNX_AutoLock
{
public:
	CNX_AutoLock( CNX_Mutex *pLock )
		: m_pLock( pLock )
	{
		m_pLock->Lock();
	}

	~CNX_AutoLock()
	{
		m_pLock->Unlock();
	}

private:
	CNX_Mutex*	m_pLock;

private:
	CNX_AutoLock (const CNX_AutoLock &Ref);
	CNX_AutoLock &operator=(const CNX_AutoLock &Ref);
};

#endif	// __CNX_BASE_H__
