#include "MainDialog.h"
#include "ui_MainDialog.h"

#define MAX_RETRY_COMMAND_COUNT 3

//pthread_cond_t g_pthread_condition;
//pthread_mutex_t g_pthread_mutex;

///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief NxStatusBar - [Home] button clicked
/// \param obj
///
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
	case Menu_Call:
		self->setCurrentMenu(Menu_Select);
		break;

	case Menu_Message:
		self->setCurrentMenu(Menu_Select);
		break;

	case Menu_Select:
		self->close();
		break;

	default:
		break;
	}
}

MainDialog::MainDialog(QWidget *parent) :
	QDialog(parent, Qt::FramelessWindowHint),
	ui(new Ui::MainDialog)
{
	ui->setupUi(this);

//    m_bProtectForPlayStatus = false;
	m_bDisconnectedCall = false;
//    m_bAudioRequired = false;
//    m_bHasAudioFocus = false;
	m_nRetryCommandCount = 0;
	m_bAudioClosedForHS = false;

	connect(&m_RetryCommandTimer, SIGNAL(timeout()), this, SLOT(slotRetryCommandTimer()));

	m_pBTCommandProcessor = BTCommandProcessor::instance();
	connect(m_pBTCommandProcessor, SIGNAL(signalCommandFromServer(QString)), this, SLOT(slotCommandFromServer(QString)));
	m_pBTCommandProcessor->start();

	// settings for NxStatusBar
	m_pNxStatusBar = new CNX_StatusBar(this);
	m_pNxStatusBar->move(0, 0);
	m_pNxStatusBar->resize(width(), height() / 10);
	m_pNxStatusBar->RegOnClickedHome(callbackStatusHomeButtonClicked);
	m_pNxStatusBar->RegOnClickedBack(callbackStatusBackButtonClicked);
	m_pNxStatusBar->SetTitleName("Nexell BT Phone");

	m_pSelectMenuWidget = new SelectMenuWidget(this);
	m_pSelectMenuWidget->setGeometry(0, 60, 1024, 540);
	connect(m_pSelectMenuWidget, SIGNAL(signalCurrentMenuChanged(Menu)), this, SLOT(slotCurrentMenuChanged(Menu)));

	m_pCallMenuWidget = new CallMenuWidget(this);
	m_pCallMenuWidget->setGeometry(0, 60, 1024, 540);

	m_pMessageMenuWidget = new MessageMenuWidget(this);
	m_pMessageMenuWidget->setGeometry(0, 60, 1024, 540);
	connect(m_pMessageMenuWidget, SIGNAL(signalCommandToServer(QString)), m_pBTCommandProcessor, SLOT(slotCommandToServer(QString)));
	connect(m_pBTCommandProcessor, SIGNAL(signalCommandFromServer(QString)), m_pMessageMenuWidget, SLOT(slotCommandFromServer(QString)));

	m_pCallingDialog = new InCommingCallDialog(this);
	connect(m_pCallingDialog, SIGNAL(signalCommandToServer(QString)), m_pBTCommandProcessor, SLOT(slotCommandToServer(QString)));
	connect(m_pBTCommandProcessor, SIGNAL(signalCommandFromServer(QString)), m_pCallingDialog, SLOT(slotCommandFromServer(QString)));

	setCurrentMenu(g_current_menu);

	NX_PacpClientStart(this);

	installEventFilter(this);

	m_bBTConnectedForHS = true;
}

MainDialog::~MainDialog()
{
	NX_PacpClientStop();

	delete ui;
}

void MainDialog::slotCurrentMenuChanged(Menu menu)
{
	setCurrentMenu(menu);
}

void MainDialog::setCurrentMenu(Menu menu)
{
	g_current_menu = menu;

	switch (g_current_menu) {
	case Menu_Select:
		m_pSelectMenuWidget->show();
		m_pCallMenuWidget->hide();
		m_pMessageMenuWidget->hide();
		m_pCallingDialog->hide();
		break;

	case Menu_Call:
		m_pCallMenuWidget->show();
		m_pSelectMenuWidget->hide();
		m_pMessageMenuWidget->hide();
		m_pCallingDialog->hide();
		break;

	case Menu_Message:
		m_pMessageMenuWidget->show();
		m_pSelectMenuWidget->hide();
		m_pCallMenuWidget->hide();
		m_pCallingDialog->hide();
		break;

	case Menu_Calling:
		m_pCallingDialog->show();
		break;
	}
}
#ifdef CONFIG_NX_DAUDIO_MANAGER
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
//	LOGQTA(Q_FUNC_INFO);

//	QDialog::show();
//}

//void MainDialog::hideEvent(QHideEvent *)
//{
//    if (!isHidden())
//        QDialog::hide();
//    if (!m_pCallingDialog->isHidden())
//        m_pCallingDialog->hide();
//}

void MainDialog::closeEvent(QCloseEvent *)
{
	QDialog::close();

	if (NX_REPLY_FAIL == NX_RequestCommand(NX_REQUEST_PROCESS_REMOVE)) {
		printf("@Fail, NX_RequestCommand(). command = NX_REQUEST_PROCESS_REMOVE\n");
	}
}
#endif /* CONFIG_NX_DAUDIO_MANAGER */

void MainDialog::slotCommandFromServer(QString command)
{
	LOGQTA(command);
	int stx = command.indexOf("$");
	int etx = command.indexOf("\n");

	if (stx < 0 || etx < 0) {
		return;
	}

	stx++;
	QString body = command.mid(stx, etx-stx);

	QStringList tokens = body.split("#");

	// failure conditions
	if (tokens.size() < 3 || tokens[0] == "NG") {
		m_nRetryCommandCount++;
		return;
	}

	m_nRetryCommandCount = 0;

	if (tokens[2] == "CALL STATUS") {
		if (tokens.size() < 4)
			return;

		if (tokens[3] == "INCOMMING CALL") {
			g_previous_menu = g_current_menu;
			g_current_menu = Menu_Calling;
			m_bDisconnectedCall = false;

			// if all menu dialog(except IncommingCallDialog) is hide state, send NX_REQUEST_PROCESS_SHOW to d-audio manager.
			if (isHidden()) {
#ifdef CONFIG_NX_DAUDIO_MANAGER
				if (NX_REPLY_DONE != NX_RequestCommand(NX_REQUEST_PROCESS_SHOW)) {
					LOGQ("NX_REQUEST_PROCESS_SHOW FAIL!");
				}
#else
				m_pInCommingCallDialog->show();
#endif
			} else {
				m_pCallingDialog->show();
			}
		} else if (tokens[3] == "HANG UP CALL" || tokens[3] == "DISCONNECTED CALL") {
			m_bDisconnectedCall = true;

			processForCallDisconnected();
		} else if (tokens[3] == "READY OUTGOING CALL") {
			g_previous_menu = g_current_menu;
			g_current_menu = Menu_Calling;
			m_bDisconnectedCall = false;
		}
	} else if (tokens[2] == "AUDIO MUTE STATUS") {
		if (tokens.size() < 4)
			return;

		if (tokens[3] == "AUDIO CLOSED") {
			m_bAudioClosedForHS = true;

			processForCallDisconnected();
		} else if (tokens[3] == "AUDIO OPENED") {
			m_bAudioClosedForHS = false;
		}
	} else if (tokens[1] == "HS" && tokens[2] == "CONNECTION STATUS") {
		if (tokens.size() < 4)
			return;

		if (tokens[3] == "CONNECTED") {
			m_bBTConnectedForHS = true;
		} else if (tokens[3] == "DISCONNECTED") {
			m_bBTConnectedForHS = false;

			if (g_current_menu == Menu_Calling) {
				processForCallDisconnected();
			}
		}
	}
}

void MainDialog::slotRetryCommandTimer()
{
	m_RetryCommandTimer.stop();

	if (m_nRetryCommandCount <= MAX_RETRY_COMMAND_COUNT) {
		LOGQ("[RETRY COMMAND]" << "[RETRY# " << m_nRetryCommandCount << "]" << m_RetryCommand);
		m_pBTCommandProcessor->commandToServer(m_RetryCommand);
	}
}

void MainDialog::processForCallDisconnected()
{
	if (!m_bBTConnectedForHS || (m_bDisconnectedCall && m_bAudioClosedForHS)) {
		g_current_menu = g_previous_menu;

		// if all menu dialog(except IncommingCallDialog) is hide state, send NX_REQUEST_PROCESS_SHOW to d-audio manager.
		if (g_calling_end_is_exit) {
#ifdef CONFIG_NX_DAUDIO_MANAGER
//			if (NX_REPLY_DONE != NX_RequestCommand(NX_REQUEST_PROCESS_REMOVE, &g_process_info)) {
//				LOGQ("NX_REQUEST_PROCESS_HIDE [FAILED]!");
//			}

			close();
#else
			exit(0);
#endif
		} else {
			if (isHidden()) {
#ifdef CONFIG_NX_DAUDIO_MANAGER
				if (NX_REPLY_DONE != NX_RequestCommand(NX_REQUEST_PROCESS_HIDE)) {
					LOGQ("NX_REQUEST_PROCESS_HIDE [FAILED]!");
				}
#else
				m_pInCommingCallDialog->hide();
#endif
			} else {
				m_pCallingDialog->hide();
			}
		}
	}
}
