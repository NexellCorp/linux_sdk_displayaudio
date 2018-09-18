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

#ifndef __CNX_VOLUMEMANAGER_H__
#define __CNX_VOLUMEMANAGER_H__

#include <NX_Type.h>
#include "CNX_Base.h"

#define NX_ENABLE_VOLUME_MANAGER	0

class CNX_VolumeManager
{
public:
	CNX_VolumeManager();
	~CNX_VolumeManager();

public:
	void	SetDeviceReserved( const char **ppDevice, int32_t iDeviceNum );
	void	SetMountPosition( const char **ppMount, int32_t iMountNum );

	int32_t	GetMount( NX_VOLUME_INFO **ppVolume, int32_t *iVolumeNum );
	int32_t IsMounted( char *pDevice );

private:
	int32_t IsDeviceReserved( char *pDevice );
	int32_t IsMountPosition( char *pMount );
	int32_t GetMountType( char *pType );

private:
	enum { MAX_VOLUME_NUM = 16, MAX_VOLUME_STR = 1024 };
	NX_VOLUME_INFO	m_Volume[MAX_VOLUME_NUM];
	NX_VOLUME_INFO	m_CurVolume[MAX_VOLUME_NUM];
	int32_t			m_iCurVolumeNum;

	void			(*m_EventCallbackFunc)( uint32_t iEventType, uint8_t *pDevice, uint8_t *pMountPos, void *pObj );
	void*			m_pObj;

	const char**	m_ppDeviceReserved;
	const char**	m_ppMountPosition;

	int32_t			m_iNumDeviceReserved;
	int32_t			m_iNumMountPosition;

private:
	CNX_VolumeManager (const CNX_VolumeManager &Ref);
	CNX_VolumeManager &operator=(const CNX_VolumeManager &Ref);
};

#endif	// __CNX_VOLUMEMANAGER_H__
