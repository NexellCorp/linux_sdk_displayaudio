#ifndef NXPACKAGEMANAGER_H
#define NXPACKAGEMANAGER_H

#include <QObject>
#include <QVariantList>
#include <QFileSystemWatcher>
#include "nxappinfo.h"

class NxPackageManager : public QObject
{
	Q_OBJECT
#ifdef CONFIG_USE_NO_QML
signals:
	void signalStateChanged(NxAppInfo*);

private slots:
	void slotFileChanged(QString path);
#endif
public:
    explicit NxPackageManager(QObject *parent = 0, QString szPath = "./");
    ~NxPackageManager();

public:
    NxAppInfoList getAppInfoList();
    NxAppInfoMap getAppInfoMap();

    QString getAppType(const QString& szName);
    QString getAppExec(const QString& szName);

    Q_INVOKABLE void reload();
    Q_INVOKABLE QVariantList getAppInfoVariantList();
    Q_INVOKABLE QVariant getAppInfoVariant(const QString& szKey);

private:
    void load( QString szPath );
    void clear();

    NxAppInfo* loadAppInfo(const QString& szInfo);

	void fillAppInfo(const QString &szInfo, NxAppInfo* pInfo);

private:
	NxAppInfoList	m_AppInfoList;
	NxAppInfoMap	m_AppInfoMap;
    QString m_szPath;
#ifdef CONFIG_USE_NO_QML
	QFileSystemWatcher* m_pWatcher;
#endif
};

#endif // NXPACKAGEMANAGER_H
