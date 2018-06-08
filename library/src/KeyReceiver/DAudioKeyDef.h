#ifndef DAUDIOKEYDEF_H
#define DAUDIOKEYDEF_H

#include <stdint.h>

#define KEY_PRESSED		1
#define KEY_RELEASED	0

//
//		DAUDIO_KEY Type Format
//		Upper 2 Byte : Key Category
//		Lower 2 Byte : Key Value
//

typedef enum DAUDIO_KEY{
	//	Category Main
	DAUD_KEY_POWER			= 0x00010000,

	//	Category Mode
	DAUD_KEY_MODE			= 0x00020000,
	DAUD_KEY_MODE_AUDIO		,
	DAUD_KEY_MODE_VIDEO		,
	DAUD_KEY_MODE_RADIO		,
	DAUD_KEY_MODE_BLUETOOTH	,
	DAUD_KEY_MODE_AVIN		,
	DAUD_KEY_MODE_PHONE		,
	DAUD_KEY_MODE_SETTING	,
	DAUD_KEY_MODE_3DAVM		,
	DAUD_KEY_MODE_3DAVM_CLOSE,

	//	Category Volume
	DAUD_KEY_VOL_UP			= 0x00030000,
	DAUD_KEY_VOL_DOWN		,
	DAUD_KEY_VOL_MUTE		,

	//	Category Navigation
	DAUD_KEY_NAVI_UP		= 0x00040000,
	DAUD_KEY_NAVI_DOWN		,
	DAUD_KEY_NAVI_LEFT		,
	DAUD_KEY_NAVI_RIGHT		,
} DAUDIO_KEY;

#ifdef __cplusplus
extern "C"{
#endif

const char *GetKeyName( int32_t key );

#ifdef __cplusplus
}
#endif

#endif // DAUDIOKEYDEF_H
