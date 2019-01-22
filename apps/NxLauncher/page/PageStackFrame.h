#ifndef PAGESTACKFRAME_H
#define PAGESTACKFRAME_H

#include <QFrame>
#include <QResizeEvent>
#include "PageItemFrame.h"
#include "PageFrame.h"
#include "../nxappinfo.h"

#include <vector>
using namespace std;

namespace Ui {
class PageStackFrame;
}

class PageStackFrame : public QFrame
{
	Q_OBJECT

signals:
	void onButtonClicked(NxPluginInfo* pInfo);

private slots:
	void onButtonClickedFromItem(NxPluginInfo *pInfo);

	void onButtonStateChnaged(NxPluginInfo* pInfo);

public:
	explicit PageStackFrame(QWidget *parent = 0);
	~PageStackFrame();

	void pushItem(QString text, QString icon);

	void pushItem(NxPluginInfo* pInfo);

	void resizeItem();

	void setCellSize(QSize cell);

	void setSpacing(int spacing);

	void resizeEvent(QResizeEvent *event);

	void setPage(int page);

	int currentPage();

	int GetPageCount();

private:
	void calculateMatrix();

private:
	vector<PageItemFrame*> m_PageItemList;

	vector<PageFrame*> m_PageList;

	int m_Row;

	int m_Col;

	QSize m_Cell;

	int m_Spacing;

	int m_CurrentPage;

private:
	Ui::PageStackFrame *ui;
};

#endif // PAGESTACKFRAME_H
