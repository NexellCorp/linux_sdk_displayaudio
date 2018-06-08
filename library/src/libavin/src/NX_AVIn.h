#ifndef __NX_AVIN_H__
#define __NX_AVIN_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
	Display method enumerate

	@ DSP_MODULE_DRM
	 : Display on DRM video layer.

	@ DSP_MOUDLE_QT
	 : Display on QT windows.(N/A)
*/
enum{
	DSP_MODULE_DRM,
	DSP_MODULE_QT,
};

enum {
	CB_TYPE_BUFFER,
	CB_TYPE_CMD_HIDE,
	CB_TYPE_CMD_SHOW,
};

typedef struct tagCAMERA_INFO{
	int32_t		iModule;
	int32_t		iSensorId;

	int32_t		bInterlace;

	int32_t		iWidth;
	int32_t		iHeight;

	int32_t		iCropX;
	int32_t		iCropY;
	int32_t		iCropWidth;
	int32_t		iCropHeight;
	int32_t		iOutWidth;
	int32_t		iOutHeight;
} CAMERA_INFO;

typedef struct tagDISPLAY_INFO{
	int32_t		iPlaneId;       //  DRM Plane ID
	int32_t		iCrtcId;        //  DRM CRTC ID
	uint32_t	uDrmFormat;		//	DRM Data Format
	int32_t		iSrcWidth;      //  Input Image's Width
	int32_t		iSrcHeight;     //  Input Image's Height
	int32_t		iCropX;         //  Input Source Position
	int32_t		iCropY;
	int32_t		iCropWidth;
	int32_t		iCropHeight;
	int32_t		iDspX;          //  Display Position
	int32_t		iDspY;
	int32_t		iDspWidth;
	int32_t		iDspHeight;
} DISPLAY_INFO;


//
//	A/V In Service Start/Stop Control	
//
int32_t NXDA_StartAVInService(CAMERA_INFO *, DISPLAY_INFO *);
void NXDA_StopAVInService();
int32_t NXDA_SetAVInVideoPosition( int32_t x, int32_t y, int32_t width, int32_t height );

//
// Callback Function Format
//  : int32_t callback( void *pApp, int32_t type, void *data, int32_t dataSize );
//  Parameters :
//		void *pApp     :
//			This is the pApp pointer data used in the NXDA_RegRenderCallback( void *pApp ).
//			This parameter is used when you want to use a class object in c ++ or
//			a specific pointer managed by an application at the callback return point.
//		int32_t type  :
//			Callback function type. See CB_TYPE_XXXX parameter.
//		void *data     :
//			This parameter is used to send data through the Callback function.
//		void *dataSize :
//			This is the size of data.
//
//	Callback for Rendering
//	if callback is not set, use drm video layer.
void NXDA_RegAVInRenderCallback( void *pApp, int32_t (callback)(void *, int32_t, void *, int32_t) );
//	Callback for Control
void NXDA_RegAVInControlCallback( void *pApp, int32_t (callback)(void *, int32_t, void *, int32_t) );


const char* NXDA_AVInGetVersion();

#ifdef __cplusplus
}
#endif

#endif	// __NX_AVIN_H__
