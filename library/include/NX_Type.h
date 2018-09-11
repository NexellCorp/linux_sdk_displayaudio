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

#ifndef __NX_TYPE_H__
#define __NX_TYPE_H__

#include <stdint.h>

//
//	Common Parameter
//
#define NX_QT_CUSTOM_EVENT_TYPE		1000

//
// 
//
enum FocusType {
    FocusType_Get,
    FocusType_Set
};

enum FocusPriority {
	FocusPriority_Highest,
	FocusPriority_High,
	FocusPriority_Normal
};

enum ButtonType {
	ButtonType_Ok = 1,
	ButtonType_Cancel,
	ButtonType_Count
};

enum ButtonVisibility {
	ButtonVisibility_Ok = ButtonType_Ok,
	ButtonVisibility_Cencel = ButtonType_Cancel,
	ButtonVisibility_Default = ButtonType_Ok|ButtonType_Cancel,
	ButtonVisibility_Count = ButtonVisibility_Default
};

enum ButtonLocation {
	ButtonLocation_Ok_Cancel,
	ButtonLocation_Cancel_Ok
};

struct PopupMessage {
	char *pMsgTitle;
	char *pMsgBody;

	ButtonVisibility eVisibility;
	ButtonLocation eLocation;
	const char *pStylesheet[ButtonType_Count];

	unsigned int uiTimeout;

	PopupMessage()
	{
		pMsgTitle = 0;
		pMsgBody = 0;

		eVisibility = ButtonVisibility_Default;

		eLocation = ButtonLocation_Cancel_Ok;

		pStylesheet[ButtonType_Ok] = 0;
		pStylesheet[ButtonType_Cancel] = 0;

		uiTimeout = 0;
	}
};

//
//	IPC Command
//
enum {
	//	For Reply
	NX_REPLY_DONE					= 0x00000000,
	NX_REPLY_FAIL					= 0x00000001,

	//	For Process
	NX_REQUEST_PROCESS_ADD			= 0x10000001,
	NX_REQUEST_PROCESS_REMOVE		= 0x10000002,
	NX_REQUEST_PROCESS_CHECK		= 0x10000004,
	NX_REQUEST_PROCESS_SHOW			= 0x10000008,
	NX_REQUEST_PROCESS_HIDE			= 0x10000010,
	NX_REQUEST_LAUNCHER_SHOW		= 0x10000020,
	NX_REQUEST_PROCESS_RESERVED1	= 0x10000040,
	NX_REQUEST_PROCESS_RESERVED2	= 0x10000080,

	//	For Video / Audio Focus
	NX_REQUEST_FOCUS_MASK			= 0x0FFFFFFF,
	NX_REQUEST_FOCUS_VIDEO			= 0x10001000,
	NX_REQUEST_FOCUS_AUDIO			= 0x10002000,
	NX_REQUEST_FOCUS_VIDEO_TRANSIENT= 0x10004000,
	NX_REQUEST_FOCUS_AUDIO_TRANSIENT= 0x10008000,
	NX_REQUEST_FOCUS_VIDEO_LOSS		= 0x10010000,
	NX_REQUEST_FOCUS_AUDIO_LOSS		= 0x10020000,
	NX_REQUEST_FOCUS_RESERVED1		= 0x10040000,
	NX_REQUEST_FOCUS_RESERVED2		= 0x10080000,

	NX_REQUEST_FOCUS_AV				= NX_REQUEST_FOCUS_AUDIO | NX_REQUEST_FOCUS_VIDEO,
	NX_REQUEST_FOCUS_AV_TRANSIENT	= NX_REQUEST_FOCUS_AUDIO_TRANSIENT | NX_REQUEST_FOCUS_VIDEO_TRANSIENT,
	NX_REQUEST_FOCUS_AV_LOSS		= NX_REQUEST_FOCUS_AUDIO_LOSS | NX_REQUEST_FOCUS_VIDEO_LOSS,

	//	For Event
	NX_EVENT_BROADCAST				= 0x80000000,
	NX_EVENT_SDCARD_INSERT			= 0x80000001,
	NX_EVENT_SDCARD_REMOVE			= 0x80000002,
	NX_EVENT_USB_INSERT				= 0x80000003,
	NX_EVENT_USB_REMOVE				= 0x80000004,
	NX_EVENT_MEDIA_SCAN_DONE		= 0x80000005,

	//	For Debugging
	NX_REQUEST_PROCESS_INFO			= 0xF0000000,
};


//
//	Process Flags
//
enum {
	NX_PROCESS_FLAG_NONE			= 0x00000000,
	NX_PROCESS_FLAG_LAUNCHER		= 0x10000000,
	NX_PROCESS_FLAG_BACKGROUND		= 0x20000000,
	NX_PROCESS_FLAG_CAMERA			= 0x40000000,

	NX_PROCESS_FLAG_FOCUS_VIDEO		= 0x00000001,
	NX_PROCESS_FLAG_FOCUS_AUDIO		= 0x00000002,
	NX_PROCESS_FLAG_FOCUS_AV		= NX_PROCESS_FLAG_FOCUS_VIDEO | NX_PROCESS_FLAG_FOCUS_AUDIO,
};


//
//	Focus Priority
//	: If the FocusPriority is equal or greater,
//	  the Application can gain/loss focus
//
enum {
	NX_FOCUS_PRIORITY_LEAST			= 0,	//	default video priority
	NX_FOCUS_PRIORITY_LOW,
	NX_FOCUS_PRIORITY_MIDDLE,
	NX_FOCUS_PRIORITY_TOP,
	NX_FOCUS_PRIORITY_MOST,
};


//
//	Display Device
//
enum {
	NX_DISPLAY_PRIMARY				= 0,	//	default display device
	NX_DISPLAY_SECONDARY,
};


//
//	Process Information
//
typedef struct NX_PROCESS_INFO {
	char		szAppName[64];		//	Application Name
	char		szSockName[64];		//	Socket Name

	uint32_t 	iProcessId;			//	Process Id
	uint32_t	iFlags;				//	Flags
	uint32_t	iVideoPriority;		//	Video Focus Priority
	uint32_t	iAudioPriority;		//	Audio Focus Priority
	uint32_t	iDisplayDevice;		//	Display Device
} NX_PROCESS_INFO;


//
//	Volume Information
//
#define NX_MEDIA_DATABASE_PATH	"/home/root"
#define NX_MEDIA_DATABASE_NAME	"mediainfo.db"
#define NX_MEDIA_DATABASE_TABLE	"tbl_media"

enum {
	NX_VOLUME_TYPE_UNKNOWN = -1,
	NX_VOLUME_TYPE_MSDOS,
	NX_VOLUME_TYPE_VFAT,
	NX_VOLUME_TYPE_NTFS,
	NX_VOLUME_TYPE_EXT2,
	NX_VOLUME_TYPE_EXT3,
	NX_VOLUME_TYPE_EXT4,
};

typedef struct NX_VOLUME_INFO {
	char	szDev[64];				//	Storage Device Node
	char	szVolume[64];			//	Mount Position
	int32_t iType;					//	Mount VFS Type
} NX_VOLUME_INFO;


//
//	Version Information
//
#define NX_VERSION_LIBKEYRECEIVER	"1.0.0"
#define NX_VERSION_LIBAVIN			"1.0.0"
#define NX_VERSION_LIBBASEUI		"1.0.0"
#define NX_VERSION_LIBCONFIG		"1.0.0"
#define NX_VERSION_LIBDAUDIOIPC		"1.0.0"
#define NX_VERSION_LIBDAUDIOUTILS	"1.0.0"
#define NX_VERSION_LIBPACPCLIENT	"1.0.0"
#define NX_VERSION_LIBREARCAM		"1.0.0"

#endif	// __NX_TYPE_H__
