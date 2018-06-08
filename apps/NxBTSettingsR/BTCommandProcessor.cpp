#include <QStringList>
#include "BTCommandProcessor.h"

#ifndef BUFFER_SIZE
#   define BUFFER_SIZE 1024
#endif /* BUFFER_SIZE */

#ifndef DEFAULT_SERVER_PATH
#   define DEFAULT_SERVER_PATH              "/tmp/nexell.daudio.bluetooth.service"
#endif /* DEFAULT_SERVER_PATH */

#ifndef DEFAULT_CLIENT_PATH_FOR_AUDIO
#   define DEFAULT_CLIENT_PATH_FOR_AUDIO    "/tmp/nexell.daudio.bluetooth.client.audio"
#endif

#ifndef DEFAULT_CLIENT_PATH_FOR_PHONE
#   define DEFAULT_CLIENT_PATH_FOR_PHONE    "/tmp/nexell.daudio.bluetooth.client.phone"
#endif

#ifndef DEFAULT_CLIENT_PATH_FOR_SETTINGS
#   define DEFAULT_CLIENT_PATH_FOR_SETTINGS "/tmp/nexell.daudio.bluetooth.client.settings"
#endif

BTCommandProcessor* BTCommandProcessor::m_pInstance = NULL;

BTCommandProcessor::BTCommandProcessor()
{
    LOGQ(Q_FUNC_INFO);
}

BTCommandProcessor::~BTCommandProcessor()
{
    LOGQ(Q_FUNC_INFO);
    if (isRunning()) {
        m_IPCClient.stop();

        wait();
        quit();
    }
}

BTCommandProcessor* BTCommandProcessor::instance()
{
    LOGQ(Q_FUNC_INFO);
    if (!m_pInstance) {
        m_pInstance = new BTCommandProcessor();
    }

    return m_pInstance;
}

void BTCommandProcessor::run()
{
    char buffer[BUFFER_SIZE];

    if (!m_IPCClient.isRunning())
        m_IPCClient.start((char*)DEFAULT_CLIENT_PATH_FOR_SETTINGS, (char*)DEFAULT_SERVER_PATH);

    while (m_IPCClient.isRunning()) {
        // 1. wait for read from server.
        m_IPCClient.read(buffer, BUFFER_SIZE);

        emit signalCommandFromServer(QString("%1").arg(buffer));
    }
}

void BTCommandProcessor::slotCommandToServer(QString command)
{
    commandToServer(command);
}

void BTCommandProcessor::commandToServer(QString command)
{
    LOGQ(QString("%1\t%2").arg(Q_FUNC_INFO).arg(command));
    std::string s_command = command.toStdString();
    char c_command[s_command.size()+1]; // + 1 null character
    strcpy(c_command, s_command.c_str());
    m_IPCClient.write(c_command);
}

void BTCommandProcessor::commandListToServer(QStringList commands)
{
    foreach (QString command, commands) {
        LOGQ(QString("%1\t%2").arg(Q_FUNC_INFO).arg(command));
        commandToServer(command);
    }
}
