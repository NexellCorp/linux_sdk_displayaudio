#ifndef PACPCLIENT_H
#define PACPCLIENT_H

#include <QLocalSocket>
#include <QObject>
#include <QWidget>
#include "pacpprotocol.h"


class PACPClient : public QObject
{
    Q_OBJECT
private:
    QString m_pid;
    QString m_name;
    QString m_serverName;
    QLocalSocket* m_socket;
    QWidget* m_window;
    bool m_directEvent;
    PACPProtocol::ClientType m_clientType;

public:
    explicit PACPClient(QObject *parent = Q_NULLPTR, bool directEvent= true);

    QWidget *window() const;
    void setWindow(QWidget *window, bool directEvent=true);

    bool directEvent() const;
    void setDirectEvent(bool directEvent);

    QString name() const;
    void setName(const QString &name);

    QString serverName() const;
    void setServerName(const QString &serverName);

    bool connectToServer(const QString &serverName= QString());
    bool disconnectToServer();

    void requireRaise(const QString& receiver);
    void requireRaiseToCompositor();
    void requireLower(const QString& receiver);
    void requireLowerToCompositor();
    void requireShow(const QString& receiver);
    void requireHide(const QString& receiver);
    void requireMove(const QString& receiver, const QPoint& point);
    void requireMoveToCompositor(const QPoint& point);
    void requireClose(const QString& receiver);
    void sendMessage(const QString& receiver, const QString& message);
    void sendData(const QString& receiver, const QByteArray& data);
    PACPProtocol::ClientType clientType() const;
    void setClientType(const PACPProtocol::ClientType &clientType);

signals:
    void requestRaise(const QString& sender);
    void requestRaiseAtCompositor(const QString& pid);
    void requestLower(const QString& sender);
    void requestLowerAtCompositor(const QString& pid);
    void requestShow(const QString& sender);
    void requestHide(const QString& sender);
    void requestClose(const QString& sender);
    void requestMove(const QString& sender, const QPoint& pos);
    void receiveData(const QString& sender, const QByteArray& data);
    void receiveMessage(const QString& sender, const QString& message);
    void requestMoveAtCompositor(const QString& pid, const QPoint& pos);

public slots:
    void connectedToServer();
    void readyReadFromServer();
    void errorConnectServer(QLocalSocket::LocalSocketError error);
};

#endif // PACPCLIENT_H
