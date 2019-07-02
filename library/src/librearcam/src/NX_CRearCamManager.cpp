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

#include "NX_CRearCamManager.h"
#include <drm_fourcc.h>
#define virtual vir
#include <xf86drm.h>
#include <xf86drmMode.h>
#undef virtual

#ifdef NX_DTAG
#undef NX_DTAG
#endif
#define NX_DTAG "[NX_CRearCamManager]"
#include <NX_DbgMsg.h>

#define VER_MAJOR   0
#define VER_MINOR   1
#define VER_REVISION    0
#define VER_RESERVATION 0

static int32_t find_video_plane( int fd, int crtcIdx, uint32_t *connId, uint32_t *crtcId, uint32_t *planeId )
{
    uint32_t possible_crtcs = 0;
    drmModeRes *res;
    drmModePlaneRes *pr;
    drmModePlane *plane;
    uint32_t i, j;
    int32_t found = 0;

    res = drmModeGetResources(fd);

    if( crtcIdx >= res->count_crtcs )
        goto ErrorExit;

    *crtcId = res->crtcs[ crtcIdx ];
    *connId = res->connectors[ crtcIdx ];

    possible_crtcs = 1<<crtcIdx;

    drmSetClientCap(fd, DRM_CLIENT_CAP_UNIVERSAL_PLANES, 1);
    pr = drmModeGetPlaneResources( fd );

    for( i=0 ; i<pr->count_planes ; ++i )
    {
        plane = drmModeGetPlane( fd, pr->planes[i] );
        if( plane->possible_crtcs & possible_crtcs )
        {
            for( j=0 ; j<plane->count_formats ; j++ )
            {
                if( plane->formats[j]==DRM_FORMAT_YUV420 ||
                    plane->formats[j]==DRM_FORMAT_YVU420 ||
                    plane->formats[j]==DRM_FORMAT_UYVY ||
                    plane->formats[j]==DRM_FORMAT_VYUY ||
                    plane->formats[j]==DRM_FORMAT_YVYU ||
                    plane->formats[j]==DRM_FORMAT_YUYV )
                {
                    found = 1;
                    *planeId = plane->plane_id;
                }
            }
        }
    }
    drmModeFreeResources(res);
    return found?0:-1;
ErrorExit:
    drmModeFreeResources(res);
    return -1;
}

static int32_t find_rgb_plane( int fd, int32_t crtcIdx, int32_t layerIdx, uint32_t *connId, uint32_t *crtcId, uint32_t *planeId )
{
    uint32_t possible_crtcs = 0;
    drmModeRes *res;
    drmModePlaneRes *pr;
    drmModePlane *plane;
    uint32_t i, j;
    int32_t found = 0;
    int32_t findIdx = 0;
    int32_t isRgb = 0;

    res = drmModeGetResources(fd);
    if( crtcIdx >= res->count_crtcs )
        goto ErrorExit;

    *crtcId = res->crtcs[ crtcIdx ];
    *connId = res->connectors[ crtcIdx ];

    possible_crtcs = 1<<crtcIdx;

    drmSetClientCap(fd, DRM_CLIENT_CAP_UNIVERSAL_PLANES, 1);
    pr = drmModeGetPlaneResources( fd );

    for( i=0 ; i<pr->count_planes ; i++ )
    {
        plane = drmModeGetPlane( fd, pr->planes[i] );
        if( plane->possible_crtcs & possible_crtcs )
        {
            isRgb = 0;
            for( j=0 ; j<plane->count_formats ; j++ )
            {
                if( plane->formats[j]==DRM_FORMAT_ABGR8888 ||
                    plane->formats[j]==DRM_FORMAT_RGBA8888 ||
                    plane->formats[j]==DRM_FORMAT_XBGR8888 ||
                    plane->formats[j]==DRM_FORMAT_RGBX8888 ||
                    plane->formats[j]==DRM_FORMAT_RGB888 ||
                    plane->formats[j]==DRM_FORMAT_BGR888 )
                {
                    isRgb = 1;
                }
            }

            if( isRgb )
            {
                if( findIdx == layerIdx )
                {
                    found = 1;
                    *planeId = plane->plane_id;
                    break;
                }
                findIdx++;
            }
        }
    }
    drmModeFreeResources(res);
    return found?0:-1;
ErrorExit:
    drmModeFreeResources(res);
    return -1;
}


static int32_t find_plane_for_display(int32_t crtcIdx,
                                   int32_t findRgb,
                                   int32_t layerIdx,
                                   NX_DISPLAY_INFO *pdsp_info)
{
    int32_t hDrmFd = -1;
    uint32_t connId = 0;
    uint32_t crtcId = 0;
    uint32_t planeId = 0;

    hDrmFd = drmOpen( "nexell", NULL );

    if( findRgb )
    {
        if( 0 == find_rgb_plane(hDrmFd, crtcIdx, layerIdx, &connId, &crtcId, &planeId) )
        {
			NxDbgMsg( NX_DBG_INFO, "RGB : connId = %d, crtcId = %d, planeId = %d\n", connId, crtcId, planeId);
        }
        else
        {
            printf("cannot found video format for %dth crtc\n", crtcIdx );
        }
    }
    else
    {
        if( 0 == find_video_plane(hDrmFd, crtcIdx, &connId, &crtcId, &planeId) )
        {
            NxDbgMsg( NX_DBG_INFO, "VIDEO : connId = %d, crtcId = %d, planeId = %d\n", connId, crtcId, planeId);
        }
        else
        {
            NxDbgMsg( NX_DBG_ERR, "cannot found video format for %dth crtc\n", crtcIdx );
        }
    }
    drmClose( hDrmFd );

	printf("found crtcId( %d ), planeId( %d )\n",  crtcId, planeId);

    pdsp_info->connectorId     = connId;
    pdsp_info->ctrlId        	= crtcId;
    //pdsp_info->iPlaneId        = planeId;
	return planeId;
}

//------------------------------------------------------------------------------
NX_CRearCamManager::NX_CRearCamManager()
	: m_pEventNotifier		( NULL )
	, m_pV4l2VipFilter		( NULL )
	, m_pDeinterlaceFilter	( NULL )
	, m_pVideoRenderFilter	( NULL )
	, NotifyCallbackFunc	( NULL )
#ifdef ANDROID_SURF_RENDERING
	, m_pAndroidRenderer	( NULL )
#endif
	, mRearCamStatus		(0)
{
}

//------------------------------------------------------------------------------
NX_CRearCamManager::~NX_CRearCamManager()
{
	Stop();
	Deinit();
}

//------------------------------------------------------------------------------
int32_t NX_CRearCamManager::Init( NX_REARCAM_INFO *pInfo , DISPLAY_INFO *pDspInfo, DEINTERLACE_INFO *pDeinterInfo)
{
	NxDbgMsg( NX_DBG_INFO, "%s()++\n", __FUNCTION__ );
	NX_CAutoLock lock( &m_hLock );

	NX_MEDIA_INFO videoInfo;
	NX_DISPLAY_INFO dspInfo;
	NX_DEINTERLACE_INFO deinterInfo;


	memset( &videoInfo, 0x00, sizeof(videoInfo) );
	memset( &dspInfo, 0x00, sizeof(dspInfo));
	if( pInfo->iType == CAM_TYPE_VIP )
	{
		videoInfo.iMediaType		= NX_TYPE_VIDEO;
		videoInfo.iContainerType	= NX_CONTAINER_MP4;
		videoInfo.iModule			= pInfo->iModule;
		videoInfo.iSensorId			= pInfo->iSensor;
		videoInfo.bInterlace		= pInfo->bUseInterCam;
		videoInfo.iWidth			= pInfo->iWidth;
		videoInfo.iHeight			= pInfo->iHeight;
		videoInfo.iFpsNum			= 30;
		videoInfo.iFpsDen			= 1;
		videoInfo.iNumPlane			= 1;
		videoInfo.iCodec			= NX_CODEC_H264;
		videoInfo.iCropX			= pInfo->iCropX;
		videoInfo.iCropY			= pInfo->iCropY;
		videoInfo.iCropWidth		= pInfo->iCropWidth;
		videoInfo.iCropHeight		= pInfo->iCropHeight;
		videoInfo.iOutWidth			= pInfo->iOutWidth;
		videoInfo.iOutHeight		= pInfo->iOutHeight;
	}

	dspInfo.planeId = find_plane_for_display(pDspInfo->iCrtcIdx, 0, pDspInfo->iPlaneIdx, &dspInfo);

	dspInfo.width				= pDspInfo->iSrcWidth;
	dspInfo.height				= pDspInfo->iSrcHeight;
	dspInfo.stride				= 0;
	dspInfo.drmFormat			= pDspInfo->uDrmFormat;
	dspInfo.numPlane				= 1;//MEM_NUM_PLANE;
	dspInfo.dspSrcRect.left		= pDspInfo->iCropX;
	dspInfo.dspSrcRect.top		= pDspInfo->iCropY;
	dspInfo.dspSrcRect.right		= pDspInfo->iCropX + pDspInfo->iCropWidth;
	dspInfo.dspSrcRect.bottom	= pDspInfo->iCropY + pDspInfo->iCropHeight;
	dspInfo.dspDstRect.left		= pDspInfo->iDspX;
	dspInfo.dspDstRect.top		= pDspInfo->iDspY;
	dspInfo.dspDstRect.right		= pDspInfo->iDspWidth;
	dspInfo.dspDstRect.bottom		= pDspInfo->iDspHeight;


// #ifdef DRAW_PARKING_LINE
// 	dspInfo.planeId_PGL			= pDspInfo->iPlaneId_PGL;
// 	dspInfo.drmFormat_PGL		= pDspInfo->uDrmFormat_PGL;
// 	dspInfo.width_PGL			= pDspInfo->iDspWidth;
// 	dspInfo.height_PGL			= pDspInfo->iDspHeight;
// #endif

	dspInfo.pNativeWindow		= pDspInfo->m_pNativeWindow;


	deinterInfo.width 			= pDeinterInfo->iWidth;
	deinterInfo.height			= pDeinterInfo->iHeight;
	deinterInfo.engineSel		= pDeinterInfo->iEngineSel;
	deinterInfo.corr			= pDeinterInfo->iCorr;

	//
	//	Create Instance
	//
	m_pRefClock			= new NX_CRefClock();
	m_pEventNotifier	= new NX_CEventNotifier();

#ifdef ANDROID_SURF_RENDERING
	if(pDspInfo->m_pNativeWindow)
	{
		m_pAndroidRenderer = new NX_CAndroidRenderer((ANativeWindow*) pDspInfo->m_pNativeWindow);
	}
#endif
	m_pVideoRenderFilter= new NX_CVideoRenderFilter();

	if( pInfo->iType == CAM_TYPE_VIP )
	{
		m_pV4l2VipFilter		= new NX_CV4l2VipFilter();
	}

	if( deinterInfo.engineSel != NON_DEINTERLACER)
	{
		m_pDeinterlaceFilter 		= new NX_CDeinterlaceFilter();
	}

	if( m_pV4l2VipFilter		) m_pV4l2VipFilter		->SetRefClock( m_pRefClock );

	//
	//	Set Notifier
	//
	if( m_pV4l2VipFilter		) m_pV4l2VipFilter		->SetEventNotifier( m_pEventNotifier );

	//
	//	Set Configuration
	//
#ifdef ANDROID_SURF_RENDERING
	if(pDspInfo->m_pNativeWindow == NULL)
	{
		if( m_pV4l2VipFilter		) m_pV4l2VipFilter		->SetConfig( &videoInfo, NULL);
		if( m_pVideoRenderFilter 	) m_pVideoRenderFilter  ->SetConfig( &dspInfo, NULL);
		if( m_pDeinterlaceFilter	) m_pDeinterlaceFilter  ->SetConfig( &deinterInfo, NULL);
	}

	else
	{	if(deinterInfo.engineSel == NON_DEINTERLACER)
		{
			if( m_pV4l2VipFilter		) m_pV4l2VipFilter		->SetConfig( &videoInfo, m_pAndroidRenderer);
			if( m_pVideoRenderFilter 	) m_pVideoRenderFilter  ->SetConfig( &dspInfo, m_pAndroidRenderer);
			if( m_pDeinterlaceFilter	) m_pDeinterlaceFilter  ->SetConfig( &deinterInfo, NULL);
		}else
		{
			if( m_pV4l2VipFilter		) m_pV4l2VipFilter		->SetConfig( &videoInfo, NULL);
			if( m_pVideoRenderFilter 	) m_pVideoRenderFilter  ->SetConfig( &dspInfo, m_pAndroidRenderer);
			if( m_pDeinterlaceFilter	) m_pDeinterlaceFilter  ->SetConfig( &deinterInfo, m_pAndroidRenderer);
		}
	}
#else
	if( m_pV4l2VipFilter		) m_pV4l2VipFilter		->SetConfig( &videoInfo);
	if( m_pVideoRenderFilter 	) m_pVideoRenderFilter  ->SetConfig( &dspInfo);
	if( m_pDeinterlaceFilter	) m_pDeinterlaceFilter  ->SetConfig( &deinterInfo);

#endif
	//
	//	Pin Negotiate
	//
	NX_CBasePin	*pInputPin, *pOutputPin;


	if( m_pV4l2VipFilter && m_pDeinterlaceFilter && m_pVideoRenderFilter )
	{
		pOutputPin	= m_pV4l2VipFilter->FindPin( NX_PIN_OUTPUT, 0 );
		pInputPin 	= m_pDeinterlaceFilter->FindPin (NX_PIN_INPUT, 0);
		if( 0 > ((NX_CBaseInputPin*)pInputPin)->PinNegotiation( (NX_CBaseOutputPin*)pOutputPin ) ) goto Error;

		pOutputPin 	= m_pDeinterlaceFilter->FindPin(NX_PIN_OUTPUT, 0);
		pInputPin	= m_pVideoRenderFilter->FindPin( NX_PIN_INPUT, 0 );
		if( 0 > ((NX_CBaseInputPin*)pInputPin)->PinNegotiation( (NX_CBaseOutputPin*)pOutputPin ) ) goto Error;
	}
	else if(deinterInfo.engineSel == NON_DEINTERLACER && m_pV4l2VipFilter && m_pVideoRenderFilter)  //bypass
	{
		pOutputPin	= m_pV4l2VipFilter->FindPin( NX_PIN_OUTPUT, 0 );
		pInputPin	= m_pVideoRenderFilter->FindPin( NX_PIN_INPUT, 0 );
		if( 0 > ((NX_CBaseInputPin*)pInputPin)->PinNegotiation( (NX_CBaseOutputPin*)pOutputPin ) ) goto Error;
	}


	mRearCamStatus = REAR_CAM_STATUS_STOP;


	NxDbgMsg( NX_DBG_VBS, "%s()--\n", __FUNCTION__ );
	return 0;

Error:
	if( m_pRefClock				) { delete m_pRefClock;				m_pRefClock				= NULL;	}
	if( m_pV4l2VipFilter		) {	delete m_pV4l2VipFilter;		m_pV4l2VipFilter		= NULL; }
	if( m_pDeinterlaceFilter    ) { delete m_pDeinterlaceFilter;    m_pDeinterlaceFilter    = NULL; }
	if( m_pVideoRenderFilter	) {	delete m_pVideoRenderFilter;	m_pVideoRenderFilter	= NULL; }
#ifdef ANDROID_SURF_RENDERING
	if(m_pAndroidRenderer		) {	delete m_pAndroidRenderer;		m_pAndroidRenderer		= NULL; }
#endif

	NxDbgMsg( NX_DBG_ERR, "%s()--NxQuickRearCam Init Fail\n", __FUNCTION__ );
	return -1;
}

//------------------------------------------------------------------------------
int32_t NX_CRearCamManager::Deinit( void )
{
	NxDbgMsg( NX_DBG_VBS, "%s()++\n", __FUNCTION__ );
	NX_CAutoLock lock( &m_hLock );

	if( m_pRefClock				) { delete m_pRefClock;				m_pRefClock				= NULL;	}
	if( m_pV4l2VipFilter		) {	delete m_pV4l2VipFilter;		m_pV4l2VipFilter		= NULL; }
	if( m_pDeinterlaceFilter    ) { delete m_pDeinterlaceFilter;    m_pDeinterlaceFilter    = NULL; }
	if( m_pVideoRenderFilter	) {	delete m_pVideoRenderFilter;	m_pVideoRenderFilter	= NULL; }
#ifdef ANDROID_SURF_RENDERING
	if(m_pAndroidRenderer		) {	delete m_pAndroidRenderer;		m_pAndroidRenderer		= NULL; }
#endif

	mRearCamStatus = REAR_CAM_STATUS_STOP;

	NxDbgMsg( NX_DBG_VBS, "%s()--\n", __FUNCTION__ );
	return 0;
}

//------------------------------------------------------------------------------
int32_t NX_CRearCamManager::Start( void )
{
	NxDbgMsg( NX_DBG_VBS, "%s()++\n", __FUNCTION__ );
	int32_t ret = 0;
	NX_CAutoLock lock( &m_hLock );

	if( m_pVideoRenderFilter)
	{
		if(m_pVideoRenderFilter->Run() < 0)
		{
			NxDbgMsg( NX_DBG_VBS, "%s()--RenderFilter Run Fail\n", __FUNCTION__ );
			Stop();
			Deinit();
			return -1;
		}
	}

	if( m_pDeinterlaceFilter )
	{
		if(m_pDeinterlaceFilter->Run() < 0)
		{
			NxDbgMsg( NX_DBG_VBS, "%s()--DeinterlaceFilter Run Fail\n", __FUNCTION__ );
			Stop();
			Deinit();
			return -1;
		}
	}
	if( m_pV4l2VipFilter )
	{
		if(m_pV4l2VipFilter->Run() < 0)
		{
			NxDbgMsg( NX_DBG_VBS, "%s()--DeinterlaceFilter Run Fail\n", __FUNCTION__ );
			Stop();
			Deinit();
			return -1;
		}
	}

	mRearCamStatus = REAR_CAM_STATUS_RUNNING;

	NxDbgMsg( NX_DBG_VBS, "%s()--\n", __FUNCTION__ );
	return 0;
}

//------------------------------------------------------------------------------
int32_t NX_CRearCamManager::Stop( void )
{
	NxDbgMsg( NX_DBG_VBS, "%s()++\n", __FUNCTION__ );
	NX_CAutoLock lock( &m_hLock );

	if( m_pVideoRenderFilter	) m_pVideoRenderFilter	->Stop();
	if( m_pDeinterlaceFilter    ) m_pDeinterlaceFilter  ->Stop();
	if( m_pV4l2VipFilter		) m_pV4l2VipFilter		->Stop();

	NxDbgMsg( NX_DBG_VBS, "%s()--\n", __FUNCTION__ );
	return 0;
}

//------------------------------------------------------------------------------
int32_t NX_CRearCamManager::Pause( int32_t mbPause )
{
	NxDbgMsg( NX_DBG_VBS, "%s()++\n", __FUNCTION__ );
	NX_CAutoLock lock( &m_hLock );

	if( m_pVideoRenderFilter	) m_pVideoRenderFilter	->Pause(mbPause);
	if( m_pDeinterlaceFilter    ) m_pDeinterlaceFilter  ->Pause(mbPause);
	if( m_pV4l2VipFilter		) m_pV4l2VipFilter		->Pause(mbPause);

	NxDbgMsg( NX_DBG_VBS, "%s()--\n", __FUNCTION__ );
	return 0;
}


//------------------------------------------------------------------------------
int32_t NX_CRearCamManager::RegNotifyCallback( void (*cbFunc)(uint32_t, void*, uint32_t) )
{
	NxDbgMsg( NX_DBG_VBS, "%s()++\n", __FUNCTION__ );
	NX_CAutoLock lock( &m_hLock );

	NotifyCallbackFunc = cbFunc;

	NxDbgMsg( NX_DBG_VBS, "%s()--\n", __FUNCTION__ );
	return 0;
}

//------------------------------------------------------------------------------
int32_t NX_CRearCamManager::ChangeDebugLevel( int32_t iLevel )
{
	NX_CAutoLock lock( &m_hLock );

	int32_t iDebugLevel = NX_DBG_INFO;

	if( 0 == iLevel ) 		iDebugLevel = NX_DBG_DISABLE;
	else if( 1 == iLevel )	iDebugLevel = NX_DBG_ERR;
	else if( 2 == iLevel )	iDebugLevel = NX_DBG_WARN;
	else if( 3 == iLevel )	iDebugLevel = NX_DBG_INFO;
	else if( 4 == iLevel )	iDebugLevel = NX_DBG_DEBUG;
	else if( 5 == iLevel )	iDebugLevel = NX_DBG_VBS;

	NxChgFilterDebugLevel( iDebugLevel );
	return 0;
}

//------------------------------------------------------------------------------
void NX_CRearCamManager::ProcessEvent( uint32_t iEventCode, void *pData, uint32_t iDataSize )
{
	if( NotifyCallbackFunc && (iEventCode < NOTIFY_INTERNAL) ) {
		NotifyCallbackFunc( iEventCode, pData, iDataSize );
		return;
	}

	switch( iEventCode )
	{
		case NOTIFY_CAPTURE_DONE:
		{
			NxDbgMsg( NX_DBG_INFO, "Capture Done. (%s)\n", (char*)pData );
			break;
		}

		default:
			break;
	}
}

//------------------------------------------------------------------------------
int32_t NX_CRearCamManager::GetStatus()
{
	NxDbgMsg( NX_DBG_INFO, "%s() ++++ : status : %d\n", __FUNCTION__, mRearCamStatus );
	return mRearCamStatus;
}

//------------------------------------------------------------------------------
int32_t NX_CRearCamManager::SetDisplayPosition(int32_t x, int32_t y, int32_t w, int32_t h)
{
	NxDbgMsg( NX_DBG_INFO, "%s() ++++ \n", __FUNCTION__);

	if(m_pVideoRenderFilter != NULL)
		m_pVideoRenderFilter->SetPosition(x, y, w, h);

	return 0;
}



//------------------------------------------------------------------------------
NX_CRearCamManager* NX_CRearCamManager::m_pManager = NULL;

NX_CRearCamManager* NX_CRearCamManager::GetManager( void )
{
	if( m_pManager == NULL )
	{
		m_pManager = new NX_CRearCamManager();
	}
	return m_pManager;
}

//------------------------------------------------------------------------------
void NX_CRearCamManager::ReleaseManager( void )
{
	if( m_pManager )
	{
		delete m_pManager;
		m_pManager = NULL;
	}
}


//------------------------------------------------------------------------------
void ReleaseRearCamManager( void )
{
	NX_CRearCamManager::ReleaseManager();
}

//------------------------------------------------------------------------------
int32_t NX_RearCamInit(NX_REARCAM_INFO *p_VipInfo, DISPLAY_INFO* p_dspInfo, DEINTERLACE_INFO *p_deinterInfo)
{
	NxDbgMsg( NX_DBG_INFO, "%s() ++++ \n", __FUNCTION__ );
	NX_CRearCamManager *pstRearCamManager = NULL;
	int32_t ret = 0;

	pstRearCamManager = NX_CRearCamManager::GetManager();

	ret = pstRearCamManager->Init(p_VipInfo, p_dspInfo, p_deinterInfo);

	NxDbgMsg( NX_DBG_INFO, "%s() ----- \n", __FUNCTION__ );

	return ret;
}

//------------------------------------------------------------------------------
int32_t NX_RearCamDeInit()
{
	NxDbgMsg( NX_DBG_INFO, "%s() ++++ \n", __FUNCTION__ );
	NX_CRearCamManager *pstRearCamManager = NULL;

	pstRearCamManager = NX_CRearCamManager::GetManager();

	pstRearCamManager->Stop();
	pstRearCamManager->Deinit();

	ReleaseRearCamManager();

	NxDbgMsg( NX_DBG_INFO, "%s() ----- \n", __FUNCTION__ );

	return 0;
}


//------------------------------------------------------------------------------
int32_t NX_RearCamStart()
{
	NxDbgMsg( NX_DBG_INFO, "%s() ++++ \n", __FUNCTION__ );
	NX_CRearCamManager *pstRearCamManager;
	int32_t ret = 0;

	pstRearCamManager = NX_CRearCamManager::GetManager();

	ret = pstRearCamManager->Start();

	return ret;
}

//------------------------------------------------------------------------------
int32_t NX_RearCamPause(int32_t m_bPause)
{
	//NxDbgMsg( NX_DBG_INFO, "%s() ++++ \n", __FUNCTION__ );
    NX_CRearCamManager *pstRearCamManager;
    int32_t ret = 0;

	pstRearCamManager = NX_CRearCamManager::GetManager();

	ret = pstRearCamManager->Pause(m_bPause);

	return ret;
}

//------------------------------------------------------------------------------
int32_t NX_RearCamGetStatus()
{
	NxDbgMsg( NX_DBG_INFO, "%s() ++++ \n", __FUNCTION__ );
    NX_CRearCamManager *pstRearCamManager;
    int32_t ret = 0;

	pstRearCamManager = NX_CRearCamManager::GetManager();

	ret = pstRearCamManager->GetStatus();

	return ret;
}

//---------------------------------------------------------------------------
int32_t NX_RearCamSetDisplayPosition(int32_t x, int32_t y, int32_t w, int32_t h)
{
	NxDbgMsg( NX_DBG_INFO, "%s() ++++ \n", __FUNCTION__ );
    NX_CRearCamManager *pstRearCamManager;
    int32_t ret = 0;

	pstRearCamManager = NX_CRearCamManager::GetManager();

	ret = pstRearCamManager->SetDisplayPosition(x, y, w, h);

	return ret;
}


int32_t NX_RearCamGetVersion()
{
	NxDbgMsg( NX_DBG_INFO, "%s() --- NxQuickRearCam Version : %d.%d.%d.%d", __FUNCTION__, VER_MAJOR, VER_MINOR, VER_REVISION, VER_RESERVATION);

    return ((VER_MAJOR << 24) | (VER_MINOR << 16) | (VER_REVISION << 8) | (VER_RESERVATION));
}

