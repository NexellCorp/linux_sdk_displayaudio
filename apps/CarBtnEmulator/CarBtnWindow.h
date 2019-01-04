#ifndef CARBTNWINDOW_H
#define CARBTNWINDOW_H

#include <QMainWindow>
#include <QTableWidget>

#include <TabSetting.h>
#include <TabButtons.h>
#include <TabTest.h>

namespace Ui {
class CarBtnWindow;
}

class CarBtnWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit CarBtnWindow(QWidget *parent = 0);
	~CarBtnWindow();

private:
	Ui::CarBtnWindow *ui;

	//	User Space
	QTabWidget *m_TabWidget;
};

#endif // CARBTNWINDOW_H
