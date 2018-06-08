#ifndef SELECTMENUWIDGET_H
#define SELECTMENUWIDGET_H

#include <QDialog>
#include "defines.h"

namespace Ui {
class SelectMenuWidget;
}

class SelectMenuWidget : public QWidget
{
    Q_OBJECT

signals:
    void signalCurrentMenuChanged(Menu menu);

public:
    explicit SelectMenuWidget(QWidget *parent = 0);
    ~SelectMenuWidget();

private slots:
    void on_BUTTON_ENTER_CONNECTION_MENU_clicked();

    void on_BUTTON_ENTER_ADVANCED_MENU_clicked();

private:
    Ui::SelectMenuWidget *ui;
};

#endif // SELECTMENUWIDGET_H
