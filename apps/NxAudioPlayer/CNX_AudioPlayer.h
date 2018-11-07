/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef CNX_AudioPlayer_H
#define CNX_AudioPlayer_H

#include <QTime>
#include <QDebug>

#include <NX_MoviePlay.h>

typedef enum
{
	StoppedState	= 0,
	PlayingState	= 1,
	PausedState		= 2,
	ReadyState		= 3,
}NX_MediaStatus;

#define SOFT_VOLUME

//------------------------------------------------------------------------------
//
//  Fucntion Lock
//
class CNX_AutoLock {
public:
	CNX_AutoLock( pthread_mutex_t *pLock )
		: m_pLock(pLock)
	{
		pthread_mutex_lock( m_pLock );
	}
	~CNX_AutoLock()
	{
		pthread_mutex_unlock( m_pLock );
	}

protected:
	pthread_mutex_t *m_pLock;

private:
	CNX_AutoLock (const CNX_AutoLock &Ref);
	CNX_AutoLock &operator=(CNX_AutoLock &Ref);
};
//------------------------------------------------------------------------------

class CNX_AudioPlayer
{

public:
    CNX_AudioPlayer();
    ~CNX_AudioPlayer();

public:
	//
	//MediaPlayer commomn Initialize , close
	//mediaType is MP_TRACK_VIDEO or MP_TRACK_AUDIO
	int InitMediaPlayer(	void (*pCbEventCallback)( void *privateDesc, unsigned int EventType, unsigned int /*EventData*/, unsigned int /*param*/ ),
							void *pCbPrivate,
							const char *pUri,
							int mediaType,
							void (*pCbQtUpdateImg)(void *pImg)
						);

	int CloseHandle();

	//
	//MediaPlayer common Control
    int SetVolume(int32_t volume);
	int Play();
	int Seek(qint64 position);
	int Pause();
	int Stop();

	//
	//MediaPlayer common information
	void PrintMediaInfo( const char* pUri );
	qint64 GetMediaPosition();
	qint64 GetMediaDuration();
	NX_MediaStatus GetState();

private:
	//
	//MediaPlayer InitMediaPlayer
    int32_t OpenHandle( void (*pCbEventCallback)( void *privateDesc, uint32_t EventType, uint32_t /*EventData*/, uint32_t /*param*/ ),
					void *cbPrivate );
    int32_t SetUri(const char *pUri);
    int32_t GetMediaInfo();
    int32_t AddTrack(int mediaType);
    int32_t AddTrackForAudio();
    int32_t AddAudioTrack( int32_t track );
    int32_t GetTrackIndex( int32_t trackType, int32_t track );

	//
	//vars
	pthread_mutex_t	m_hLock;
	MP_HANDLE		m_hPlayer;

	MP_MEDIA_INFO	m_MediaInfo;

	int				m_iMediaType;
};

#endif // CNX_AudioPlayer_H
