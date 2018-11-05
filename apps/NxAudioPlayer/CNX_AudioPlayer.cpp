#include "CNX_AudioPlayer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#define LOG_TAG "[NxAudioPlayer]"
#include <NX_Log.h>

#define AUDIO_DEFAULT_DEVICE "plughw:0,0"
#define AUDIO_HDMI_DEVICE    "plughw:0,3"

//------------------------------------------------------------------------------
CNX_AudioPlayer::CNX_AudioPlayer()
	: m_hPlayer( NULL )
	, m_iStatus( StoppedState )
	, m_iMediaType( 0 )
{
	pthread_mutex_init( &m_hLock, NULL );
	memset(&m_MediaInfo, 0x00, sizeof(MP_MEDIA_INFO));
}

CNX_AudioPlayer::~CNX_AudioPlayer()
{
	pthread_mutex_destroy( &m_hLock );
}


//================================================================================================================
//================================================================================================================
//public methods	commomn Initialize , close
int32_t CNX_AudioPlayer::InitMediaPlayer(	void (*pCbEventCallback)( void *privateDesc, uint32_t EventType, uint32_t /*EventData*/, uint32_t /*param*/ ),
										void *pCbPrivate,
										const char *pUri,
                                        int32_t mediaType,
										void (*pCbQtUpdateImg)(void *pImg)
									)
{
	CNX_AutoLock lock( &m_hLock );

	if(0 > OpenHandle(pCbEventCallback, pCbPrivate) )		return -1;
	if(0 > SetUri(pUri) )									return -1;
	if(0 > GetMediaInfo() )									return -1;
	if(0 > AddTrack(mediaType) )							return -1;
	//PrintMediaInfo(pUri);
	m_iStatus = ReadyState;
	return 0;
}

int32_t CNX_AudioPlayer::CloseHandle()
{
	CNX_AutoLock lock( &m_hLock );
	if( NULL == m_hPlayer )
	{
		NXLOGE( "%s: Error! Handle is not initialized!", __FUNCTION__ );
		return -1;
	}

	NX_MPClose( m_hPlayer );

	m_hPlayer = NULL;

	m_iStatus = StoppedState;
	return 0;
}

//================================================================================================================
//================================================================================================================
//public methods	common Control
int32_t CNX_AudioPlayer::SetVolume(int32_t volume)
{
	CNX_AutoLock lock( &m_hLock );
	if( NULL == m_hPlayer )
	{
		NXLOGE( "%s: Error! Handle is not initialized!", __FUNCTION__ );
		return -1;
	}

	MP_RESULT iResult = NX_MPSetVolume( m_hPlayer, volume );
	if( MP_ERR_NONE != iResult )
	{
		NXLOGE( "%s(): Error! NX_MPSetVolume() Failed! (ret = %d)", __FUNCTION__, iResult);
		return -1;
	}
	return 0;
}

int32_t CNX_AudioPlayer::Play()
{
	CNX_AutoLock lock( &m_hLock );
	if( NULL == m_hPlayer )
	{
		NXLOGE( "%s: Error! Handle is not initialized!", __FUNCTION__ );
		return -1;
	}
	MP_RESULT iResult = NX_MPPlay( m_hPlayer );
	if( MP_ERR_NONE != iResult )
	{
		NXLOGE( "%s(): Error! NX_MPPlay() Failed! (ret = %d)", __FUNCTION__, iResult);
		return -1;
	}

	m_iStatus = PlayingState;
	return 0;
}

int32_t CNX_AudioPlayer::Seek(qint64 position)
{
	CNX_AutoLock lock( &m_hLock );
	if( NULL == m_hPlayer )
	{
		NXLOGE( "%s: Error! Handle is not initialized!", __FUNCTION__ );
		return -1;
	}
	MP_RESULT iResult = NX_MPSeek( m_hPlayer, position );
	if( MP_ERR_NONE != iResult )
	{
		NXLOGE( "%s(): Error! NX_MPSeek() Failed! (ret = %d)", __FUNCTION__, iResult);
		return -1;
	}
	return 0;
}

int32_t CNX_AudioPlayer::Pause()
{
	CNX_AutoLock lock( &m_hLock );
	if( NULL == m_hPlayer )
	{
		NXLOGE( "%s: Error! Handle is not initialized!", __FUNCTION__ );
		return -1;
	}

	MP_RESULT iResult = NX_MPPause( m_hPlayer );
	if( MP_ERR_NONE != iResult )
	{
		NXLOGE("%s(): Error! NX_MPPause() Failed! (ret = %d)", __FUNCTION__, iResult);
		return -1;
	}

	m_iStatus = PausedState;
	return 0;
}

int32_t CNX_AudioPlayer::Stop()
{
	CNX_AutoLock lock( &m_hLock );
	if( NULL == m_hPlayer )
	{
		NXLOGE( "%s: Error! Handle is not initialized!", __FUNCTION__ );
		return -1;
	}
		MP_RESULT iResult = NX_MPStop( m_hPlayer );
	if( MP_ERR_NONE != iResult )
	{
		NXLOGE( "%s(): Error! NX_MPStop() Failed! (ret = %d)", __FUNCTION__, iResult);
		return -1;
	}
	m_iStatus = StoppedState;
	return 0;
}

//================================================================================================================
//================================================================================================================
//public methods	common information
qint64 CNX_AudioPlayer::GetMediaPosition()
{
	CNX_AutoLock lock( &m_hLock );
    if( NULL == m_hPlayer )
	{
		NXLOGE( "%s: Error! Handle is not initialized!", __FUNCTION__ );
		return -1;
	}

	int64_t iPosition;
	MP_RESULT iResult = NX_MPGetPosition( m_hPlayer, &iPosition );
	if( MP_ERR_NONE != iResult )
	{
		NXLOGE( "%s(): Error! NX_MPGetPosition() Failed! (ret = %d)", __FUNCTION__, iResult);
		return -1;
	}
	return (qint64)iPosition;
}

qint64 CNX_AudioPlayer::GetMediaDuration()
{
	CNX_AutoLock lock( &m_hLock );
	if( NULL == m_hPlayer )
	{
		NXLOGE( "%s: Error! Handle is not initialized!", __FUNCTION__ );
		return -1;
	}

	int64_t duration;
	MP_RESULT iResult = NX_MPGetDuration( m_hPlayer, &duration );
	if( MP_ERR_NONE != iResult )
	{
		NXLOGE( "%s(): Error! NX_MPGetDuration() Failed! (ret = %d)", __FUNCTION__, iResult);
		return -1;
	}
	return (qint64)duration;
}

NX_MediaStatus CNX_AudioPlayer::GetState()
{
	CNX_AutoLock lock( &m_hLock );
	if( NULL == m_hPlayer )
	{
		return StoppedState;
	}
	return m_iStatus;
}

void CNX_AudioPlayer::PrintMediaInfo( const char *pUri )
{

	NXLOGD("####################################################################################################\n");
	NXLOGD( "FileName : %s\n", pUri );
	NXLOGD( "Media Info : Program( %d EA ), Video( %d EA ), Audio( %d EA ), Subtitle( %d EA ), Data( %d EA )\n",
			m_MediaInfo.iProgramNum, m_MediaInfo.iVideoTrackNum, m_MediaInfo.iAudioTrackNum, m_MediaInfo.iSubTitleTrackNum, m_MediaInfo.iDataTrackNum );

	for( int32_t i = 0; i < m_MediaInfo.iProgramNum; i++ )
	{
		NXLOGD( "Program Info #%d : Video( %d EA ), Audio( %d EA ), Subtitle( %d EA ), Data( %d EA ), Duration( %lld ms )\n",
				i, m_MediaInfo.ProgramInfo[i].iVideoNum, m_MediaInfo.ProgramInfo[i].iAudioNum, m_MediaInfo.ProgramInfo[i].iSubTitleNum, m_MediaInfo.ProgramInfo[i].iDataNum, m_MediaInfo.ProgramInfo[i].iDuration);

		if( 0 < m_MediaInfo.ProgramInfo[i].iVideoNum )
		{
            int32_t num = 0;
            for( int32_t j = 0; j < m_MediaInfo.ProgramInfo[i].iVideoNum + m_MediaInfo.ProgramInfo[i].iAudioNum; j++ )
			{
				MP_TRACK_INFO *pTrackInfo = &m_MediaInfo.ProgramInfo[i].TrackInfo[j];
				if( MP_TRACK_VIDEO == pTrackInfo->iTrackType )
				{
					NXLOGD( "-. Video Track #%d : Index( %d ), Type( %d ), Resolution( %d x %d ), Duration( %lld ms )\n",
                            num++, pTrackInfo->iTrackIndex, (int32_t)pTrackInfo->iCodecId, pTrackInfo->iWidth, pTrackInfo->iHeight, pTrackInfo->iDuration );
				}
			}
		}

		if( 0 < m_MediaInfo.ProgramInfo[i].iAudioNum )
		{
            int32_t num = 0;
            for( int32_t j = 0; j < m_MediaInfo.ProgramInfo[i].iVideoNum + m_MediaInfo.ProgramInfo[i].iAudioNum; j++ )
			{
				MP_TRACK_INFO *pTrackInfo = &m_MediaInfo.ProgramInfo[i].TrackInfo[j];
				if( MP_TRACK_AUDIO == pTrackInfo->iTrackType )
				{
					NXLOGD( "-. Audio Track #%d : Index( %d ), Type( %d ), Ch( %d ), Samplerate( %d Hz ), Bitrate( %d bps ), Duration( %lld ms )\n",
							num++, pTrackInfo->iTrackIndex, (int)pTrackInfo->iCodecId, pTrackInfo->iChannels, pTrackInfo->iSampleRate, pTrackInfo->iBitrate, pTrackInfo->iDuration );
				}
			}
		}
	}
	NXLOGD( "####################################################################################################\n");
}
//================================================================================================================


//================================================================================================================
//================================================================================================================
//private methods	for InitMediaPlayer
//================================================================================================================
int32_t CNX_AudioPlayer::OpenHandle( void (*pCbEventCallback)( void *privateDesc, uint32_t EventType, uint32_t /*EventData*/, uint32_t /*param*/ ),
								void *cbPrivate )
{
	MP_RESULT iResult = NX_MPOpen( &m_hPlayer, pCbEventCallback, cbPrivate );

	if( MP_ERR_NONE != iResult )
	{
		NXLOGE( "%s: Error! Handle is not initialized!\n", __FUNCTION__ );
		return -1;
	}
	return 0;
}

int32_t CNX_AudioPlayer::SetUri(const char *pUri)
{
	if( NULL == m_hPlayer )
	{
		NXLOGE( "%s: Error! Handle is not initialized!", __FUNCTION__ );
		return -1;
	}
	MP_RESULT iResult = NX_MPSetUri( m_hPlayer, pUri );
	if( MP_ERR_NONE != iResult )
	{
		NXLOGE( "%s(): Error! NX_MPSetUri() Failed! (ret = %d, uri = %s)\n", __FUNCTION__, iResult, pUri );
		return -1;
	}
	return 0;
}

int32_t CNX_AudioPlayer::GetMediaInfo()
{
	if( NULL == m_hPlayer )
	{
		NXLOGE( "%s: Error! Handle is not initialized!", __FUNCTION__ );
		return -1;
	}
	MP_RESULT iResult = NX_MPGetMediaInfo( m_hPlayer, &m_MediaInfo );
	if( MP_ERR_NONE != iResult )
	{
		NXLOGE( "%s(): Error! NX_MPGetMediaInfo() Failed! (ret = %d)\n", __FUNCTION__, iResult );
		return -1;
	}

	return 0;
}

int32_t CNX_AudioPlayer::AddTrack(int32_t mediaType)
{
	if( NULL == m_hPlayer )
	{
		NXLOGE( "%s: Error! Handle is not initialized!", __FUNCTION__ );
		return -1;
	}
	m_iMediaType = mediaType;
    int32_t iResult = -1;

	if(MP_TRACK_AUDIO == mediaType)	iResult = AddTrackForAudio();

	if(0 > iResult ) return -1;

	return 0;
}

int32_t CNX_AudioPlayer::AddTrackForAudio()
{
	//audio
	if( m_MediaInfo.iAudioTrackNum <= 0 )
	{
		NXLOGE("Fail, this contents do not have audio track.");
		return -1;
	}
	else
	{
		// <<-- FIXED AUDIO TRACK -->>
		if( 0 > AddAudioTrack(0) )
			return -1;
	}
	return 0;
}

int32_t CNX_AudioPlayer::AddAudioTrack( int32_t track )
{
	if( NULL == m_hPlayer )
	{
		NXLOGE( "%s: Error! Handle is not initialized!", __FUNCTION__ );
		return -1;
	}

	if( track >= m_MediaInfo.iAudioTrackNum )
	{
		NXLOGE( "%s(): Error! Track Number. (track = %d / audioTrack = %d)\n", __FUNCTION__, track, m_MediaInfo.iAudioTrackNum );
		return -1;
	}

	if( track >= m_MediaInfo.ProgramInfo[0].iAudioNum )
	{
		NXLOGE( "%s(): Error! Track Number. (track = %d / audioTrack = %d)\n", __FUNCTION__, track, m_MediaInfo.ProgramInfo[0].iAudioNum );
		return -1;
	}

    int32_t index = GetTrackIndex( MP_TRACK_AUDIO, track );
	if( 0 > index )
	{
		NXLOGE(  "%s(): Error! Get Audio Index. ( track = %d )", __FUNCTION__, track );
		return -1;
	}

	MP_RESULT iResult = NX_MPAddAudioTrack( m_hPlayer, index, NULL, AUDIO_DEFAULT_DEVICE );

	if( MP_ERR_NONE != iResult )
	{
		NXLOGE( "%s(): Error! NX_MPAddTrack() Failed! (ret = %d)", __FUNCTION__, iResult);
		return -1;
	}
	return 0;
}

int32_t CNX_AudioPlayer::GetTrackIndex( int32_t trackType, int32_t track )
{
    int32_t index = -1, trackOrder = 0;

    for( int32_t i = 0; i < m_MediaInfo.iProgramNum; i++ )
	{
        for( int32_t j = 0; j < m_MediaInfo.ProgramInfo[i].iVideoNum + m_MediaInfo.ProgramInfo[i].iAudioNum; j++ )
		{
			if( trackType == m_MediaInfo.ProgramInfo[i].TrackInfo[j].iTrackType )
			{
				if( track == trackOrder )
				{
					index = m_MediaInfo.ProgramInfo[i].TrackInfo[j].iTrackIndex;
					// NXLOGD( "[%s] Require Track( %d ), Stream Index( %d )", (trackType == MP_TRACK_AUDIO) ? "AUDIO" : "VIDEO", track, index );
					return index;
				}
				trackOrder++;
			}
		}
	}

	return index;
}


//================================================================================================================

