#include "PageStackFrame.h"
#include "ui_PageStackFrame.h"
#include <QDebug>
#include <NX_Log.h>

PageStackFrame::PageStackFrame(QWidget *parent) :
	QFrame(parent),
	ui(new Ui::PageStackFrame)
{
	ui->setupUi(this);

	m_Spacing = 0;
	m_CurrentPage = -1;
}

PageStackFrame::~PageStackFrame()
{
	delete ui;
}

void PageStackFrame::pushItem(QString text, QString icon)
{
	PageItemFrame *item = NULL;

	item = new PageItemFrame(this);
	item->setFixedSize(m_Cell.width(), m_Cell.height());
#if 1
	item->setButtonUISize(m_Cell.width()*0.9, m_Cell.height()*0.8);
#else
	item->setButtonUISize(m_Cell.width(), m_Cell.height()*0.9);
#endif
	item->setTextUISize(m_Cell.width(), m_Cell.height()*0.15);
	item->setIcon(icon, QString(), QString());
	item->setText(text);
	item->show();

	m_PageItemList.push_back(item);
}

void PageStackFrame::pushItem(NxPluginInfo* pInfo)
{
	PageItemFrame *item = NULL;

	item = new PageItemFrame(this);
	connect(item, SIGNAL(onButtonClicked(NxPluginInfo*)), this, SLOT(onButtonClickedFromItem(NxPluginInfo*)));
	item->setFixedSize(m_Cell.width(), m_Cell.height());
	item->setButtonUISize(m_Cell.width()*0.9, m_Cell.height()*0.8);
	item->setButtonEnabled(pInfo->getEnabled());
	item->setTextUISize(m_Cell.width(), m_Cell.height()*0.15);
	item->setIcon(pInfo->getPath() + "/" + pInfo->getIcon(), QString(), QString());
	item->setText(pInfo->getExec());
	item->setAppInfo(pInfo);
	item->show();

	m_PageItemList.push_back(item);
}

void PageStackFrame::resizeItem()
{
	int page = m_CurrentPage < 0 ? 0 : m_CurrentPage;
	calculateMatrix();

	for (size_t i = 0; i < m_PageList.size(); ++i) {
		m_PageList[i]->setFixedSize(width(), height());
	}

	for (size_t i = 0; i < m_PageItemList.size(); ++i) {
		PageItemFrame* item = m_PageItemList[i];
		int page = i / (m_Col * m_Row);
		int x = i % m_Col;
		int y = i / m_Col - (m_Row*page);

		if (page >= (int)m_PageList.size()) {
			int diff = page - (m_PageList.size()-1);
			for (int i = 0; i < diff; ++i) {
				PageFrame *frame = new PageFrame(this);
				frame->setFrameShadow(QFrame::Plain);
				frame->setFrameShape(QFrame::NoFrame);
				frame->setFixedSize(width(), height());
				m_PageList.push_back(frame);
			}
		}

		item->setParent(m_PageList[page]);
		item->move(x * m_Cell.width() + x * m_Spacing, y * m_Cell.height() + y * m_Spacing);
		item->show();
	}

	for (int i = m_PageList.size()-1; i > -1; --i) {
		if (m_PageList[i]->children().size() == 0) {
			delete m_PageList[i];
			m_PageList.erase(m_PageList.begin()+i);
			if (page == i) {
				if (--page < 0)
					page = 0;
			}
		}
	}

	ui->PAGE_INDICATOR_FRAME->setPageCount(m_PageList.size());

	if (m_CurrentPage != page)
		setPage(page);
}

void PageStackFrame::setCellSize(QSize cell)
{
	m_Cell = cell;
	setMinimumSize(m_Cell);
}

void PageStackFrame::calculateMatrix()
{
#if 0
	m_Row = height() / m_Cell.height();
	m_Col = width() / m_Cell.width();
#else
	int h = height() - ui->PAGE_INDICATOR_FRAME->height();
	m_Row = (h - (m_Spacing * (h / m_Cell.height() - 1))) / m_Cell.height();
	m_Col = (width() - (m_Spacing * (width() / m_Cell.width() - 1))) / m_Cell.width();
#endif
}

void PageStackFrame::resizeEvent(QResizeEvent *)
{
	resizeItem();
}

void PageStackFrame::setPage(int page)
{
	if (page >= 0 && page < (int)m_PageList.size()) {
		if (m_CurrentPage != page) {
			m_CurrentPage = page;

			for (int i = 0; i < (int)m_PageList.size(); ++i) {
				if (page == i)
					m_PageList[i]->show();
				else
					m_PageList[i]->hide();
			}

			ui->PAGE_INDICATOR_FRAME->setPage(page);
		}
	}
}

int PageStackFrame::currentPage()
{
	return m_CurrentPage;
}

void PageStackFrame::setSpacing(int spacing)
{
	m_Spacing = spacing;
}

void PageStackFrame::onButtonClickedFromItem(NxPluginInfo *pInfo)
{
	emit onButtonClicked(pInfo);
}

void PageStackFrame::onButtonStateChnaged(NxPluginInfo* pInfo)
{
	for (size_t i = 0; i < m_PageItemList.size(); ++i) {
		if (m_PageItemList[i]->getAppInfo() == pInfo) {
			m_PageItemList[i]->setButtonEnabled(pInfo->getEnabled());
		}
	}
}
