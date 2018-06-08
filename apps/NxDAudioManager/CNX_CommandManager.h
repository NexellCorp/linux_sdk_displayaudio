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

#ifndef __CNX_COMMANDMANAGER_H__
#define __CNX_COMMANDMANAGER_H__

#include "CNX_Base.h"

class CNX_CommandManager
	: public CNX_Thread
{
public:
	CNX_CommandManager();
	~CNX_CommandManager();

public:
	void	RegEventCallbackFunc( void (*cbFunc)(uint8_t*, int32_t, void*), void *pObj );

private:
	virtual void ThreadProc();

private:
	void 	(*m_EventCallbackFunc)( uint8_t* pBuf, int32_t iBufSize, void *pObj );
	void*	m_pObj;

private:
	CNX_CommandManager (const CNX_CommandManager &Ref);
	CNX_CommandManager &operator=(const CNX_CommandManager &Ref);
};

#endif	// __CNX_COMMANDMANAGER_H__
