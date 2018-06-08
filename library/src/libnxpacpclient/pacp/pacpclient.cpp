#include "pacpclient.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QGuiApplication>
#include "global.h"

PACPProtocol::ClientType PACPClient::clientType() const
{
    return m_clientType;
}

void PACPClient::setClientType(const PACPProtocol::ClientType &clientType)
{
    m_clientType = clientType;
}

PACPClient::PACPClient(QObject *parent, bool directEvent) : QObject(parent)
{
    m_socket = new QLocalSocket(this);
    m_pid = QString::number(qApp->applicationPid());
    m_window = 0;
    m_directEvent = directEvent;
    m_clientType = PACPProtocol::APPLICATION;
    if(parent!=Q_NULLPTR)
    {
        QWidget *widget = qobject_cast<QWidget*>(parent);
        if(widget)
        {
            setWindow(widget,m_directEvent);
        }
    }
    setServerName("podonamu");
    connect(m_socket,SIGNAL(connected()),this, SLOT(connectedToServer()));
    connect(m_socket,SIGNAL(readyRead()), this, SLOT(readyReadFromServer()));
    connect(m_socket,SIGNAL(error(QLocalSocket::LocalSocketError)),this, SLOT(errorConnectServer(QLocalSocket::LocalSocketError)));
}

QWidget *PACPClient::window() const
{
    return m_window;
}

void PACPClient::setWindow(QWidget *window, bool directEvent)
{
    m_window = window;
    m_directEvent = directEvent;
}

bool PACPClient::directEvent() const
{
    return m_directEvent;
}

void PACPClient::setDirectEvent(bool directEvent)
{
    m_directEvent = directEvent;
}

QString PACPClient::name() const
{
    return m_name;
}

void PACPClient::setName(const QString &name)
{
    m_name = name;
}

QString PACPClient::serverName() const
{
    return m_serverName;
}

void PACPClient::setServerName(const QString &serverName)
{
    m_serverName = serverName;
    m_socket->setServerName(serverName);
}

bool PACPClient::connectToServer(const QString &serverName)
{
    if(!serverName.isEmpty())
        setServerName(serverName);
    m_socket->connectToServer();
    bool waitForConnected = m_socket->waitForConnected(2000);
    if(waitForConnected)
        qDebug() << "PACPClient::connectToServer Success connect to server ";
    else
        qDebug() << "PACPClient::connectToServer Failed connect to server ";
    return waitForConnected;
}

bool PACPClient::disconnectToServer()
{
    PACPCommand command;
    command.setSender(m_name);
    command.setPid(m_pid);
    command.setCommuniType(PACPCommand::DISCONNECT_APP);
    command.setActionData(PACPCommand::NONE);
    QByteArray jsonData = command.toJsonData();
    m_socket->write(jsonData);
    return true;

}

void PACPClient::requireRaise(const QString &receiver)
{
    PACPCommand command;
    command.setSender(m_name);
    command.setReceiver(receiver);
    command.setCommuniType(PACPCommand::APP_TO_APP);
    if(receiver.isEmpty())
        command.setCommuniType(PACPCommand::APP_TO_APPS);
    command.setActionData(PACPCommand::RAISE);
    QByteArray jsonData = command.toJsonData();
    m_socket->write(jsonData);
}

void PACPClient::requireRaiseToCompositor()
{
    PACPCommand command;
    command.setSender(m_name);
    command.setPid(m_pid);
    command.setCommuniType(PACPCommand::APP_TO_PDCOMPOSITOR);
    command.setActionData(PACPCommand::RAISE);
    QByteArray jsonData = command.toJsonData();
    m_socket->write(jsonData);
}

void PACPClient::requireLower(const QString &receiver)
{
    PACPCommand command;
    command.setSender(m_name);
    command.setReceiver(receiver);
    command.setCommuniType(PACPCommand::APP_TO_APP);
    if(receiver.isEmpty())
        command.setCommuniType(PACPCommand::APP_TO_APPS);
    command.setActionData(PACPCommand::LOWER);
    QByteArray jsonData = command.toJsonData();
    m_socket->write(jsonData);
}

void PACPClient::requireLowerToCompositor()
{
    PACPCommand command;
    command.setSender(m_name);
    command.setPid(m_pid);
    command.setCommuniType(PACPCommand::APP_TO_PDCOMPOSITOR);
    command.setActionData(PACPCommand::LOWER);
    QByteArray jsonData = command.toJsonData();
    m_socket->write(jsonData);
}

void PACPClient::requireShow(const QString &receiver)
{
    PACPCommand command;
    command.setSender(m_name);
    command.setReceiver(receiver);
    command.setCommuniType(PACPCommand::APP_TO_APP);
    if(receiver.isEmpty())
        command.setCommuniType(PACPCommand::APP_TO_APPS);
    command.setActionData(PACPCommand::SHOW);
    QByteArray jsonData = command.toJsonData();
    m_socket->write(jsonData);
}

void PACPClient::requireHide(const QString &receiver)
{
    PACPCommand command;
    command.setSender(m_name);
    command.setReceiver(receiver);
    command.setPid(m_pid);
    command.setCommuniType(PACPCommand::APP_TO_APP);
    if(receiver.isEmpty())
        command.setCommuniType(PACPCommand::APP_TO_APPS);
    command.setActionData(PACPCommand::HIDE);
    QByteArray jsonData = command.toJsonData();
    m_socket->write(jsonData);
}

void PACPClient::requireMove(const QString &receiver, const QPoint &point)
{
    PACPCommand command;
    command.setSender(m_name);
    command.setReceiver(receiver);
    command.setPid(m_pid);
    command.setCommuniType(PACPCommand::APP_TO_APP);
    if(receiver.isEmpty())
        command.setCommuniType(PACPCommand::APP_TO_APPS);
    command.setActionData(PACPCommand::MOVE);
    command.setMovePoint(point);
    QByteArray jsonData = command.toJsonData();
    m_socket->write(jsonData);
}

void PACPClient::requireMoveToCompositor(const QPoint &point)
{
    PACPCommand command;
    command.setSender(m_name);
    command.setPid(m_pid);
    command.setCommuniType(PACPCommand::APP_TO_PDCOMPOSITOR);
    command.setActionData(PACPCommand::MOVE);
    command.setMovePoint(point);
    QByteArray jsonData = command.toJsonData();
    m_socket->write(jsonData);
}

void PACPClient::requireClose(const QString &receiver)
{
    PACPCommand command;
    command.setSender(m_name);
    command.setReceiver(receiver);
    command.setPid(m_pid);
    command.setCommuniType(PACPCommand::APP_TO_APP);
    if(receiver.isEmpty())
        command.setCommuniType(PACPCommand::APP_TO_APPS);
    command.setActionData(PACPCommand::CLOSE);
    QByteArray jsonData = command.toJsonData();
    m_socket->write(jsonData);
}

void PACPClient::sendMessage(const QString &receiver, const QString &message)
{
    PACPCommand command;
    command.setSender(m_name);
    command.setReceiver(receiver);
    command.setPid(m_pid);
    command.setCommuniType(PACPCommand::APP_TO_APP);
    if(receiver.isEmpty())
        command.setCommuniType(PACPCommand::APP_TO_APPS);
    command.setActionData(PACPCommand::MESSAGE);
    command.setMessage(message);
    QByteArray jsonData = command.toJsonData();
    m_socket->write(jsonData);
}

void PACPClient::sendData(const QString &receiver, const QByteArray &data)
{
    PACPCommand command;
    command.setSender(m_name);
    command.setReceiver(receiver);
    command.setPid(m_pid);
    command.setCommuniType(PACPCommand::APP_TO_APP);
    if(receiver.isEmpty())
        command.setCommuniType(PACPCommand::APP_TO_APPS);
    command.setActionData(PACPCommand::DATA);
    command.setData(data);
    QByteArray jsonData = command.toJsonData();
    m_socket->write(jsonData);
}

void PACPClient::connectedToServer()
{
    PACPCommand command;
    command.setSender(m_name);
    command.setPid(m_pid);
    command.setCommuniType(PACPCommand::CONNECT_APP);
    if(m_clientType== PACPProtocol::COMPOSITOR)
    {
        command.setCommuniType(PACPCommand::CONNECT_PDCOMPOSITOR);
    }
    else if(m_clientType== PACPProtocol::LAUNCHER)
    {
        command.setCommuniType(PACPCommand::CONNECT_PDLAUNCHER);
    }
    command.setActionData(PACPCommand::NONE);
    QByteArray jsonData = command.toJsonData();
    qDebug() << "PACPClient::connectedToServer jsonData: " << jsonData;
    m_socket->write(jsonData);
}

void PACPClient::readyReadFromServer()
{
    QByteArray readAll = m_socket->readAll();

    while(!readAll.isEmpty())
    {
        int lastIndex = readAll.lastIndexOf("{");
        QByteArray readRight = readAll.right(readAll.length()-lastIndex);
        readAll = readAll.left(lastIndex);
        PACPCommand* pacpCommand = PACPCommand::parserJson(readRight);
        if(pacpCommand==NULL)
            return;
        switch(pacpCommand->getActionData())
        {
            case PACPCommand::NONE:
            {
                break;
            }
            case PACPCommand::RAISE:
            {
                if(m_clientType==PACPProtocol::APPLICATION)
                {
                    if(m_directEvent&& m_window!=0)
                    {
                        m_window->raise();

                        if(qApp->platformName().contains("wayland"))
                            requireRaiseToCompositor();
                    }
                    emit requestRaise(pacpCommand->getSender());
                }
                else if(m_clientType==PACPProtocol::COMPOSITOR)
                {
                    emit requestRaiseAtCompositor(pacpCommand->getPid());
                }
                break;

            }
            case PACPCommand::LOWER:
            {
                if(m_clientType==PACPProtocol::APPLICATION)
                {
                    if(m_directEvent&& m_window!=0)
                    {
                        m_window->lower();

                        if(qApp->platformName().contains("wayland"))
                            requireLowerToCompositor();
                    }
                    emit requestLower(pacpCommand->getSender());
                }
                else if(m_clientType==PACPProtocol::COMPOSITOR)
                {
                    emit requestLowerAtCompositor(pacpCommand->getPid());
                }
                break;
            }

            case PACPCommand::SHOW:
            {
                if(m_directEvent&& m_window!=0)
                    m_window->show();
                emit requestShow(pacpCommand->getSender());
                break;
            }
            case PACPCommand::HIDE:
            {
                if(m_directEvent&& m_window!=0)
                    m_window->hide();
                emit requestHide(pacpCommand->getSender());
                break;
            }
            case PACPCommand::MOVE:
            {
                if(m_clientType==PACPProtocol::APPLICATION)
                {
                    if(m_directEvent&& m_window!=0)
                    {
                        m_window->move(pacpCommand->getMovePoint());

                        if(qApp->platformName().contains("wayland"))
                            requireMoveToCompositor(pacpCommand->getMovePoint());
                    }
                    emit requestMove(pacpCommand->getSender(), pacpCommand->getMovePoint());
                }
                else if(m_clientType==PACPProtocol::COMPOSITOR)
                {
                    emit requestMoveAtCompositor(pacpCommand->getPid(), pacpCommand->getMovePoint());
                }
                break;
            }
            case PACPCommand::CLOSE:
            {
                if(m_directEvent && m_window!=0)
                    m_window->close();
                emit requestClose(pacpCommand->getSender());
            }
            case PACPCommand::DATA:
            {
                emit receiveData(pacpCommand->getSender(), pacpCommand->getData());
                break;
            }
            case PACPCommand::MESSAGE:
            {
                emit receiveMessage(pacpCommand->getSender(), pacpCommand->getMessage());
                break;
            }
            default:
                break;
        }
    }
}

void PACPClient::errorConnectServer(QLocalSocket::LocalSocketError)
{
    qDebug() << "PACPClient::errorConnectServer error :" << m_socket->errorString();
}
