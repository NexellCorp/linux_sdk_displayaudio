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

#include "CNX_FileList.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h> /* for stat() */

#include <QVector>
#include <QString>

//#define DEBUG

CNX_FileList::CNX_FileList()
{
	m_bIsMadeByLocal = false;
}

CNX_FileList::~CNX_FileList()
{
}

int CNX_FileList::IsDir(QString path)
{
	struct stat stat_buf;
	stat( path.toStdString().c_str(), &stat_buf);
	return ( S_ISDIR( stat_buf.st_mode) ? 1: 0);
}

int32_t CNX_FileList::IsNormalFile(QString path)
{
	struct stat stat_buf;
	stat( path.toStdString().c_str(), &stat_buf);
	return ( S_ISREG( stat_buf.st_mode) ? 1: 0);
}

int CNX_FileList::GetDir (QString dir)
{
	DIR *dp;
	struct dirent *dirp;
	if((dp  = opendir(dir.toStdString().c_str())) == NULL) {
		return errno;
	}

	while ((dirp = readdir(dp)) != NULL)
	{
		if ( strcmp (dirp->d_name, ".")  && strcmp(dirp->d_name, "..") )
		{
			QString tmpPath = dir + QString("/") + QString(dirp->d_name);
			if( IsDir(tmpPath) )
			{
#if 0	//	Linux Trash Folder
				if( strcmp(dirp->d_name,".Trash-1000") )
					GetDir( tmpPath );
#endif
				GetDir( tmpPath );
			}
			else if( IsNormalFile(tmpPath) )
			{
				char *extensionPos = NULL;

				extensionPos = strrchr( dirp->d_name, '.');
				//	Filter have no extension file
				if( extensionPos == NULL )
					continue;

				for( int32_t i=0; i<m_iNumExtension ; i++ )
				{
					if( !strcasecmp((const char *)m_ppExtentsions[i], (const char *)extensionPos) )
					{
						m_FileList.push_back(tmpPath.toStdString().c_str());
						break;
					}
				}
			}
		}
	}
	closedir(dp);
	return 0;
}


void CNX_FileList::MakeFileList(const char *pDir, const char **ppExtension, int32_t numExtension)
{
	QMutexLocker lock(&m_ListLock);
	m_bIsMadeByLocal = true;
	m_FileList.clear();
	m_iNumExtension =numExtension;
	m_ppExtentsions = ppExtension;
	GetDir( pDir );

#ifdef DEBUG
	for ( int32_t i = 0;i < m_FileList.size();i++) {
		NXLOGV( "%04d. %s\n",  m_FileList[i].toStdString().c_str());
	}
#endif

}
