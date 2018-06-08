#ifndef PACPPROTOCOL_H
#define PACPPROTOCOL_H

#include <QDebug>
#include <QJsonDocument>
#include <QPoint>
#include <QString>

namespace PACPProtocol {
    enum ClientType {
        APPLICATION,
        COMPOSITOR,
        LAUNCHER
    };
}


class PACPCommand : public QObject {
    Q_OBJECT

public:
    enum ActionData {
        NONE =0,
        SET_GEOMETRY,
        RAISE,
        LOWER,
        MOVE,
        SHOW,
        HIDE,
        CLOSE,
        MESSAGE,
        DATA
    };
    enum CommunicationType {
        APP_TO_APP,
        APP_TO_APPS,
        APP_TO_PDCOMPOSITOR,
        APP_TO_LAUNCHER,
        LAUNCHER_TO_APP,
        LAUNCHER_TO_APPS,
        CONNECT_APP,
        DISCONNECT_APP,
        CONNECT_PDLAUNCHER,
        DISCONNECT_PDLAUNCHER,
        CONNECT_PDCOMPOSITOR,
        DICONNECT_PDCOMPOSITOR,
    };
    Q_ENUM(ActionData)
    Q_ENUM(CommunicationType)
private:
    QString m_pid;
    QString m_sender;
    QString m_receiver;
    ActionData m_actionData;
    CommunicationType m_communiType;
    QPoint m_movePoint;
    QString m_message;
    QByteArray m_data;
public:
    static PACPCommand* parserJson(const QByteArray& jsonDoc);
    static PACPCommand* parserJson(const QJsonDocument& jsonDoc);



    QJsonDocument toJsonDocument() const;
    QByteArray toJsonData() const;

    QString getPid() const;
    void setPid(const QString &value);

    QString getSender() const;
    void setSender(const QString &value);

    QString getReceiver() const;
    void setReceiver(const QString &value);

    QString getMessage() const;
    void setMessage(const QString &message);

    QPoint getMovePoint() const;
    void setMovePoint(const QPoint &movePoint);

    CommunicationType getCommuniType() const;
    void setCommuniType(const CommunicationType &communiType);

    QByteArray getData() const;
    void setData(const QByteArray &data);

    ActionData getActionData() const;
    void setActionData(const ActionData &actionData);
};


#endif // PACPPROTOCOL_H












