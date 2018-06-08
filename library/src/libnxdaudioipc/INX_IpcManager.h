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

#ifndef __INX_IPCMANAGER_H__
#define __INX_IPCMANAGER_H__

#include <stdint.h>

class INX_IpcManager
{
public:
	INX_IpcManager() {}
	virtual ~INX_IpcManager() {}

public:
	//
	//	IPC Common Interface
	//
	virtual int32_t Write( int32_t iSock, uint8_t *pBuf, int32_t iSize ) = 0;
	virtual int32_t Read( int32_t iSock, uint8_t *pBuf, int32_t iSize ) = 0;

	virtual int32_t ReplyWait() = 0;
	virtual int32_t ReplyDone() = 0;

	virtual const char* GetVersion() = 0;

	//
	//	IPC Server Interface
	//
	virtual int32_t StartServer( const char *pSock ) = 0;
	virtual int32_t StopServer() = 0;
	virtual void	RegServerCallbackFunc( int32_t (*cbFunc)( int32_t iSock, uint8_t *pSendBuf, uint8_t*pRecvBuf, int32_t iMaxBufSize, void *pObj ), void *pObj ) = 0;

	//
	//	IPC Client Interface
	//
	virtual int32_t SendCommand( const char *pSock, uint8_t *pSendBuf, int32_t iSendSize, uint8_t *pRecvBuf, int32_t iRecvMaxSize ) = 0;
	virtual int32_t SendCommand( const char *pSock, int32_t (*cbFunc)( int32_t iSock, uint8_t *pSendBuf, uint8_t *pRecvBuf, int32_t iMaxBufSize, void *pObj ), void *pObj ) = 0;

private:
	INX_IpcManager (const INX_IpcManager &Ref);
	INX_IpcManager &operator=(const INX_IpcManager &Ref);
};

extern INX_IpcManager*	GetIpcManagerHandle();		// Singleton Instance
extern INX_IpcManager*	GetIpcManager();

#endif	// __INX_IPCMANAGER_H__
