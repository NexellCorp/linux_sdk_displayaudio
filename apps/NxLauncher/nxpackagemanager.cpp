#include "nxpackagemanager.h"
#include <QDir>
#include <QSettings>
#include <QLocale>

#define LOG_TAG "[NxLauncher]"
#include <NX_Log.h>

NxPackageScanner::NxPackageScanner(QString Path, QMap<QString, NxPluginInfo*> *pPlugins)
{
	m_pWatcher = new QFileSystemWatcher(this);
	connect(m_pWatcher, SIGNAL(fileChanged(QString)), this, SLOT(slotFileChanged(QString)));

	m_pPlugins = pPlugins;
	Scan(Path);
}

NxPackageScanner::~NxPackageScanner()
{

}

void NxPackageScanner::slotFileChanged(QString path)
{
	FillPluginInfo(QFileInfo(path), true);

	NXLOGI("[%s] path = %s", __FUNCTION__, path.toStdString().c_str());
	emit signalPlugInUpdated(path);
}

void NxPackageScanner::Scan(QString Path)
{
	QDir dir(Path);
	foreach (QFileInfo e, dir.entryInfoList(QDir::Dirs|QDir::NoDotAndDotDot, QDir::Name))
	{
		QDir sub(e.filePath());
		QFileInfoList entries = sub.entryInfoList(QStringList() << "*.desktop", QDir::Files);
		if (entries.size())
		{
			FillPluginInfo(entries.at(0), false);
		}
	}
}

void NxPackageScanner::FillPluginInfo(const QFileInfo& szInfo, bool bInitialized)
{
	QString szLang = QLocale::system().bcp47Name();

	QSettings settings(szInfo.filePath(), QSettings::IniFormat);
	settings.setIniCodec("UTF-8");
	settings.beginGroup("Desktop Entry");

	QStringList keys = settings.allKeys();
	NxPluginInfo *psInfo = m_pPlugins->value(szInfo.dir().dirName());
	if (!psInfo)
		psInfo = new NxPluginInfo();

	psInfo->setPath(szInfo.absolutePath());
	psInfo->setEnabled(true);
	psInfo->setVisible(true);
	psInfo->setAutoStart(false);

	for( int idx = 0; idx < keys.size(); idx++ )
	{
		QString key = keys.at(idx);
		QString lowerKey = key.toLower();

		if( lowerKey == "type" )
		{
			psInfo->setType(settings.value(key).toString());
		}
		else if( lowerKey == "version" )
		{
			psInfo->setVersion( settings.value(key).toString() );
		}
		else if( lowerKey == "tryexec" )
		{
			psInfo->setTryExec( settings.value(key).toString() );
		}
		else if( lowerKey == "exec" )
		{
			psInfo->setExec( settings.value(key).toString() );
		}
		else if( lowerKey == QString("name[%1]").arg(szLang) ||
				 lowerKey == "name" )
		{
			psInfo->setName( settings.value(key).toString() );
		}
		else if( lowerKey == QString("comment[%1]").arg(szLang) ||
				 lowerKey == "comment" )
		{
			psInfo->setComment( settings.value(key).toString() );
		}
		else if( lowerKey == QString("icon[%1]").arg(szLang) ||
				 lowerKey == "icon" )
		{
			psInfo->setIcon( settings.value(key).toString() );
		}
		else if( lowerKey == "watch")
		{
			if (settings.value(key).toString().toLower() == "yes") {
				if (!m_pWatcher->files().contains(szInfo.absoluteFilePath()))
					m_pWatcher->addPath(szInfo.absoluteFilePath());
			} else {
				if (m_pWatcher->files().contains(szInfo.absoluteFilePath()))
					m_pWatcher->removePath(szInfo.absoluteFilePath());
			}
		}
		else if (bInitialized && lowerKey == "state")
		{
			psInfo->setEnabled(settings.value(key, "active").toString().toLower() == "active");
		}
		else if (!bInitialized && lowerKey == "default")
		{
			psInfo->setEnabled(settings.value(key, "active").toString().toLower() == "active");
		}
		else if (lowerKey == "title")
		{
			psInfo->setTitle(settings.value(key).toString());
		}
		else if (lowerKey == "autostart")
		{
			psInfo->setAutoStart(settings.value(key, "no").toString().toLower() == "yes");
		}
		else if (lowerKey == "visible")
		{
			psInfo->setVisible(settings.value(key, "yes").toString().toLower() == "yes");
		}
	}

	m_pPlugins->insert(szInfo.dir().dirName(), psInfo);
}
