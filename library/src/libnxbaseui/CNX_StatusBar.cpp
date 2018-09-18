#include "CNX_StatusBar.h"
#include "ui_CNX_StatusBar.h"

#define DAUDIO_STATUS_DATABASE_PATH "/home/root/daudio.status.db"

CNX_StatusBar::CNX_StatusBar(QWidget *parent) :
	QFrame(parent),	
	m_pCbHomeButtonClicked(NULL),
	m_pCbBackButtonClicked(NULL),
	m_pCbVolumeButtonClicked(NULL),
	ui(new Ui::CNX_StatusBar)
{
	ui->setupUi(this);

	m_pParent = parent;

	m_pDAudioStatus = new CNX_DAudioStatus(DAUDIO_STATUS_DATABASE_PATH);

	connect(&m_UpdateDateTimer, SIGNAL(timeout()), this, SLOT(slotUpdateDateTime()));
	m_UpdateDateTimer.start(1000);
	slotUpdateDateTime();

	connect(&m_UpdateDAudioStatus, SIGNAL(timeout()), this, SLOT(slotUpdateDAudioStatus()));
	m_UpdateDAudioStatus.start(1000);
	slotUpdateDAudioStatus();
}

CNX_StatusBar::~CNX_StatusBar()
{
	delete ui;
}

void CNX_StatusBar::resize(int w, int h)
{
	QFrame::resize(w, h);
}

void CNX_StatusBar::RegOnClickedHome(void (*cbFunc)(void *))
{
	m_pCbHomeButtonClicked = cbFunc;
}

void CNX_StatusBar::RegOnClickedBack(void (*cbFunc)(void *))
{
	m_pCbBackButtonClicked = cbFunc;
}

void CNX_StatusBar::RegOnClickedVolume(void (*cbFunc)(void *))
{
	m_pCbVolumeButtonClicked = cbFunc;
}

void CNX_StatusBar::SetButtonEnabled(ButtonType type, bool enabled)
{
	switch (type) {
	case ButtonType_Home:
		ui->BUTTON_HOME->setEnabled(enabled);
		break;

	case ButtonType_Back:
		ui->BUTTON_BACK->setEnabled(enabled);
		break;

	default: // ButtonType_All
		ui->BUTTON_HOME->setEnabled(enabled);
		ui->BUTTON_BACK->setEnabled(enabled);
		break;
	}
}

void CNX_StatusBar::SetTitleName(QString text)
{
	ui->LABEL_TITLE->setText(text);
}

void CNX_StatusBar::SetVolume(int value)
{
	if (m_pDAudioStatus->SetVolume(value))
	{
		ui->LABEL_VOLUME_VALUE->setText(QString("%1").arg(value));
	}
}

int CNX_StatusBar::GetVolume()
{
	return ui->LABEL_VOLUME_VALUE->text().toInt();
}

QString CNX_StatusBar::GetTitleName()
{
	return ui->LABEL_TITLE->text();
}

void CNX_StatusBar::SetBTConnection(int value)
{
	if (m_pDAudioStatus->SetBTConnection(value))
	{
		ui->BUTTON_BT_CONNECTION_ICON->setChecked(value);
	}
}

int CNX_StatusBar::GetBTConnection()
{
	return ui->BUTTON_BT_CONNECTION_ICON->isChecked() ? 1 : 0;
}

void CNX_StatusBar::on_BUTTON_HOME_clicked()
{
	if (m_pCbHomeButtonClicked)
		m_pCbHomeButtonClicked(m_pParent);
}

void CNX_StatusBar::on_BUTTON_BACK_clicked()
{
	if (m_pCbBackButtonClicked)
		m_pCbBackButtonClicked(m_pParent);
}

void CNX_StatusBar::on_BUTTON_BT_CONNECTION_ICON_toggled(bool checked)
{
	ui->LABEL_BT_CONNECTION_TEXT->setText(checked ? "Connected" : "Disconnected");
}

void CNX_StatusBar::slotUpdateDateTime()
{
	ui->LABEL_DATETIME->setText(QDateTime::currentDateTime().toString("hh:mm:ss AP"));
}

void CNX_StatusBar::slotUpdateDAudioStatus()
{
	int volume = m_pDAudioStatus->GetVolume();
	int connection = m_pDAudioStatus->GetBTConnection();

	if (volume >= 0)
	{
		ui->LABEL_VOLUME_VALUE->setText(QString("%1").arg(volume));
	}

	if (connection >= 0)
	{
		ui->BUTTON_BT_CONNECTION_ICON->setChecked(connection);
	}
}

void CNX_StatusBar::on_BUTTON_VOLUME_clicked()
{
	if (m_pCbVolumeButtonClicked)
		m_pCbVolumeButtonClicked(m_pParent);
}
