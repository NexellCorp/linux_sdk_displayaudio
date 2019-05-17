#include "CNX_MetaDataReader.h"
#include <unistd.h>

CNX_MetaDataReader::CNX_MetaDataReader()
{

}

void CNX_MetaDataReader::Read(QString path, QString coverArtPath)
{
	QMimeDatabase db;
	QMimeType mimeType = db.mimeTypeForFile(QFileInfo(path));

	ReadTag(path.toStdString().c_str());

	if (mimeType.name() == "audio/mpeg")
	{
		ReadCoverArt_MP3(path.toStdString().c_str(), coverArtPath.toStdString().c_str());
	}
	else if (mimeType.name() == "audio/flac")
	{
		ReadCoverArt_FLAC(path.toStdString().c_str(), coverArtPath.toStdString().c_str());
	}
	else if (mimeType.name() == "audio/x-vorbis+ogg")
	{
		ReadCoverArt_OGG(path.toStdString().c_str(), coverArtPath.toStdString().c_str());
	}
	else if (mimeType.name() == "audio/x-ms-wma")
	{
		ReadCoverArt_WMA(path.toStdString().c_str(), coverArtPath.toStdString().c_str());
	}

}

void CNX_MetaDataReader::ReadTag(const char* path)
{
	TagLib::FileRef f(path);
	TagLib::Tag *tag = f.tag();
	m_Title = tag->title().toCString(true);
	m_Album = tag->album().toCString(true);
	m_Artist = tag->artist().toCString(true);
	m_Genre = tag->genre().toCString(true);
	m_uiTrackNumber = tag->track();
}

void CNX_MetaDataReader::ReadCoverArt_MP3(const char* path, const char* coverArtPath)
{
	TagLib::MPEG::File f(path);
	TagLib::ID3v2::Tag *tag = f.ID3v2Tag();

	TagLib::ID3v2::FrameList frames = tag->frameListMap()["APIC"];
	TagLib::ID3v2::AttachedPictureFrame *pPicture;

	if (!frames.isEmpty())
	{
		TagLib::ID3v2::FrameList::ConstIterator it= frames.begin();
		for ( ; it != frames.end(); it++)
		{
			pPicture = static_cast<TagLib::ID3v2::AttachedPictureFrame *>(*it);
			FILE *fp = fopen(coverArtPath, "wb");
			fwrite(pPicture->picture().data(), pPicture->picture().size(), 1, fp);
			fclose(fp);
			sync();
		}
	}

	m_bExistCoverArt = !frames.isEmpty();
}

void CNX_MetaDataReader::ReadCoverArt_OGG(const char* path, const char* coverArtPath)
{
	TagLib::Ogg::Vorbis::File f(path);
	TagLib::Ogg::XiphComment *tag = f.tag();
	TagLib::List<TagLib::FLAC::Picture *> pictures = tag->pictureList();
	TagLib::List<TagLib::FLAC::Picture *>::ConstIterator it = pictures.begin();
	TagLib::FLAC::Picture *pPicture;

	for ( ; it != pictures.end(); it++)
	{
		pPicture = static_cast<TagLib::FLAC::Picture *>(*it);
		FILE *fp = fopen(coverArtPath, "wb");
		fwrite(pPicture->data().data(), pPicture->data().size(), 1, fp);
		fclose(fp);
		sync();
	}

	m_bExistCoverArt = (pictures.size() > 0);
}

void CNX_MetaDataReader::ReadCoverArt_FLAC(const char* path, const char* coverArtPath)
{
	TagLib::FLAC::File f(path);
	TagLib::List<TagLib::FLAC::Picture *> pictures = f.pictureList();
	TagLib::List<TagLib::FLAC::Picture *>::ConstIterator it = pictures.begin();
	TagLib::FLAC::Picture *pPicture;

	for ( ; it != pictures.end(); it++)
	{
		pPicture = static_cast<TagLib::FLAC::Picture *>(*it);
		FILE *fp = fopen(coverArtPath, "wb");
		fwrite(pPicture->data().data(), pPicture->data().size(), 1, fp);
		fclose(fp);
		sync();
	}

	m_bExistCoverArt = (pictures.size() > 0);
}

void CNX_MetaDataReader::ReadCoverArt_WMA(const char* path, const char* coverArtPath)
{
	TagLib::ASF::File f(path);
	TagLib::ASF::Tag *tag = f.tag();

	TagLib::ASF::AttributeList attr = tag->attribute("WM/Picture");
	if (attr.size() > 0)
	{
		TagLib::ASF::Picture p = attr[0].toPicture();
		FILE *fp = fopen(coverArtPath, "wb");
		fwrite(p.picture().data(), p.picture().size(), 1, fp);
		fclose(fp);
		sync();
	}

	m_bExistCoverArt = (attr.size() > 0);
}

QString CNX_MetaDataReader::GetTitle()
{
	return m_Title;
}

QString CNX_MetaDataReader::GetAlbum()
{
	return m_Album;
}

QString CNX_MetaDataReader::GetArtist()
{
	return m_Artist;
}

QString CNX_MetaDataReader::GetGenre()
{
	return m_Genre;
}

unsigned int CNX_MetaDataReader::GetTrackNumber()
{
	return m_uiTrackNumber;
}

bool CNX_MetaDataReader::IsExistCoverArt()
{
	return m_bExistCoverArt;
}
