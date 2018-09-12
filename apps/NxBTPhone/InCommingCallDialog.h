#ifndef INCOMMINGCALLDIALOG_H
#define INCOMMINGCALLDIALOG_H

#include <QDialog>
#include "defines.h"

namespace Ui {
class InCommingCallDialog;
}

class InCommingCallDialog : public QDialog
{
    Q_OBJECT

signals:
    void signalCommandToServer(QString command);

private slots:
    void slotCommandFromServer(QString command);

public:


public:
    explicit InCommingCallDialog(QWidget *parent = 0);
    ~InCommingCallDialog();

private:
    Ui::InCommingCallDialog *ui;
};

#endif // INCOMMINGCALLDIALOG_H
