#ifndef PAGEITEMFRAME_H
#define PAGEITEMFRAME_H

#include <QFrame>
#include "nxappinfo.h"

namespace Ui {
class PageItemFrame;
}

class PageItemFrame : public QFrame
{
	Q_OBJECT

signals:
	void onButtonClicked(NxAppInfo* pInfo);

public:
	explicit PageItemFrame(QWidget *parent = 0);
	~PageItemFrame();

	void setButtonUISize(int w, int h);

	void setButtonEnabled(bool enabled);

	void setTextUISize(int w, int h);

	void setText(QString text);

	QString text();

	void setIcon(QString normal, QString pressed, QString disabled);

	void setAppInfo(NxAppInfo* pInfo);

	NxAppInfo* getAppInfo();

private slots:
	void on_BUTTON_IMAGE_clicked();

private:
	NxAppInfo* m_pAppInfo;

private:
	Ui::PageItemFrame *ui;
};

#endif // PAGEITEMFRAME_H
