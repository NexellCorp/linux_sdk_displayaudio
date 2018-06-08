#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <NX_AVIn.h>
#include <nx-v4l2.h>
#include <drm/drm_fourcc.h>

bool StartAVIn( int32_t planeId, int32_t crtcId, int dspX, int dspY, int dspW, int dspH )
{
	CAMERA_INFO m_CamInfo;
	DISPLAY_INFO m_DspInfo;
	//
	//	NOTICE: Should be change CamInfo & DspInfo for target system.
	//
	int32_t camWidth = 704;
	int32_t camHeight = 480;

	memset( &m_CamInfo, 0, sizeof(m_CamInfo) );
	memset( &m_DspInfo, 0, sizeof(m_DspInfo) );

	//  Camera Information
	m_CamInfo.iModule		= 1;
	m_CamInfo.iSensorId		= nx_sensor_subdev;
	m_CamInfo.bInterlace	= 0;
	m_CamInfo.iWidth		= camWidth;
	m_CamInfo.iHeight		= camHeight;
	m_CamInfo.iCropX		= 0;
	m_CamInfo.iCropY		= 0;
	m_CamInfo.iCropWidth	= camWidth;
	m_CamInfo.iCropHeight	= camHeight;
	m_CamInfo.iOutWidth     = camWidth;
	m_CamInfo.iOutHeight	= camHeight;


	//	Get graphic view's rect
	m_DspInfo.iPlaneId		= planeId;
	m_DspInfo.iCrtcId		= crtcId;
	m_DspInfo.uDrmFormat	= DRM_FORMAT_YUV420;
	m_DspInfo.iSrcWidth		= camWidth;
	m_DspInfo.iSrcHeight	= camHeight;
	m_DspInfo.iCropX		= 0;
	m_DspInfo.iCropY		= 0;
	m_DspInfo.iCropWidth	= camWidth;
	m_DspInfo.iCropHeight	= camHeight;
	m_DspInfo.iDspX			= dspX;
	m_DspInfo.iDspY			= dspY;
	m_DspInfo.iDspWidth 	= dspW;
	m_DspInfo.iDspHeight	= dspH;

	if( 0 != NXDA_StartAVInService( &m_CamInfo, &m_DspInfo ) )
	{
		return false;
	}

	return true;
}

void StopAVIn()
{
	NXDA_StopAVInService();
}


void AgingTest()
{
	unsigned int count = 0;
	while( 1 )
	{
		printf("=============== Count = %d ===================\n", count ++);
		if( !StartAVIn( 27, 26, 0, 0, 1024, 600 ) )
			break;
		usleep( 100000000 );
		//usleep( 1000000 );
		StopAVIn();
	}
}


int main(int argc, char *argv[])
{
	AgingTest();
	return 0;
}
