#include "DAudioKeyDef.h"

static const char *gstPowerString[] = 
{
	"DAUD_KEY_POWER",
};

static const char *gstModeString[] = 
{
	"DAUD_KEY_MODE_AUDIO",
	"DAUD_KEY_MODE_VIDEO",
	"DAUD_KEY_MODE_RADIO",
	"DAUD_KEY_MODE_BLUETOOTH",
	"DAUD_KEY_MODE_AVIN",
	"DAUD_KEY_MODE_PHONE",
	"DAUD_KEY_MODE_SETTING",
	"DAUD_KEY_MODE_3DAVM",
	"DAUD_KEY_MODE_3DAVM_CLOSE"
};

static const char *gstVolumeString[] = 
{
	"DAUD_KEY_VOL_UP",
	"DAUD_KEY_VOL_DOWN",
	"DAUD_KEY_VOL_MUTE",
};

static const char *gstNaviString[] = 
{
	"DAUD_KEY_NAVI_UP", 
	"DAUD_KEY_NAVI_DOWN", 
	"DAUD_KEY_NAVI_LEFT", 
	"DAUD_KEY_NAVI_RIGHT", 
};

#define KEY_UNKNOWN		"UNKNOWN"

const char *GetKeyName( int32_t key )
{
	if(0){}
	else if( DAUD_KEY_POWER == (key&0xffff0000) )
	{
		return gstPowerString[ key & 0xffff ];
	}
	else if( DAUD_KEY_MODE == (key&0xffff0000) )
	{
		return gstModeString[ key & 0xffff ];

	}
	else if( DAUD_KEY_VOL_UP == (key&0xffff0000) )
	{
		return gstVolumeString[ key & 0xffff ];

	}
	else if( DAUD_KEY_NAVI_UP == (key&0xffff0000) )
	{
		return gstNaviString[ key & 0xffff ];

	}
	else
	{
		return KEY_UNKNOWN;
	}
}
