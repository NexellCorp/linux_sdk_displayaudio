#include "InitThread.h"

InitThread::InitThread(QMap<QString, NxPluginInfo*> plugins)
{
	m_Plugins = plugins;
}

void InitThread::run()
{
	QStringList applications;
	QStringList services;

	foreach (NxPluginInfo *psInfo, m_Plugins) {
		if (psInfo->getAutoStart())
		{
			if (psInfo->getType().toLower() == "application")
			{
				applications << psInfo->getName();
			}
			else if (psInfo->getType().toLower() == "service")
			{
				services << psInfo->getName();
			}
		}
	}

	foreach (QString a, applications) {
		emit signalInit(a);
	}

	foreach (QString s, services) {
		emit signalInit(s);
	}
}
