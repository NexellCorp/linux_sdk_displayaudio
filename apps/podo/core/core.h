#ifndef CORE_H
#define CORE_H

#include <QObject>
#include "pacpserver.h"

class Core : public QObject
{
    Q_OBJECT
private:
    PACPServer* m_pacpServer;
public:
    explicit Core(QObject* parent=0);
    ~Core();

    void execute();
    void initPACPServer();
};

#endif // CORE_H
