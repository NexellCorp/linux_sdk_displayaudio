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

#ifndef __NX_ICONFIG_H__
#define __NX_ICONFIG_H__

#include <stdint.h>

//
//	This class models SharedPreference in Android.
//
class NX_IConfig
{
public:
	NX_IConfig() {}
	virtual ~NX_IConfig() {}

public:
	virtual int32_t Open( const char *pFile )				= 0;
	virtual void	Close( void )							= 0;

	virtual int32_t Write( const char *pKey, char *pValue )	= 0;
	virtual int32_t Read( const char *pKey, char **ppValue )= 0;
	virtual int32_t Remove( const char *pKey )				= 0;

	virtual void	Dump( void )							= 0;
	virtual const char* GetVersion( void )					= 0;

private:
	NX_IConfig (const NX_IConfig &Ref);
	NX_IConfig &operator=(const NX_IConfig &Ref);
};

extern NX_IConfig*	GetConfigHandle();

#endif	// __NX_ICONFIG_H__


