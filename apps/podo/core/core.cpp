#include "core.h"
#include "global.h"
#include <QDir>
#include <QProcess>
#include <QDebug>

Core::Core(QObject* parent)
    : QObject(parent)
{
    initPACPServer();
}

Core::~Core()
{
    m_pacpServer->deleteLater();
}

void Core::execute()
{
    QString appsPath = qgetenv("PODO_APPS_PATH");

    if (appsPath.isEmpty())
        appsPath = PD_DEFAULT_APPS_PATH;

    QString startApp = qgetenv("PODO_START_APP");

    if (startApp.isEmpty())
    {
        QString platformName = qgetenv("QT_QPA_PLATFORM");
        startApp = PD_DEFAULT_START_APP;
        if(platformName.startsWith("wayland"))
        {
            startApp = PD_DEFAULT_START_WAYLAND;
        }
    }

    QString startAppPath = appsPath + "/" + startApp;

    QDir dir(startAppPath);

    if (!dir.exists(startAppPath))
    {
        qWarning() << "Core::exec not found start app path:" << startAppPath;
        return;
    }

    QString execPath = startAppPath + "/" + startApp;
#ifdef Q_OS_LINUX
#if QT_VERSION >= 0x050000
    execPath = execPath + " -platform eglfs";
#else
    execPath = execPath + " -qws";
#endif
#endif

    qDebug() << "Core::exec run processor:" << execPath;

    QProcess::startDetached(execPath);
}

void Core::initPACPServer()
{
    m_pacpServer = new PACPServer;
    m_pacpServer->setServerName("podonamu");
}
