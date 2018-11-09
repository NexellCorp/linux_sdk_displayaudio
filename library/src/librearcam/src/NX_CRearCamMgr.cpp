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
//	Module		: Quick Rear Camera Library
//	File		: NX_CRearCamMgr.cpp
//	Description	: Rearcam management module.
//	Author		: SeongO Park(ray@nexell.co.kr)
//	Export		:
//	History		:
//
//------------------------------------------------------------------------------
#include <stdint.h>
#include <drm/drm_fourcc.h>

#include "NX_CV4l2Camera.h"
#include "NX_CDrmDisplay.h"

#include <CNX_BaseClass.h>
#include <NX_RearCam.h>

#define virtual vir
#include <xf86drm.h>
#include <xf86drmMode.h>
#undef virtual

#ifdef NX_DTAG
#undef NX_DTAG
#endif
#define NX_DTAG		"[NX_CRearCamMgr]"
#include "NX_DbgMsg.h"

enum RENDER_MODULE {
	RENDER_DRM,
	RENDER_V4L2,
	RENDER_ANDROID
};

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


static void find_plane_for_display(int32_t crtcIdx,
                                   int32_t findRgb,
                                   int32_t layerIdx,
                                   DISPLAY_INFO *pdsp_info)
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

    pdsp_info->iConnectorID     = connId;
    pdsp_info->iCrtcId        	= crtcId;
    pdsp_info->iPlaneId        = planeId;
}



class NX_CRearCamMgr : protected CNX_Thread
{
public:
	NX_CRearCamMgr();
	~NX_CRearCamMgr();

	int32_t StartService(CAMERA_INFO *pCamInfo, DISPLAY_INFO *pDspInfo);
	int32_t StopService();
	void RegisterRendererCB( void *pAppData, int32_t (callback)(void *, int32_t, void *, int32_t) );
	void RegisterControlCB( void *pAppData, int32_t (callback)(void *, int32_t, void *, int32_t) );

	int32_t Init();
	int32_t Deinit();
	int32_t Hide();     //  Display Off
	int32_t Show();     //  Display On
	int32_t SetPosition( int32_t x, int32_t y, int32_t width, int32_t height );


	//  Implementation Pure Virtual Function
	virtual void  ThreadProc();
private:
	enum { MAX_NUM_BUFFER=32, NUM_BUFFER=8 };

	int32_t			m_iDspModule;
	NX_CV4l2Camera	*m_hCam;
	NX_VIP_INFO		m_hVipInfo;
	NX_DISPLAY_INFO	m_hDspInfo;
	NX_CDrmDisplay	*m_hDisplay;
	int32_t 		(*m_hDspCallBack)(void *, int32_t, void *, int32_t);
	int32_t			(*m_hCtrlCallBack)(void *, int32_t, void *, int32_t);

	void			*m_pDspCbData;
	void			*m_pCtrlCbData;

	int32_t			m_bExitLoop;
	NX_VID_MEMORY_INFO *m_pCamBuffer[MAX_NUM_BUFFER];

	int32_t			m_bAllocated;

	//
	//	Video memory allocate & free
	//
	int32_t AllocateMemory( int32_t width, int32_t height, int32_t planes, int32_t buffers );
	void FreeMemory();


	//
	//	Display On/Off Control
	//
	void			OffDisplay();
	void			OnDisplay();

public:
	static void RearCamCallback( void *, int32_t );

public:
	static NX_CRearCamMgr *GetInstance();
	static NX_CRearCamMgr *m_pSingleTone;
};

//
//		Single Tone
//
NX_CRearCamMgr *NX_CRearCamMgr::m_pSingleTone = NULL;

NX_CRearCamMgr *NX_CRearCamMgr::GetInstance()
{
	if( NULL == m_pSingleTone )
	{
		m_pSingleTone = new NX_CRearCamMgr();
	}
	return m_pSingleTone;
}

NX_CRearCamMgr::NX_CRearCamMgr()
	: m_hCam (NULL)
	, m_hDisplay (NULL)
	, m_hDspCallBack (NULL)
	, m_hCtrlCallBack (NULL)
	, m_pDspCbData (NULL)
	, m_pCtrlCbData (NULL)
	, m_bAllocated (0)
{
	for( int32_t i=0 ; i<MAX_NUM_BUFFER ; i++ )
	{
		m_pCamBuffer[i] = NULL;
	}
}


NX_CRearCamMgr::~NX_CRearCamMgr()
{
	if( m_bAllocated )
	{
		FreeMemory();
	}
}

#ifndef ALLINEDN
#define ALLINEDN(X,N)	( (X+N-1) & (~(N-1)) )
#endif
int32_t NX_CRearCamMgr::AllocateMemory( int32_t width, int32_t height, int32_t planes, int32_t buffers )
{
	int32_t i;
	int32_t allocWidth = width;

	if( m_bAllocated )
		return 0;

	if( m_hVipInfo.bInterlace )
	{
		allocWidth  = ALLINEDN(width, 128);
	}
	else{
		allocWidth  = ALLINEDN(width, 64);
	}
	for( i=0 ; i<buffers ; i++ )
	{
		m_pCamBuffer[i] = NX_AllocateVideoMemory( allocWidth, height,
									planes, DRM_FORMAT_YUV420, 512 );
		if( NULL == m_pCamBuffer[i] )
			goto ERROR_EXIT;

		if( m_hVipInfo.bInterlace ){
			m_pCamBuffer[i]->width = width;
			m_pCamBuffer[i]->stride[0] = ALLINEDN(width, 128);
			m_pCamBuffer[i]->stride[1] = ALLINEDN(width, 64);
			m_pCamBuffer[i]->stride[2] = ALLINEDN(width, 64);
		}
	}
	m_bAllocated = 1;
	return 0;
ERROR_EXIT:
	for( i=0 ; i<buffers ; i++ )
	{
		if( m_pCamBuffer[i] )
		{
			NX_FreeVideoMemory( m_pCamBuffer[i] );
			m_pCamBuffer[i] = NULL;
		}
	}
	return -1;
}


void NX_CRearCamMgr::FreeMemory()
{
	for( int32_t i=0 ; i<MAX_NUM_BUFFER ; i++ )
	{
		if( m_pCamBuffer[i] )
		{
			NX_FreeVideoMemory( m_pCamBuffer[i] );
			m_pCamBuffer[i] = NULL;
		}
	}
	m_bAllocated = 0;
}


int32_t NX_CRearCamMgr::Init()
{
	if( 0 != AllocateMemory( m_hVipInfo.iWidth, m_hVipInfo.iHeight, m_hVipInfo.iNumPlane, NUM_BUFFER ) )
	{
		printf("AllocateMemory Failed!!!()\n");
		return -1;
	}

	//	Open Camera Module
	m_hCam = new NX_CV4l2Camera();
	//	Add Allocated Memory Buffer to Camera Module
	for( int32_t i=0; i<NUM_BUFFER ; i++ )
	{
		m_hCam->AddVideoMemory(m_pCamBuffer[i]);
	}
	//	Initialize V4L2 Camera
	if( 0 > m_hCam->Init( &m_hVipInfo ) )
	{
		goto ERROR_EXIT;
	}

	m_hDisplay = new NX_CDrmDisplay();

	if( 0 != m_hDisplay->DspInit( &m_hDspInfo ) )
	{
		goto ERROR_EXIT;
	}

	return 0;

ERROR_EXIT:
	if( m_hCam )
	{
		delete m_hCam;
		m_hCam = NULL;
	}
	FreeMemory();
	return -1;
}

int32_t NX_CRearCamMgr::Deinit()
{
	if( m_hDisplay )
	{
		delete m_hDisplay;
		m_hDisplay = NULL;
	}
	if( m_hCam )
	{
		delete m_hCam;
		m_hCam = NULL;
	}
	return 0;
}

void NX_CRearCamMgr::OffDisplay()
{
	m_bExitLoop = 1;
	Stop();
	if( m_hDisplay )
	{
		m_hDisplay->DspClose();
		delete m_hDisplay;
		m_hDisplay = NULL;
	}
}

void NX_CRearCamMgr::OnDisplay()
{
	m_hDisplay = new NX_CDrmDisplay();
	m_bExitLoop = 0;
	Start();
	m_hDisplay->DspInit( &m_hDspInfo );
}


int32_t NX_CRearCamMgr::Hide()
{
	OffDisplay();
	if( m_iDspModule == DSP_MODULE_QT && NULL != m_hCtrlCallBack )
	{
		m_hCtrlCallBack( m_pCtrlCbData, CB_TYPE_CMD_HIDE, NULL, 0 );
	}
	return 0;
}

int32_t NX_CRearCamMgr::Show()
{
	//
	//	Display Show
	//
	OnDisplay();

	if( m_iDspModule == DSP_MODULE_QT && NULL != m_hCtrlCallBack )
	{
		m_hCtrlCallBack( m_pCtrlCbData, CB_TYPE_CMD_SHOW, NULL, 0 );
	}
	return 0;
}

int32_t NX_CRearCamMgr::SetPosition( int32_t x, int32_t y, int32_t width, int32_t height )
{
	if( m_hDisplay )
	{
		return m_hDisplay->SetPosition( x, y, width, height );
	}
	return -1;
}


void NX_CRearCamMgr::ThreadProc()
{
	int32_t bufIdx = -1;
	while (!m_bExitLoop)
	{
		m_hCam->DequeueBuffer( &bufIdx );
		if( m_iDspModule == DSP_MODULE_QT && NULL != m_hDspCallBack )
		{
			m_hDspCallBack( m_pDspCbData, CB_TYPE_BUFFER, m_pCamBuffer[bufIdx], sizeof(NX_VID_MEMORY_INFO) );
		}
		else
		{
			m_hDisplay->QueueBuffer( bufIdx, m_pCamBuffer[bufIdx] );
		}
		m_hCam->QueueBuffer( bufIdx  );
	}
}


int32_t NX_CRearCamMgr::StartService(CAMERA_INFO *pCamInfo, DISPLAY_INFO *pDspInfo)
{
	int32_t ret;

	//	Setting Video Input Information
	memset( &m_hVipInfo, 0, sizeof(m_hVipInfo) );
	m_hVipInfo.iModule		= pCamInfo->iModule;
	m_hVipInfo.iSensorId	= pCamInfo->iSensorId;
	m_hVipInfo.bInterlace	= pCamInfo->bInterlace;
	m_hVipInfo.iWidth		= pCamInfo->iWidth;
	m_hVipInfo.iHeight		= pCamInfo->iHeight;
	m_hVipInfo.iFpsNum		= 30;
	m_hVipInfo.iFpsDen		= 1;
	m_hVipInfo.iNumPlane	= 1;
	m_hVipInfo.iCropX		= pCamInfo->iCropX;
	m_hVipInfo.iCropY		= pCamInfo->iCropY;
	m_hVipInfo.iCropWidth	= pCamInfo->iCropWidth;
	m_hVipInfo.iCropHeight	= pCamInfo->iCropHeight;
	m_hVipInfo.iOutWidth	= pCamInfo->iOutWidth;
	m_hVipInfo.iOutHeight	= pCamInfo->iOutHeight;

	//	Setting Display Information
	memset( &m_hDspInfo, 0, sizeof(m_hDspInfo) );

	find_plane_for_display(pDspInfo->iDspCrtcIdx, 0, pDspInfo->iDspLayerIdx, pDspInfo);

	m_hDspInfo.planeId				= pDspInfo->iPlaneId;
	m_hDspInfo.ctrlId				= pDspInfo->iCrtcId;
	m_hDspInfo.width				= pDspInfo->iSrcWidth;
	m_hDspInfo.height				= pDspInfo->iSrcHeight;
	m_hDspInfo.stride				= 0;
	m_hDspInfo.drmFormat			= pDspInfo->uDrmFormat;
	m_hDspInfo.numPlane				= 1;//MEM_NUM_PLANE;
	m_hDspInfo.dspSrcRect.left		= pDspInfo->iCropX;
	m_hDspInfo.dspSrcRect.top		= pDspInfo->iCropY;
	m_hDspInfo.dspSrcRect.right		= pDspInfo->iCropX + pDspInfo->iCropWidth;
	m_hDspInfo.dspSrcRect.bottom	= pDspInfo->iCropY + pDspInfo->iCropHeight;
	m_hDspInfo.dspDstRect.left		= pDspInfo->iDspX;
	m_hDspInfo.dspDstRect.top		= pDspInfo->iDspY;
	m_hDspInfo.dspDstRect.right		= pDspInfo->iDspX + pDspInfo->iDspWidth;
	m_hDspInfo.dspDstRect.bottom	= pDspInfo->iDspY + pDspInfo->iDspHeight;

	ret = Init();

	if( ret != 0 )
	{
		NxDbgMsg( NX_DBG_ERR, "Init() Failed !!!");
		return ret;
	}
	m_bExitLoop = 0;
	Start();
	return ret;
}

int32_t NX_CRearCamMgr::StopService()
{
	m_bExitLoop = 1;
	Stop();
	Deinit();
	return 0;
}

void NX_CRearCamMgr::RegisterRendererCB( void *pAppData, int32_t (callback)(void *, int32_t, void *, int32_t) )
{
	m_pDspCbData = pAppData;
	m_hDspCallBack = callback;
	m_iDspModule = DSP_MODULE_QT;
}

void NX_CRearCamMgr::RegisterControlCB( void *pAppData, int32_t (callback)(void *, int32_t, void *, int32_t) )
{
	m_pCtrlCbData = pAppData;
	m_hCtrlCallBack = callback;
	m_iDspModule = DSP_MODULE_QT;
}


//////////////////////////////////////////////////////////////////////////////
//																			//
//									APIs									//
//																			//
//////////////////////////////////////////////////////////////////////////////
int32_t NXDA_ShowRearCam(CAMERA_INFO *pCamInfo, DISPLAY_INFO *pDspInfo)
{
	NX_CRearCamMgr *pCamMgr = NX_CRearCamMgr::GetInstance();
	return pCamMgr->StartService(pCamInfo, pDspInfo);
}

int32_t NXDA_HideRearCam()
{
	NX_CRearCamMgr *pCamMgr = NX_CRearCamMgr::GetInstance();
	return pCamMgr->StopService();
}

void NXDA_RegRenderCallback( void *pApp, int32_t (callback)(void *, int32_t, void *, int32_t) )
{
	NX_CRearCamMgr *pCamMgr = NX_CRearCamMgr::GetInstance();
	pCamMgr->RegisterRendererCB( pApp, callback );
}

void NXDA_RegControlCallback( void *pApp, int32_t (callback)(void *, int32_t, void *, int32_t) )
{
	NX_CRearCamMgr *pCamMgr = NX_CRearCamMgr::GetInstance();
	pCamMgr->RegisterControlCB( pApp, callback );
}

#include <NX_Type.h>

const char* NXDA_RearCamGetVersion()
{
	return NX_VERSION_LIBREARCAM;
}
