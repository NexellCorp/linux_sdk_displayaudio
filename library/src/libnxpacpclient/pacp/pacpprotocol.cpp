#include "pacpprotocol.h"

#include <QJsonObject>
#include <QPoint>


QString PACPCommand::getMessage() const
{
    return m_message;
}

void PACPCommand::setMessage(const QString &message)
{
    m_message = message;
}

QPoint PACPCommand::getMovePoint() const
{
    return m_movePoint;
}

void PACPCommand::setMovePoint(const QPoint &movePoint)
{
    m_movePoint = movePoint;
}

PACPCommand::CommunicationType PACPCommand::getCommuniType() const
{
    return m_communiType;
}

void PACPCommand::setCommuniType(const CommunicationType &communiType)
{
    m_communiType = communiType;
}

QByteArray PACPCommand::getData() const
{
    return m_data;
}

void PACPCommand::setData(const QByteArray &data)
{
    m_data = data;
}

QString PACPCommand::getPid() const
{
    return m_pid;
}

void PACPCommand::setPid(const QString &value)
{
    m_pid = value;
}

QString PACPCommand::getSender() const
{
    return m_sender;
}

void PACPCommand::setSender(const QString &value)
{
    m_sender = value;
}

QString PACPCommand::getReceiver() const
{
    return m_receiver;
}

void PACPCommand::setReceiver(const QString &value)
{
    m_receiver = value;
}

PACPCommand::ActionData PACPCommand::getActionData() const
{
    return m_actionData;
}

void PACPCommand::setActionData(const ActionData &actionData)
{
    m_actionData = actionData;
}

PACPCommand* PACPCommand::parserJson(const QJsonDocument &jsonDoc)
{
    if(jsonDoc.isEmpty())
        return NULL;
    PACPCommand* pacpCommand = new PACPCommand;
    QJsonObject jsonObject = jsonDoc.object();
    qDebug() << "PACPCommand::parserJson jsonObject : " << jsonObject;
    QString pid = jsonObject.value("pid").toString();
    QString sender = jsonObject.value("sender").toString();
    QString receiver = jsonObject.value("receiver").toString();
    CommunicationType communicationType = static_cast<CommunicationType>(jsonObject.value("communicationType").toInt());
    ActionData actionData = static_cast<ActionData>(jsonObject.value("actionData").toInt());
    QByteArray data = QByteArray::fromHex(jsonObject.value("data").toString().toUtf8());
    int xPos = jsonObject.value("xPos").toInt();
    int yPos = jsonObject.value("yPos").toInt();
    QPoint pos = QPoint(xPos,yPos);
    QString message = jsonObject.value("message").toString();
    pacpCommand->setPid(pid);
    pacpCommand->setSender(sender);
    pacpCommand->setReceiver(receiver);
    pacpCommand->setCommuniType(communicationType);
    pacpCommand->setActionData(actionData);
    pacpCommand->setData(data);
    pacpCommand->setMovePoint(pos);
    pacpCommand->setMessage(message);
    return pacpCommand;
}

PACPCommand* PACPCommand::parserJson(const QByteArray &jsonDoc)
{
    return PACPCommand::parserJson(QJsonDocument::fromJson(jsonDoc));
}

QJsonDocument PACPCommand::toJsonDocument() const
{
    QJsonObject jsonObject;
    jsonObject.insert("pid",m_pid);
    jsonObject.insert("sender",m_sender);
    jsonObject.insert("receiver", m_receiver);
    jsonObject.insert("communicationType", m_communiType);
    jsonObject.insert("actionData",m_actionData);
    jsonObject.insert("data", QString(m_data.toHex()));
    jsonObject.insert("message",m_message);
    jsonObject.insert("xPos",m_movePoint.x());
    jsonObject.insert("yPos",m_movePoint.y());
    qDebug() << "PACPCommand::toJsonDocument jsonObject : " << jsonObject;
    return QJsonDocument(jsonObject);
}

QByteArray PACPCommand::toJsonData() const
{
    return toJsonDocument().toJson();
}

