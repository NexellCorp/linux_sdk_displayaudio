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

#ifndef __CNX_UEVENTMANAGER_H__
#define __CNX_UEVENTMANAGER_H__

#include "CNX_Base.h"

class CNX_UeventManager
	: public CNX_Thread
{
public:
	CNX_UeventManager();
	~CNX_UeventManager();

public:
	void	RegEventCallbackFunc( void (*cbFunc)(uint8_t*, uint32_t, void *), void *pObj );

private:
	virtual void ThreadProc();

private:
	void 	(*m_EventCallbackFunc)( uint8_t *pDesc, uint32_t iDescSize, void *pObj );
	void*	m_pObj;

private:
	CNX_UeventManager (const CNX_UeventManager &Ref);
	CNX_UeventManager &operator=(const CNX_UeventManager &Ref);
};

#endif	// __CNX_UEVENTMANAGER_H__
