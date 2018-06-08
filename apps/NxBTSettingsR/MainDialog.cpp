#include "MainDialog.h"
#include "ui_MainDialog.h"
#include <QDebug>

void MainDialog::callbackStatusHomeButtonClicked(void* obj)
{
	// to avoid compiler warning message.
	(void)obj;
#ifdef CONFIG_NX_DAUDIO_MANAGER
	if (NX_REPLY_FAIL == NX_RequestCommand(NX_REQUEST_LAUNCHER_SHOW))
	{
		printf("@fail, NX_REQUEST_LAUNCHER_SHOW\n");
	}
#else
	exit(0);
#endif
}

void MainDialog::callbackStatusBackButtonClicked(void* obj)
{
	MainDialog* self = (MainDialog*)obj;

	switch (g_current_menu) {
	case Menu_Init:
	case Menu_Select:
		// NX_REQUEST_PROCESS_REMOVE
#ifdef CONFIG_NX_DAUDIO_MANAGER		
		self->close();
#else
		exit(0);
#endif
		break;

	case Menu_Connection:
		self->setCurrentMenu(Menu_Select);
		break;

	case Menu_Advanced:
		self->setCurrentMenu(Menu_Select);
		break;
	}
}

MainDialog::MainDialog(QWidget *parent) :
	QDialog(parent, Qt::FramelessWindowHint),
	ui(new Ui::MainDialog)
{
	ui->setupUi(this);

	m_pCommandProcessor = BTCommandProcessor::instance();
	if (m_pCommandProcessor) {
		connect(m_pCommandProcessor, SIGNAL(signalCommandFromServer(QString)), this, SLOT(slotCommandFromServer(QString)));
		m_pCommandProcessor->start();
	}

	// settings for NxStatusBar
	m_pNxStatusBar = new CNX_StatusBar(this);
#if 1
	m_pNxStatusBar->move(0, 0);
	m_pNxStatusBar->resize(width(), height() / 10);
#else
	m_pNxStatusBar->setGeometry(0, 0, width(), height() / 10);
#endif
	m_pNxStatusBar->RegOnClickedHome(callbackStatusHomeButtonClicked);
	m_pNxStatusBar->RegOnClickedBack(callbackStatusBackButtonClicked);
	m_pNxStatusBar->SetTitleName("Nexell BT Settings");
	m_pNxStatusBar->show();

	m_pSelectMenuWidget = new SelectMenuWidget(this);
	m_pSelectMenuWidget->setGeometry(0, 60, 1024, 540);
	connect(m_pSelectMenuWidget, SIGNAL(signalCurrentMenuChanged(Menu)), this, SLOT(slotCurrentMenuChanged(Menu)));

	m_pConnectionMenuWidget = new ConnectionMenuWidget(this);
	m_pConnectionMenuWidget->setGeometry(0, 60, 1024, 540);

	m_pAdvancedMenuWidget = new AdvancedMenuWidget(this);
	m_pAdvancedMenuWidget->setGeometry(0, 60, 1024, 540);

	setCurrentMenu(Menu_Init);

	NX_PacpClientStart(this);

	installEventFilter(this);

	// check for initialized state of service.
	m_pCommandProcessor->commandToServer("$MGT#PING\n");

	m_pLoadingImage = new QMovie("://UI/loading3_100x100.gif");
	ui->LABEL_LOADING_IMAGE->setMovie(m_pLoadingImage);
	m_pLoadingImage->start();
}

MainDialog::~MainDialog()
{
	NX_PacpClientStop();
	delete ui;
}

void MainDialog::setCurrentMenu(Menu menu)
{
	g_current_menu = menu;

	switch (g_current_menu) {
	case Menu_Init:
		m_pSelectMenuWidget->hide();
		m_pConnectionMenuWidget->hide();
		m_pAdvancedMenuWidget->hide();
		break;

	case Menu_Select:
		m_pSelectMenuWidget->show();
		m_pConnectionMenuWidget->hide();
		m_pAdvancedMenuWidget->hide();
		break;

	case Menu_Connection:
		m_pConnectionMenuWidget->show();
		m_pSelectMenuWidget->hide();
		m_pAdvancedMenuWidget->hide();
		break;

	case Menu_Advanced:
		m_pAdvancedMenuWidget->show();
		m_pSelectMenuWidget->hide();
		m_pConnectionMenuWidget->hide();
		break;
	}
}

void MainDialog::slotCurrentMenuChanged(Menu menu)
{
	setCurrentMenu(menu);
}

bool MainDialog::eventFilter(QObject *watched, QEvent *event)
{
	if (watched == this && event->type() == NX_QT_CUSTOM_EVENT_TYPE) {
		NxEvent *pEvent = reinterpret_cast<NxEvent*>(event);

		switch (pEvent->m_iEventType) {
		case NX_REQUEST_PROCESS_SHOW:
			NX_PacpClientRequestRaise();
			break;

		case NX_REQUEST_PROCESS_HIDE:
			break;

		case NX_REQUEST_PROCESS_REMOVE:
			close();
			break;

		default:
			break;
		}
	}

#ifdef CONFIG_TEST_FLAG
	if (watched == this && event->type() == QEvent::WindowActivate)
	{
		if (g_first_shown)
		{
			NX_ReplyDone();
		}
		else
		{
			g_first_shown = true;
		}
	}
#endif

	return QDialog::eventFilter(watched, event);
}

//void MainDialog::showEvent(QShowEvent *)
//{
//	qDebug() << Q_FUNC_INFO;
//	if (isVisible())
//		return;

//	if (NX_REPLY_FAIL == NX_RequestCommand(NX_REQUEST_PROCESS_SHOW))
//	{
//		printf("Fail, NX_RequestCommand(NX_REQUEST_PROCESS_SHOW)\n");
//	}

//	setCurrentMenu(g_current_menu);
//}

//void MainDialog::hideEvent(QHideEvent *)
//{
//	NX_PacpClientRequestLower();
//}

void MainDialog::closeEvent(QCloseEvent *)
{
	m_pSelectMenuWidget->close();
	m_pSelectMenuWidget->deleteLater();

	m_pConnectionMenuWidget->close();
	m_pConnectionMenuWidget->deleteLater();

	m_pAdvancedMenuWidget->close();
	m_pAdvancedMenuWidget->deleteLater();

	QDialog::close();

	if (NX_REPLY_FAIL == NX_RequestCommand(NX_REQUEST_PROCESS_REMOVE)) {
		printf("@Fail, NX_RequestCommand(). command = NX_REQUEST_PROCESS_REMOVE\n");
	}
}

void MainDialog::slotCommandFromServer(QString command)
{
	qDebug() << Q_FUNC_INFO << command;
	// $OK#MGT#PING
	int stx = command.indexOf("$");
	int etx = command.indexOf("\n");

	// assume unknown command format.
	if (stx < 0 || etx < 0) {
		return;
	}

	// body = remove STX and ETX from command
	stx++;
	QString body = command.mid(stx, etx-stx);
	QStringList tokens = body.split("#");

	if (tokens.size() == 3) {
		if (tokens[0] == "OK" && tokens[1] == "MGT" && tokens[2] == "PING") {
			disconnect(m_pCommandProcessor, SIGNAL(signalCommandFromServer(QString)), this, SLOT(slotCommandFromServer(QString)));
			setCurrentMenu(Menu_Select);
			m_pLoadingImage->stop();
			ui->LABEL_LOADING_IMAGE->setMovie(NULL);
			ui->LABEL_LOADING_IMAGE->clear();
			delete m_pLoadingImage;
			m_pLoadingImage = NULL;
		}
	}

}
