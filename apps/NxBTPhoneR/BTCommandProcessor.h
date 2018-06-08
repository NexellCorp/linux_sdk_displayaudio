#ifndef BTCOMMANDPROCESSOR_H
#define BTCOMMANDPROCESSOR_H

#include <QThread>
#include "UDS_Client.h"

class BTCommandProcessor : public QThread
{
    Q_OBJECT

signals:
    void signalCommandProcessorEnabled();

    void signalCommandFromServer(QString command);

private slots:
    void slotCommandToServer(QString command);

public:
    ~BTCommandProcessor();

    static BTCommandProcessor* instance(); // for single ton pattern

    void commandToServer(QString command);

    void commandListToServer(QStringList commands);

protected:
    virtual void run();

private:
    BTCommandProcessor();

private:
    static BTCommandProcessor* m_pInstance;

private:
    UDS_Client m_IPCClient;
};

#endif // BTCOMMANDPROCESSOR_H
