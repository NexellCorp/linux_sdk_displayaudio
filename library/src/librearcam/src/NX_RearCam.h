#ifndef __NX_REARCAM_H__
#define __NX_REARCAM_H__

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
}CAMERA_INFO;

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
}DISPLAY_INFO;


/*
	Sart RearCam Service
*/

int32_t NXDA_ShowRearCam(CAMERA_INFO *, DISPLAY_INFO *);
int32_t NXDA_HideRearCam();

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
void NXDA_RegRenderCallback( void *pApp, int32_t (callback)(void *, int32_t, void *, int32_t) );
//	Callback for Control
void NXDA_RegControlCallback( void *pApp, int32_t (callback)(void *, int32_t, void *, int32_t) );



//
//
//	Back Gear Detect
//
//
enum {
	BACKGEAR_UNKNOWN = -1,
	BACKGEAR_ON = 0,
	BACKGEAR_OFF = 1
};

/*
	@ nGpio : GPIO Port Number
		GPIOA0 - GPIOA31 (   0 -  31 )
		GPIOB0 - GPIOB31 (  32 -  63 )
		GPIOC0 - GPIOC31 (  64 -  95 )
		GPIOD0 - GPIOD31 (  96 - 127 )
		GPIOE0 - GPIOE31 ( 128 - 159 )
		ALIVE0 - ALIVE7  ( 160 - 167 )

	@ nChkDelay : GPIO check delay (milli-seconds)
*/
int32_t NXDA_StartBackGearDetectService( int32_t nGpio, int32_t nChkDelay );
void NXDA_StopBackGearDectectService();
void NXDA_RegisterBackGearEventCallBack(void *, void (*callback)(void *, int32_t));

const char* NXDA_RearCamGetVersion();

#ifdef __cplusplus
}
#endif

#endif
