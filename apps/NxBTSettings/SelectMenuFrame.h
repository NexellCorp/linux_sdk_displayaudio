#ifndef SELECTMENUFRAME_H
#define SELECTMENUFRAME_H

#include <QFrame>
#include "Types.h"

namespace Ui {
class SelectMenuFrame;
}

class SelectMenuFrame : public QFrame
{
	Q_OBJECT

signals:
	void signalCurrentMenuChanged(Menu menu);

public:
	explicit SelectMenuFrame(QWidget *parent = 0);
	~SelectMenuFrame();

private slots:
	void on_BUTTON_ENTER_CONNECTION_MENU_clicked();

	void on_BUTTON_ENTER_ADVANCED_MENU_clicked();

private:
	Ui::SelectMenuFrame *ui;
};

#endif // SELECTMENUFRAME_H
