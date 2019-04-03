#include "PageItemFrame.h"
#include "ui_PageItemFrame.h"

PageItemFrame::PageItemFrame(QWidget *parent) :
	QFrame(parent),
	ui(new Ui::PageItemFrame)
{
	ui->setupUi(this);
}

PageItemFrame::~PageItemFrame()
{
	delete ui;
}

void PageItemFrame::setButtonUISize(int w, int h)
{
	ui->BUTTON_IMAGE->setFixedSize(w, h);
}

void PageItemFrame::setButtonEnabled(bool enabled)
{
	if (enabled)
	{
		ui->BUTTON_IMAGE->show();
		ui->LABEL_TEXT->setStyleSheet("color: rgba(1,1,1,100%);");
	}
	else
	{
		ui->BUTTON_IMAGE->hide();
		ui->LABEL_TEXT->setStyleSheet("color: rgba(1,1,1,30%);");
	}

	ui->BUTTON_IMAGE->setEnabled(enabled);
}

void PageItemFrame::setTextUISize(int w, int h)
{
	ui->LABEL_TEXT->setFixedSize(w, h);
}

void PageItemFrame::setText(QString text)
{
	ui->LABEL_TEXT->setText(text);
}

QString PageItemFrame::text()
{
	return ui->LABEL_TEXT->text();
}

void PageItemFrame::setIcon(QString normal, QString pressed, QString disabled)
{
	QString styleSheetData;
	if (!normal.isEmpty()) {
		styleSheetData += "QPushButton {\n";
		styleSheetData += "  border-image: url(" + normal + ");\n";
		styleSheetData += "  border: none;\n";
		styleSheetData += "}";
	}

	if (pressed.isEmpty()) {
		styleSheetData += "QPushButton:pressed {\n";
		styleSheetData += "  background: rgba(30, 30, 30, 30%);\n";
		styleSheetData += "  border: none;\n";
		styleSheetData += "}";
	} else {
		styleSheetData += "QPushButton:pressed {\n";
		styleSheetData += "  border-image: url(" + pressed + ");\n";
		styleSheetData += "  border: none;\n";
		styleSheetData += "}";
	}

	if (!disabled.isEmpty()) {
		styleSheetData += "QPushButton:disabled {\n";
		styleSheetData += "  border-image: url(" + disabled + ");\n";
		styleSheetData += "  border: none;\n";
		styleSheetData += "}";
	}

	ui->BUTTON_IMAGE->setStyleSheet(styleSheetData);
}

void PageItemFrame::setAppInfo(NxPluginInfo* pInfo)
{
	m_pAppInfo = pInfo;
}

NxPluginInfo* PageItemFrame::getAppInfo()
{
	return m_pAppInfo;
}

void PageItemFrame::on_BUTTON_IMAGE_clicked()
{
	emit onButtonClicked(m_pAppInfo);
}
