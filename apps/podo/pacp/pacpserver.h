#ifndef PACPSERVER_H
#define PACPSERVER_H

#include <QObject>
#include <QLocalServer>
#include <QLocalSocket>
#include <QMutexLocker>

typedef QMultiMap<QString,QLocalSocket*> LocalSocketMultiMap;
class PACPServer : public QObject
{
    Q_OBJECT
private:
    QLocalServer* m_server;
    LocalSocketMultiMap m_localSocketMultiMap;
    QString m_pdlauncherName;
    QString m_pdcompositorName;

public:
    explicit PACPServer(QObject *parent = 0);
    ~PACPServer();
    Q_INVOKABLE bool setServerName(const QString& name);

signals:
    void requestMoveWindow(const QString& pId, const QPoint& position);
    void requestShowWindow(const QString& pId);
    void requestHideWindow(const QString& pId);
    void requestRaiseWindow(const QString& pId);
    void requestLowerWindow(const QString& pId);

public slots:

private slots:
    Q_INVOKABLE void newConnection();
    Q_INVOKABLE void readyReadFromClient();
    Q_INVOKABLE void disconnectedFromClient();
};

#endif // PACPSERVER_H
