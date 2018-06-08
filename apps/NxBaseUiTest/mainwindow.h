#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>

#include <CNX_StatusBar.h>
#include <CNX_MediaController.h>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    void resizeEvent(QResizeEvent* event);
	bool eventFilter(QObject *watched, QEvent *event);

private:
    Ui::MainWindow *ui;

	CNX_StatusBar* m_pStatusBar;
	CNX_MediaController* m_pMediaController;

    QTimer m_Timer;

private slots:
	void CheckStatus();
	void on_BUTTON_STATUSBAR_ENABLED_toggled(bool checked);
	void on_BUTTON_SET_BUTTON_ALL_ENABLED_toggled(bool checked);
	void on_BUTTON_SET_BUTTON_HOME_ENABLED_toggled(bool checked);
	void on_BUTTON_SET_BUTTON_BACK_ENABLED_toggled(bool checked);
};

#endif // MAINWINDOW_H
