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

#ifndef __NX_UTILS_H__
#define __NX_UTILS_H__



//
//	C++ Utils
//
#ifdef __cplusplus

#include <pthread.h>
class NX_CAutoLock {
public:
	NX_CAutoLock( pthread_mutex_t *pLock )
		: m_pLock(pLock)
	{
		pthread_mutex_lock( m_pLock );
	}
	~NX_CAutoLock()
	{
		pthread_mutex_unlock( m_pLock );
	}

protected:
	pthread_mutex_t *m_pLock;

private:
	NX_CAutoLock (const NX_CAutoLock &Ref);
	NX_CAutoLock &operator=(NX_CAutoLock &Ref);
};

#endif

#endif	//	__NX_UTILS_H__
