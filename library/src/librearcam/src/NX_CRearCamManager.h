//------------------------------------------------------------------------------
//
//	Copyright (C) 2016 Nexell Co. All Rights Reserved
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

#ifndef __NX_CREARCAMERAMANAGER_H__
#define __NX_CREARCAMERAMANAGER_H__

#ifdef __cplusplus

#include <stdint.h>
#include <NX_CV4l2VipFilter.h>
#include <NX_CVideoRenderFilter.h>
#include <NX_CDeinterlaceFilter.h>
#include <NX_RearCam.h>

#ifdef ANDROID_SURF_RENDERING
#include <NX_CAndroidRenderer.h>
#endif

enum {
	REAR_CAM_STATUS_STOP = 0,
	REAR_CAM_STATUS_INIT ,
	REAR_CAM_STATUS_RUNNING,
	RAER_CAM_STATUS_MAX
};

class NX_CRearCamManager
{
public:
	static NX_CRearCamManager* GetManager( void );
	static void ReleaseManager( void );

public:
	virtual int32_t Init( NX_REARCAM_INFO *pInfo, DISPLAY_INFO* p_dspInfo , DEINTERLACE_INFO *pDeinterInfo);
	virtual int32_t Deinit( void );
	virtual int32_t Start( void );
	virtual int32_t Stop( void );
	virtual int32_t Pause( int32_t mbPause );
	virtual int32_t GetStatus();
	virtual int32_t SetDisplayPosition(int32_t x, int32_t y, int32_t w, int32_t h);

	virtual int32_t RegNotifyCallback( void (*cbFunc)(uint32_t, void*, uint32_t) );

	virtual int32_t ChangeDebugLevel( int32_t iLevel );

public:
	virtual void 	ProcessEvent( uint32_t iEventCode, void *pData, uint32_t iDataSize );


private:
	NX_CRearCamManager();
	~NX_CRearCamManager();

	static int32_t CaptureFileName( uint8_t *pBuf, uint32_t iBufSize );

private:
	static NX_CRearCamManager* m_pManager;

	int32_t mRearCamStatus;

	NX_CRefClock*			m_pRefClock;
	NX_CEventNotifier*		m_pEventNotifier;

	NX_CV4l2VipFilter*		m_pV4l2VipFilter;
	NX_CDeinterlaceFilter*  m_pDeinterlaceFilter;
	NX_CVideoRenderFilter*	m_pVideoRenderFilter;

#ifdef ANDROID_SURF_RENDERING
	NX_CAndroidRenderer* 	m_pAndroidRenderer;
#endif

	NX_CMutex		m_hLock;

private:
	void (*NotifyCallbackFunc)( uint32_t iEventCode, void *pData, uint32_t iDataSize );

private:
	NX_CRearCamManager (const NX_CRearCamManager &Ref);
	NX_CRearCamManager &operator=(const NX_CRearCamManager &Ref);
};

#endif	// __cplusplus

#endif	// __NX_CREARCAMERAMANAGER_H__
