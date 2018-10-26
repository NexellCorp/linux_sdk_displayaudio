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
//	Module		: Audio Manager Library for D-Audio
//	File		: libaudmanager.h
//	Description	: This module audio path and device ownership management library.
//	Author		: SeongO Park ( ray@nexell.co.kr )
//	Export		:
//	History		:
//		- 2017.05.26 : First implementation.
//
//------------------------------------------------------------------------------


#ifndef __LIBAUDMANAGER_H__
#define __LIBAUDMANAGER_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


//	Version Informations
#define	DAAM_VER_MAJOR	0
#define DAAM_VER_MINOR	1
#define DAAM_VER_REV	0
#define DAAM_REL_DATE	"20170526"


//
//	Audio Path ID
//
#define DAAM_MAIN_AUDIO			0
#define DAAM_SECOND_AUDIO		1


//
//	Audio Status
//
#define DAAM_STATE_NOPATH		-2		//	Have no audio path
#define DAAM_STATE_ERR			-1		//	other general errors.
#define DAAM_STATE_NONE			0		//	exist path and not used.
#define DAAM_STATE_INUSE		1		//	exist path and using other app.


//
//	General Description
//
//	How to get audio path's ownership.
//
//	Method 1. Using DAAM_RequestOwner function.
//		If other application have audio path's ownership,
//		then audio manager broadcast RELEASE_OWNERSHIP message.
//		And DAAM_RequestOwne() function return DAAM_STATE_INUSE.
//		And the application with ownership call DAAM_ReleaseOwner() to release ownership.
//		The application check audio path status using DAAM_GetStatus().
//		If the audio path's state is DAAM_STATE_NONE. DAAM_GetOwner function.
//
/*		pseudo code
		state = DAAM_ReqeustOwner( DAAM_MAIN_AUDIO );
		if( state == DAAM_STATE_INUSE )
		{
			counter = 100;
			do
			{
				state = DAAM_GetStatus( DAAM_MAIN_AUDIO );
				if( state != DAAM_STATE_NONE )
				{
					usleep( 10000 );	//	check every 10 msec
				}
			} while( counter -- );
			if( counter == 0  )
			{
				//	Timeout
			}
			DAAM_GetOwner( DAAM_MAIN_AUDIO );
		}
*/
//
//	Method 2. Direct call DAAM_GetOwner function.
//		DAAM_GetOwner function take audio path's ownership.
//		Because this way internally kill current owner application, 
//		so audio manager cannot managemnet audio ownership.
//		If your system is not requried depth of audio path, using this way.
//		Do not use this method if an interrupt occurs during audio service like a BlueTooth
//		and you need to go back to the original service.
//



//
//	Client Side	APIs
//

//
//	Function : DAAM_RequestOwner
//	Returns : Audio Status
int32_t DAAM_RequestOwner( int32_t pathID );

//
//	Function : DAAM_GetOwner
//	Returns : 0(success) or -1(fail)
//
int32_t DAAM_GetOwner( int32_t pathID );

//
//	Function : DAAM_ReleaseOwner
//	Returns : 0(success) or -1(fail)
//
int32_t DAAM_ReleaseOwner( int32_t pathID );

//
//	Function : DAAM_GetStatus
//	Returns : Audio Status
//
int32_t DAAM_GetStatus( int32_t pathID );


//============================================================================
//
//							Server Side APIs
//
//============================================================================

//
//	Functions :
//	Parameters : configFile
//
void NXDA_StartAudioManager( const char *configFile );
void NXDA_StopAudioManager();
//
//============================================================================


#ifdef __cplusplus
}
#endif

#endif	//	__LIBAUDMANAGER_H__
