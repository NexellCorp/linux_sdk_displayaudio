#include "pacpserver.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QMutexLocker>
#include <QPoint>
#include <QVariant>
#include "pacpprotocol.h"
#include <QDebug>

PACPServer::PACPServer(QObject *parent) : QObject(parent)
{
    m_server = new QLocalServer(this);
    connect(m_server, SIGNAL(newConnection()), this, SLOT(newConnection()));
}

PACPServer::~PACPServer()
{
    m_localSocketMultiMap.clear();
    m_server->close();
    m_server->deleteLater();
    m_server=0;
}

bool PACPServer::setServerName(const QString &name)
{
    if(QFile::exists("/tmp/"+name))
        QFile::remove("/tmp/"+name);
    if (!m_server->listen(name))
    {
        qDebug() << "PACPServer::setServerName openFailed errorString: "<<m_server->errorString();
        m_server->close();
        return false;
    }
    qDebug() << "PACPServer::setServerName connected serverName: "<<m_server->serverName();
    return true;
}

void PACPServer::newConnection()
{
    QLocalSocket *clientConnection = m_server->nextPendingConnection();
    connect(clientConnection, SIGNAL(disconnected()), this, SLOT(disconnectedFromClient()));
    connect(clientConnection, SIGNAL(readyRead()), this, SLOT(readyReadFromClient()));
}

void PACPServer::readyReadFromClient()
{
    QLocalSocket* socket = (QLocalSocket*)sender();

    QByteArray readAll = socket->readAll();
    while(!readAll.isEmpty())
    {
        int lastIndex = readAll.lastIndexOf("{");
        QByteArray readRight = readAll.right(readAll.length()-lastIndex);
        readAll = readAll.left(lastIndex);
        PACPCommand *command = PACPCommand::parserJson(readRight);
        if(command==NULL)
            return;
        switch (command->getCommuniType())
        {
        case PACPCommand::CONNECT_APP:
        {
            m_localSocketMultiMap.insert(command->getSender(),socket);
            break;
        }
        case PACPCommand::DISCONNECT_APP:
        {
            m_localSocketMultiMap.remove(command->getSender(),socket);
            socket->disconnectFromServer();
            break;
        }
        case PACPCommand::APP_TO_APP:
        {
            QString receiver = command->getReceiver();
            foreach (QLocalSocket* receiverSocket, m_localSocketMultiMap.values(receiver))
            {
                receiverSocket->write(readRight);
            }
            break;
        }
        case PACPCommand::APP_TO_APPS:
        {
            foreach (QLocalSocket* receiverSocket, m_localSocketMultiMap.values())
            {
                receiverSocket->write(readRight);
            }
            break;
        }
        case PACPCommand::CONNECT_PDCOMPOSITOR:
        {
            m_pdcompositorName = command->getSender();
            m_localSocketMultiMap.insert(command->getSender(),socket);
            break;
        }
        case PACPCommand::CONNECT_PDLAUNCHER:
        {
            m_pdlauncherName = command->getSender();
            m_localSocketMultiMap.insert(command->getSender(),socket);
            break;
        }
        case PACPCommand::APP_TO_PDCOMPOSITOR:
        {
            foreach (QLocalSocket* receiverSocket, m_localSocketMultiMap.values(m_pdcompositorName))
            {
                receiverSocket->write(readRight);
            }
            break;
        }
        default:
            break;
        }
    }
}

void PACPServer::disconnectedFromClient()
{
    QLocalSocket* socket = (QLocalSocket*)sender();
    QString key = m_localSocketMultiMap.key(socket);
    m_localSocketMultiMap.remove(key,socket);
    qDebug() << "PACPServer::disconnectedFromClient clientName :" << key;
}
