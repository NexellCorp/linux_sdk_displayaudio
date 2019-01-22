#include "MainFrame.h"
#include "ui_MainFrame.h"
#include <QDesktopWidget>

#define DEFAULT_WIDTH	1024
#define DEFAULT_HEIGHT	540

#define LOG_TAG "[NxBTPhone]"
#include <NX_Log.h>

MainFrame* MainFrame::m_spInstance = NULL;

void (*MainFrame::m_pRequestLauncherShow)(bool *bOk) = NULL;
void (*MainFrame::m_pRequestSendMessage)(const char *pDst, const char *pMsg, int32_t iMsgSize) = NULL;
void (*MainFrame::m_pRequestVideoFocus)(FocusPriority ePriority, bool *bOk) = NULL;
void (*MainFrame::m_pRequestVideoFocusTransient)(FocusPriority ePriority, bool *bOk) = NULL;
void (*MainFrame::m_pRequestVideoFocusLoss)(void) = NULL;
void (*MainFrame::m_pRequestTerminate)(void) = NULL;

MainFrame* MainFrame::GetInstance(void *pObj)
{
	if (!m_spInstance)
		m_spInstance = new MainFrame((QWidget *)pObj);

	return m_spInstance;
}

MainFrame* MainFrame::GetInstance()
{
	return m_spInstance;
}

void MainFrame::DestroyInstance()
{
	if (m_spInstance)
	{
		delete m_spInstance;
		m_spInstance = NULL;
	}
}

MainFrame::MainFrame(QWidget *parent) :
	QFrame(parent),
	ui(new Ui::MainFrame)
{
	ui->setupUi(this);

	const QRect screen = QApplication::desktop()->screenGeometry();
	move(0, screen.height() * 0.1);

	if ((width() != screen.width()) || (height() != screen.height()))
	{
		setFixedSize(screen.width(), screen.height() * 0.9);
	}

	m_bHasVideoFocus = false;
	m_bHasVideoFocusTransient = false;

	m_bDisconnectedCall = false;
	m_bAudioClosedForHS = false;
	m_bBTConnectedForHS = true;

	m_pCommandProcessor = new BTCommandProcessor();

	connect(this, SIGNAL(signalCommandToServer(QString)), m_pCommandProcessor, SLOT(slotCommandToServer(QString)));
	connect(m_pCommandProcessor, SIGNAL(signalCommandFromServer(QString)), this, SLOT(slotCommandFromServer(QString)));

	// connection: call widget AND command processor
	connect(ui->callMenu, SIGNAL(signalCommandToServer(QString)), m_pCommandProcessor, SLOT(slotCommandToServer(QString)));
	connect(m_pCommandProcessor, SIGNAL(signalCommandFromServer(QString)), ui->callMenu, SLOT(slotCommandFromServer(QString)));
	// connection: message widget AND command processor
	connect(ui->messageMenu, SIGNAL(signalCommandToServer(QString)), m_pCommandProcessor, SLOT(slotCommandToServer(QString)));
	connect(m_pCommandProcessor, SIGNAL(signalCommandFromServer(QString)), ui->messageMenu, SLOT(slotCommandFromServer(QString)));
	// connection: calling widget AND command processor
	connect(ui->callingMenu, SIGNAL(signalCommandToServer(QString)), m_pCommandProcessor, SLOT(slotCommandToServer(QString)));
	connect(m_pCommandProcessor, SIGNAL(signalCommandFromServer(QString)), ui->callingMenu, SLOT(slotCommandFromServer(QString)));

	if (m_pRequestSendMessage)
		m_pCommandProcessor->RegisterRequestSendMessage(m_pRequestSendMessage);

	m_pCommandProcessor->start();

	connect(ui->selectMenu, SIGNAL(signalCurrentMenuChanged(Menu)), this, SLOT(slotCurrentMenuChanged(Menu)));

	SetCurrentMenu(Menu_Select);
}

MainFrame::~MainFrame()
{
	delete ui;
}

bool MainFrame::event(QEvent *event)
{
	switch ((int)event->type()) {
	case E_NX_EVENT_STATUS_BACK:
	{
		NxStatusBackEvent *e = static_cast<NxStatusBackEvent *>(event);
		StatusBackEvent(e);
		return true;
	}

	default: break;
	}

	return QFrame::event(event);
}

bool MainFrame::Initialize(QString args)
{
	bool bOk = false;
	FocusPriority ePriority = FocusPriority_Normal;

	if (m_bInitialized)
		return true;

	if (!m_pRequestVideoFocus)
	{
		NXLOGE("[%s] REQUEST VIDEO FOCUS does not exist.", __FUNCTION__);
		return false;
	}

	if (args.indexOf("--menu calling") > -1)
	{
		ePriority = FocusPriority_High;
		m_bDisconnectedCallIsExit = true;
	}

	m_pRequestVideoFocus(ePriority, &bOk);
	if (!bOk)
	{
		NXLOGE("[%s] REQUEST VIDEO FOCUS <FAIL>", __FUNCTION__);
		return false;
	}

	ui->callMenu->Initialize();

	m_bHasVideoFocus = true;

	if (isHidden())
		show();
	raise();

	m_bInitialized = true;

	return true;
}

void MainFrame::RegisterRequestSendMessage(void (*cbFunc)(const char *pDst, const char *pMsg, int32_t iMsgSize))
{
	if (cbFunc)
		m_pRequestSendMessage = cbFunc;
}

void MainFrame::SendMessage(QString msg)
{
	if (m_pCommandProcessor)
		m_pCommandProcessor->Push(msg);
}


void MainFrame::RequestVideoFocus(FocusType eType, FocusPriority ePriority, bool *bOk)
{
	if (eType == FocusType_Get)
	{
		//
		FocusPriority eCurrPriority = FocusPriority_Normal;
		if (eCurrPriority > ePriority)
			*bOk = false;
		else
			*bOk = true;

		m_bHasVideoFocus = false;
	}
	else
	{
		*bOk = true;
		m_bHasVideoFocus = true;

		if (isHidden())
			show();
		raise();
	}
}

void MainFrame::RegisterRequestVideoFocus(void (*cbFunc)(FocusPriority ePriority, bool *bOk))
{
	if (cbFunc)
		m_pRequestVideoFocus = cbFunc;
}

void MainFrame::RequestVideoFocusTransient(FocusPriority ePriority, bool *bOk)
{
	FocusPriority eCurrPriority = FocusPriority_Normal;

	if (eCurrPriority > ePriority)
		*bOk = false;
	else
		*bOk = true;

	m_bHasVideoFocus = *bOk ? false : true;
}

void MainFrame::RegisterRequestVideoFocusTransient(void (*cbFunc)(FocusPriority ePriority, bool *bOk))
{
	if (cbFunc)
		m_pRequestVideoFocusTransient = cbFunc;
}

void MainFrame::RegisterRequestVideoFocusLoss(void (*cbFunc)(void))
{
	if (cbFunc)
		m_pRequestVideoFocusLoss = cbFunc;
}

void MainFrame::RegisterRequestTerminate(void (*cbFunc)(void))
{
	if (cbFunc)
		m_pRequestTerminate = cbFunc;
}

void MainFrame::StatusBackEvent(NxStatusBackEvent *)
{
	switch (m_eCurrentMenu) {
	case Menu_Select:
		if (m_pRequestTerminate)
			m_pRequestTerminate();
		break;

	case Menu_Call:
	case Menu_Message:
		SetCurrentMenu(Menu_Select);
		break;

	default: break;
	}
}

void MainFrame::SetCurrentMenu(Menu menu, bool update/*= true*/)
{
	if (update)
		m_eCurrentMenu = menu;

	switch (menu) {
	case Menu_Select:
		ui->selectMenu->raise();
		break;

	case Menu_Call:
		ui->callMenu->raise();
		break;

	case Menu_Message:
		ui->messageMenu->raise();
		break;

	case Menu_Calling:
	{
		if (!m_bHasVideoFocus && !m_bHasVideoFocusTransient)
		{
			bool bOk = false;

			NXLOGI("[%s] Try Video focus transient 1", __FUNCTION__);
			if (m_pRequestVideoFocusTransient)
				m_pRequestVideoFocusTransient(FocusPriority_High, &bOk);
			NXLOGI("[%s] Try Video focus transient 2", __FUNCTION__);
			if (bOk)
				m_bHasVideoFocusTransient = true;
			else
				NXLOGE("[%s]", __FUNCTION__);
		}

		if (m_bHasVideoFocus || m_bHasVideoFocusTransient)
		{
			ui->callingMenu->raise();
			raise();
		}

		break;
	}

	case Menu_CallingEnd:
	{
		ui->callingMenu->lower();
		if (m_bHasVideoFocusTransient)
		{
			m_bHasVideoFocusTransient = false;
			if (m_pRequestVideoFocusLoss)
				m_pRequestVideoFocusLoss();
			lower();
		}
		break;
	}
	}
}

void MainFrame::slotCurrentMenuChanged(Menu eMenu)
{
	SetCurrentMenu(eMenu);
}

void MainFrame::slotCommandFromServer(QString command)
{
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
		return;
	}

	if (tokens[2] == "CALL STATUS") {
		if (tokens.size() < 4)
			return;

		if (tokens[3] == "INCOMMING CALL") {
			m_bDisconnectedCall = false;
			SetCurrentMenu(Menu_Calling, false);
		} else if (tokens[3] == "HANG UP CALL" || tokens[3] == "DISCONNECTED CALL") {
			m_bDisconnectedCall = true;

			ProcessForCallDisconnected();
		} else if (tokens[3] == "READY OUTGOING CALL") {
			m_bDisconnectedCall = false;
			SetCurrentMenu(Menu_Calling, false);
		}
	} else if (tokens[2] == "AUDIO MUTE STATUS") {
		if (tokens.size() < 4)
			return;

		if (tokens[3] == "AUDIO CLOSED") {
			m_bAudioClosedForHS = true;

			ProcessForCallDisconnected();
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
			ProcessForCallDisconnected();

		}
	}
}

void MainFrame::ProcessForCallDisconnected()
{
	if (!m_bBTConnectedForHS || (m_bDisconnectedCall && m_bAudioClosedForHS)) {
		SetCurrentMenu(Menu_CallingEnd, false);

		// if all menu dialog(except IncommingCallDialog) is hide state, send NX_REQUEST_PROCESS_SHOW to d-audio manager.
		if (m_bDisconnectedCallIsExit) {
			if (m_pRequestTerminate)
				m_pRequestTerminate();
		}
	}
}

void MainFrame::BackButtonClicked()
{
	QApplication::postEvent(this, new NxStatusBackEvent());
}
