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

#ifndef __CNX_SEMAHPORE_H__
#define __CNX_SEMAHPORE_H__

#include <stdlib.h>
#include <pthread.h>

class CNX_Semaphore
{
public:
	CNX_Semaphore( int32_t iMax, int32_t iInit );
	~CNX_Semaphore();

public:
	void	Init();
	void	Deinit();

	int32_t	Post();
	int32_t	Pend( int32_t mSec = 3000 );

private:
	pthread_cond_t	m_hCond;
	pthread_mutex_t	m_hLock;

	int32_t m_iValue;
	int32_t m_iMax;
	int32_t m_iInit;
	int32_t m_bReset;

private:
	CNX_Semaphore (const CNX_Semaphore &Ref);
	CNX_Semaphore &operator=(const CNX_Semaphore &Ref);
};

#endif	// __CNX_SEMAHPORE_H__
