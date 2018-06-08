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

#ifndef __CNX_POSTPROCESS_H__
#define __CNX_POSTPROCESS_H__

#include <stdint.h>
#include <pthread.h>

class CNX_PostProcess
{
public:
	CNX_PostProcess();
	~CNX_PostProcess();

public:
	int32_t Start( void (*cbFunc)(void*), int64_t iDelayTime, void *pObj = NULL );
	void	Cancel( void );

private:
	static void *ThreadStub( void *pObj );
	void		ThreadProc( void );

private:
	pthread_t	m_hThread;
	int32_t		m_bThreadRun;

	void		(*m_ProcessCallbackFunc)( void * );
	void*		m_pObj;
	int64_t		m_iDelayTime;

private:
	CNX_PostProcess (const CNX_PostProcess &Ref);
	CNX_PostProcess &operator=(const CNX_PostProcess &Ref);
};

#endif	// __CNX_POSTPROCESS_H__
