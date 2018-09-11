#ifndef NXLAUNCHER_H
#define NXLAUNCHER_H

#include <QMainWindow>
#include <QProcess>
#include <QEvent>


namespace Ui {
class NxLauncher;
}

class NxLauncher : public QMainWindow
{
    Q_OBJECT



public:
    explicit NxLauncher(QWidget *parent = 0);
    ~NxLauncher();

protected:
	bool event(QEvent *event);



private:
    Ui::NxLauncher *ui;
};

#endif // NXLAUNCHER_H
