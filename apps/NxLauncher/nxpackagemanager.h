#ifndef NXPACKAGEMANAGER_H
#define NXPACKAGEMANAGER_H

#include <QString>
#include <QMap>
#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QObject>
#include "nxappinfo.h"

class NxPackageScanner : public QObject
{
	Q_OBJECT

private slots:
	void slotFileChanged(QString Path);

public:
	explicit NxPackageScanner(QString Path, QMap<QString, NxPluginInfo*> *pPlugins);
	~NxPackageScanner();

private:
	void Scan(QString Path);

	void FillPluginInfo(const QFileInfo& Info, bool bInitialized);

private:
	QMap<QString, NxPluginInfo*> *m_pPlugins;

	QFileSystemWatcher *m_pWatcher;
};

#endif // NXPACKAGEMANAGER_H
