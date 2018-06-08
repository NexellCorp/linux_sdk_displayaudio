#include "core.h"
#include <QCoreApplication>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    Core core;
    core.execute();

    return a.exec();
}
