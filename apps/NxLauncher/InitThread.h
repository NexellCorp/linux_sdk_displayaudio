#ifndef INITTHREAD_H
#define INITTHREAD_H

#include <QThread>
#include <QMap>
#include "nxappinfo.h"

class InitThread : public QThread
{
	Q_OBJECT

signals:
	void signalInit(QString plugin);

public:
	InitThread(QMap<QString, NxPluginInfo*> plugins);

protected:
	void run();

private:
	QMap<QString, NxPluginInfo*> m_Plugins;
};

#endif // INITTHREAD_H
