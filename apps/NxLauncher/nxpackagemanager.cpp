#include "nxpackagemanager.h"

#include <QDirIterator>
#include <QDebug>
#include <QFileInfo>
#include <QSettings>

NxPackageManager::NxPackageManager(QObject *parent, QString szPath)
	: QObject( parent )
	, m_szPath( szPath )
{
#ifdef CONFIG_USE_NO_QML
	m_pWatcher = new QFileSystemWatcher(this);
	connect(m_pWatcher, SIGNAL(fileChanged(QString)), this, SLOT(slotFileChanged(QString)));
	m_pWatcher->addPath(m_szPath);
#endif
	load( m_szPath );
}

NxPackageManager::~NxPackageManager()
{
#ifdef CONFIG_USE_NO_QML
	delete m_pWatcher;
#endif
}

NxAppInfoList NxPackageManager::getAppInfoList()
{
	return m_AppInfoList;
}

NxAppInfoMap NxPackageManager::getAppInfoMap()
{
	return m_AppInfoMap;
}

QString NxPackageManager::getAppType(const QString& szName)
{
	NxAppInfo *pAppInfo = m_AppInfoMap.value( szName, 0 );
	return pAppInfo ? pAppInfo->getType() : "";
}

QString NxPackageManager::getAppExec(const QString& szName)
{
	NxAppInfo *pAppInfo = m_AppInfoMap.value( szName, 0 );
	return pAppInfo ? pAppInfo->getPath() + "/" + pAppInfo->getExec() : "";
}

void NxPackageManager::reload()
{
	clear();
	load( m_szPath );
}

QVariantList NxPackageManager::getAppInfoVariantList()
{
	QVariantList infoVariantList;
	foreach(NxAppInfo* pInfo, m_AppInfoList)
	{
		infoVariantList.append( qVariantFromValue((QObject*)pInfo) );
	}

	return infoVariantList;
}

QVariant NxPackageManager::getAppInfoVariant(const QString& szName)
{
	QVariant infoVariant;
	if( m_AppInfoMap.contains(szName) )
	{
		infoVariant = qVariantFromValue( (QObject*)m_AppInfoMap.value(szName) );
	}

	return infoVariant;
}

void NxPackageManager::load( QString szPath )
{
	QStringList szInfoList;
	QStringList szFilters;

	szFilters += "*.desktop";
	QDirIterator dirIterator(szPath, szFilters, QDir::Files | QDir::NoSymLinks, QDirIterator::Subdirectories);

	while( dirIterator.hasNext() )
	{
		dirIterator.next();
		szInfoList += dirIterator.filePath();
	}

	for( int i = 0; i < szInfoList.length(); i++ )
	{
		QString szInfo = szInfoList.at(i);
		NxAppInfo *pAppInfo = loadAppInfo( szInfo );

		m_AppInfoList.push_back( pAppInfo );
		m_AppInfoMap[pAppInfo->getName()] = pAppInfo;

#if 0
		qDebug() << "------------------------------------------------------------";
		qDebug() << "info:"      << szInfo;
		qDebug() << "path: "     << pAppInfo->getPath();
		qDebug() << "exec:"      << pAppInfo->getExec();
		qDebug() << "name:"      << pAppInfo->getName();
		qDebug() << "type:"      << pAppInfo->getType();
		qDebug() << "version:"   << pAppInfo->getVersion();
		qDebug() << "comment:"   << pAppInfo->getComment();
		qDebug() << "icon:"      << pAppInfo->getIcon();
		qDebug() << "tryexec:"   << pAppInfo->getTryExec();
		qDebug() << "------------------------------------------------------------";
#endif
	}
}

void NxPackageManager::clear()
{
	qDeleteAll( m_AppInfoList );
	m_AppInfoList.clear();
	m_AppInfoMap.clear();
#ifdef CONFIG_USE_NO_QML
	m_pWatcher->removePaths(m_pWatcher->files());
#endif
}

void NxPackageManager::fillAppInfo(const QString &szInfo, NxAppInfo* pInfo)
{
	QString szLang = QLocale::system().bcp47Name();

	QSettings settings( szInfo, QSettings::IniFormat );
	settings.setIniCodec("UTF-8");
	settings.beginGroup("Desktop Entry");

	QStringList keys = settings.allKeys();

	QFileInfo fileInfo(szInfo);
	pInfo->setPath( fileInfo.absolutePath() );
	pInfo->setEnabled(true);

	for( int idx = 0; idx < keys.size(); idx++ )
	{
		QString key = keys.at(idx);
		QString lowerKey = key.toLower();

		if( lowerKey == "type" )
		{
			pInfo->setType( settings.value(key).toString() );
		}
		else if( lowerKey == "version" )
		{
			pInfo->setVersion( settings.value(key).toString() );
		}
		else if( lowerKey == "tryexec" )
		{
			pInfo->setTryExec( settings.value(key).toString() );
		}
		else if( lowerKey == "exec" )
		{
			pInfo->setExec( settings.value(key).toString() );
		}
		else if( lowerKey == QString("name[%1]").arg(szLang) ||
				 lowerKey == "name" )
		{
			pInfo->setName( settings.value(key).toString() );
		}
		else if( lowerKey == QString("comment[%1]").arg(szLang) ||
				 lowerKey == "comment" )
		{
			pInfo->setComment( settings.value(key).toString() );
		}
		else if( lowerKey == QString("icon[%1]").arg(szLang) ||
				 lowerKey == "icon" )
		{
			pInfo->setIcon( settings.value(key).toString() );
		}
#ifdef CONFIG_USE_NO_QML
		else if( lowerKey == "watch")
		{
			if (settings.value(key).toString().toLower() == "yes") {
				if (!m_pWatcher->files().contains(fileInfo.absoluteFilePath()))
					m_pWatcher->addPath(fileInfo.absoluteFilePath());
			} else {
				if (m_pWatcher->files().contains(fileInfo.absoluteFilePath()))
					m_pWatcher->removePath(fileInfo.absoluteFilePath());
			}
		}
		else if( lowerKey == "state")
		{
			pInfo->setEnabled(settings.value(key, "active").toString().toLower() == "active");
		}
#endif
		else
		{
			// qWarning() << "Unknown key. : " << key;
			continue;
		}

	}
}

NxAppInfo *NxPackageManager::loadAppInfo( const QString &szInfo )
{
	NxAppInfo *pAppInfo = new NxAppInfo();
	fillAppInfo(szInfo, pAppInfo);
	return pAppInfo;
}
#ifdef CONFIG_USE_NO_QML
void NxPackageManager::slotFileChanged(QString path)
{
	if (!QFile::exists(path)) {
		m_pWatcher->removePath(path);
		return;
	}

	for (int i = 0; i < m_AppInfoList.size(); ++i) {
		if (path.indexOf(m_AppInfoList[i]->getPath()) == 0) {
			bool enabled = m_AppInfoList[i]->getEnabled();
			fillAppInfo(path, m_AppInfoList[i]);
			m_AppInfoMap[m_AppInfoList[i]->getName()] = m_AppInfoList[i];
			if (enabled != m_AppInfoList[i]->getEnabled())
				emit signalStateChanged(m_AppInfoList[i]);
			break;
		}
	}
}
#endif
