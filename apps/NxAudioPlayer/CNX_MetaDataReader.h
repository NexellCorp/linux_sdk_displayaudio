#ifndef CNX_METADATAREADER_H
#define CNX_METADATAREADER_H

#include <QMimeDatabase>
#include <QFileInfo>
#include <taglib/tag.h>
#include <taglib/fileref.h>
#include <taglib/mpegfile.h>

#include <taglib/id3v2tag.h>
#include <taglib/id3v2frame.h>
#include <taglib/id3v2header.h>
#include <taglib/attachedpictureframe.h>

#include <taglib/mp4coverart.h>
#include <taglib/mp4file.h>

#include <taglib/flacfile.h>

#include <taglib/oggfile.h>
#include <taglib/vorbisfile.h>

#include <taglib/asffile.h>
#include <taglib/asftag.h>
#include <taglib/asfpicture.h>

class CNX_MetaDataReader
{
public:
	CNX_MetaDataReader();

	void Reset();

	void Read(QString path, QString coverArtPath);

	QString GetTitle();

	QString GetAlbum();

	QString GetArtist();

	QString GetGenre();

	unsigned int GetTrackNumber();

	bool IsExistCoverArt();

private:
	void ReadTag(const char* path);

	void ReadCoverArt_MP3(const char* path, const char* coverArtPath);

	void ReadCoverArt_OGG(const char* path, const char* coverArtPath);

	void ReadCoverArt_FLAC(const char* path, const char* coverArtPath);

	void ReadCoverArt_WMA(const char* path, const char* coverArtPath);

//	un
private:
	QString m_Title;
	QString m_Album;
	QString m_Artist;
	QString m_Genre;
	unsigned int m_uiTrackNumber;
	bool m_bExistCoverArt;
};

#endif // CNX_METADATAREADER_H
