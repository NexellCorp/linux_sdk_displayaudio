#ifndef PAGEFRAME_H
#define PAGEFRAME_H

#include <QFrame>

namespace Ui {
class PageFrame;
}

class PageFrame : public QFrame
{
	Q_OBJECT

public:
	explicit PageFrame(QWidget *parent = 0);

	~PageFrame();

private:
	Ui::PageFrame *ui;
};

#endif // PAGEFRAME_H
