#include "MainFrame.h"
#include "ui_MainFrame.h"

MainFrame* MainFrame::m_spInstance = NULL;

// Launcher Show
void (*MainFrame::m_pRequestLauncherShow)(bool *bOk) = NULL;
// Popup Message
void (*MainFrame::m_RequestPopupMessage)(PopupMessage *, bool *) = NULL;
// Send Message
void (*MainFrame::m_pRequestSendMessage)(const char *pDst, const char *pMsg, int32_t iMsgSize) = NULL;
// Video
void (*MainFrame::m_pRequestVideoFocus)(FocusPriority ePriority, bool *bOk) = NULL;
void (*MainFrame::m_pRequestVideoFocusTransient)(FocusPriority ePriority, bool *bOk) = NULL;
void (*MainFrame::m_pRequestVideoFocusLoss)(void) = NULL;
// Terminate
void (*MainFrame::m_pRequestTerminate)(void) = NULL;

#include <QDebug>

#define LOG_TAG "[NxBTSettings]"
#include <NX_Log.h>

MainFrame::MainFrame(QWidget *parent) :
	QFrame(parent),
	ui(new Ui::MainFrame)
{
	ui->setupUi(this);

	move(0, 60);

	m_bInitialized = false;

	m_pCommandProcessor = new BTCommandProcessor();
	connect(m_pCommandProcessor, SIGNAL(signalCommandFromServer(QString)), this, SLOT(slotCommandFromServer(QString)));
	connect(m_pCommandProcessor, SIGNAL(signalCommandFromServer(QString)), ui->connectionMenuFrame, SLOT(slotCommandFromServer(QString)));
	connect(ui->connectionMenuFrame, SIGNAL(signalCommandToServer(QString)), m_pCommandProcessor, SLOT(slotCommandToServer(QString)));
	connect(m_pCommandProcessor, SIGNAL(signalCommandFromServer(QString)), ui->advancedOptionFrame, SLOT(slotCommandFromServer(QString)));
	connect(ui->advancedOptionFrame, SIGNAL(signalCommandToServer(QString)), m_pCommandProcessor, SLOT(slotCommandToServer(QString)));
	m_pCommandProcessor->start();

	connect(ui->selectMenuFrame, SIGNAL(signalCurrentMenuChanged(Menu)), this, SLOT(slotCurrentMenuChanged(Menu)));

	if (m_pRequestSendMessage)
		m_pCommandProcessor->RegisterRequestSendMessage(m_pRequestSendMessage);

	setCurrentMenu(Menu_Init);

	m_pLoadingImage = new QMovie("://UI/loading3_100x100.gif");
	ui->LABEL_LOADING_IMAGE->setMovie(m_pLoadingImage);
	m_pLoadingImage->start();
}

MainFrame::~MainFrame()
{
	delete ui;

	delete m_pCommandProcessor;
}

MainFrame* MainFrame::GetInstance(void *pObj)
{
	qDebug() << Q_FUNC_INFO << 1;
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

bool MainFrame::Initialize()
{
	bool bOk = false;

	if (m_bInitialized)
		return true;

	if (!m_pRequestVideoFocus)
	{
		NXLOGE("[%s] REQUEST VIDEO FOCUS does not exist.", __FUNCTION__);
		return false;
	}

	m_pRequestVideoFocus(FocusPriority_Normal, &bOk);
	if (!bOk)
	{
		NXLOGE("[%s] REQUEST VIDEO FOCUS <FAIL>", __FUNCTION__);
		return false;
	}

	if (isHidden())
		show();
	raise();

	m_bInitialized = true;

	// check for initialized state of service.
	m_pCommandProcessor->CommandToServer("$MGT#PING\n");

	return true;
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

void MainFrame::StatusHomeEvent(NxStatusHomeEvent *)
{
	if (m_pRequestLauncherShow)
	{
		bool bOk = false;
		m_pRequestLauncherShow(&bOk);
		NXLOGI("[%s] REQUEST LAUNCHER SHOW <%s>", __FUNCTION__, bOk ? "OK" : "NG");
	}
}

void MainFrame::StatusBackEvent(NxStatusBackEvent *)
{
	Menu eMenu = GetCurrentMenu();

	switch (eMenu) {
	case Menu_Init:
	case Menu_Select:
		if (m_pRequestTerminate)
			m_pRequestTerminate();
		break;

	case Menu_Connection:
		setCurrentMenu(Menu_Select);
		break;

	case Menu_Advanced:
		setCurrentMenu(Menu_Select);
		break;
	}
}

void MainFrame::cbStatusHome(void *pObj)
{
	MainFrame *p = (MainFrame *)pObj;
	QApplication::postEvent(p, new NxStatusHomeEvent());
}

void MainFrame::cbStatusBack(void *pObj)
{
	MainFrame *p = (MainFrame *)pObj;
	Menu eMenu = p->GetCurrentMenu();

	switch (eMenu) {
	case Menu_Init:
	case Menu_Select:
		QApplication::postEvent(p, new NxStatusBackEvent());
		break;

	case Menu_Connection:
		p->setCurrentMenu(Menu_Select);
		break;

	case Menu_Advanced:
		p->setCurrentMenu(Menu_Select);
		break;
	}
}

void MainFrame::setCurrentMenu(Menu menu)
{
	m_Menu = menu;

	switch (menu) {
	case Menu_Init:
		ui->selectMenuFrame->hide();
		ui->connectionMenuFrame->hide();
		ui->advancedOptionFrame->hide();
		break;

	case Menu_Select:
		ui->selectMenuFrame->show();
		ui->connectionMenuFrame->hide();
		ui->advancedOptionFrame->hide();
		break;

	case Menu_Connection:
		ui->connectionMenuFrame->show();
		ui->selectMenuFrame->hide();
		ui->advancedOptionFrame->hide();
		break;

	case Menu_Advanced:
		ui->advancedOptionFrame->show();
		ui->selectMenuFrame->hide();
		ui->connectionMenuFrame->hide();
		break;
	}
}

Menu MainFrame::GetCurrentMenu()
{
	return m_Menu;
}

void MainFrame::slotCommandFromServer(QString command)
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

void MainFrame::slotCurrentMenuChanged(Menu eMenu)
{
	setCurrentMenu(eMenu);
}

// Message
void MainFrame::SendMessage(QString msg)
{
	qDebug() << msg;
	m_pCommandProcessor->Push(msg);
}

void MainFrame::RegisterRequestSendMessage(void (*cbFunc)(const char *pDst, const char *pMsg, int32_t iMsgSize))
{
	if (cbFunc)
		m_pRequestSendMessage = cbFunc;
}

// Popup Message
void MainFrame::RegisterRequestPopupMessage(void (*cbFunc)(PopupMessage *, bool *))
{
	if (cbFunc)
		m_RequestPopupMessage = cbFunc;
}

void MainFrame::PopupMessageResponse(bool bOk)
{

}

// Video Focus
void MainFrame::RequestVideoFocus(FocusType eType, FocusPriority ePriority, bool *bOk)
{
	if (eType == FocusType_Get)
	{
		FocusPriority eCurrPriority = FocusPriority_Normal;

		if (eCurrPriority > ePriority)
			*bOk = false;
		else
			*bOk = true;

		m_bHasVideoFocus = *bOk ? false : true;
	}
	else // FocusType_Set
	{
		NXLOGI("[%s] SET", __FUNCTION__);
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

void MainFrame::RegisterRequestTerminate(void (*cbFunc)())
{
	if (cbFunc)
		m_pRequestTerminate = cbFunc;
}

void MainFrame::BackButtonClicked()
{
	QApplication::postEvent(this, new NxStatusBackEvent());
}
