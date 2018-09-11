#include "Frame.h"
#include "ui_Frame.h"
#include "TimeUtility.hpp"

#define LOG_TAG "[NxBTAudio]"
#include <NX_Log.h>

Frame* Frame::m_spInstance = NULL;

void (*Frame::m_pRequestLauncherShow)(bool *bOk) = NULL;
void (*Frame::m_pRequestSendMessage)(const char *pDst, const char *pMsg, int32_t iMsgSize) = NULL;
void (*Frame::m_pRequestVideoFocus)(FocusPriority ePriority, bool *bOk) = NULL;
void (*Frame::m_pRequestVideoFocusTransient)(FocusPriority ePriority, bool *bOk) = NULL;
void (*Frame::m_pRequestVideoFocusLoss)(void) = NULL;
void (*Frame::m_pRequestTerminate)(void) = NULL;

#include <QDebug>
Frame::Frame(QWidget *parent) :
	QFrame(parent),
	ui(new Ui::Frame)
{
	ui->setupUi(this);

	m_bInitialized = false;

	ui->statusBar->SetTitleName("Nexell BT Audio Player");
	ui->statusBar->RegOnClickedHome(cbStatusHome);
	ui->statusBar->RegOnClickedBack(cbStatusBack);

	setUIState(UIState_Stopped);

	m_pCommandProcessor = new BTCommandProcessor();
	connect(m_pCommandProcessor, SIGNAL(signalCommandFromServer(QString)), this, SLOT(slotCommandFromServer(QString)));
	connect(this, SIGNAL(signalCommandToServer(QString)), m_pCommandProcessor, SLOT(slotCommandToServer(QString)));

	m_pCommandProcessor->start();

	if (m_pRequestSendMessage)
		m_pCommandProcessor->RegisterRequestSendMessage(m_pRequestSendMessage);
}

Frame::~Frame()
{
	delete m_pCommandProcessor;

	delete ui;
}

bool Frame::Initialize()
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

	emit signalCommandToServer("$AVK#IS CONNECTED\n");
	emit signalCommandToServer("$AVK#PLAY STATUS\n");
	emit signalCommandToServer("$AVK#PLAY INFO\n");
	emit signalCommandToServer("$AVK#UPDATE MEDIA ELEMENT\n");

	return true;
}

bool Frame::event(QEvent *event)
{
	switch ((int)event->type()) {
	case E_NX_EVENT_STATUS_HOME:
	{
		NxStatusHomeEvent *e = static_cast<NxStatusHomeEvent *>(event);
		StatusHomeEvent(e);
		return true;
	}

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

void Frame::StatusHomeEvent(NxStatusHomeEvent *)
{
	if (m_pRequestLauncherShow)
	{
		bool bOk = false;
		m_pRequestLauncherShow(&bOk);
		NXLOGI("[%s] REQUEST LAUNCHER SHOW <%s>", __FUNCTION__, bOk ? "OK" : "NG");
	}
}

void Frame::StatusBackEvent(NxStatusBackEvent *)
{
	if (m_pRequestTerminate)
		m_pRequestTerminate();
}

Frame* Frame::GetInstance(void *pObj)
{
	if (!m_spInstance)
		m_spInstance = new Frame((QWidget *)pObj);

	return m_spInstance;
}

Frame* Frame::GetInstance()
{
	return m_spInstance;
}

void Frame::DestroyInstance()
{
	if (m_spInstance)
	{
		delete m_spInstance;
		m_spInstance = NULL;
	}
}

void Frame::cbStatusHome(void *pObj)
{
	Frame *p = (Frame *)pObj;
	QApplication::postEvent(p, new NxStatusHomeEvent());
}

void Frame::cbStatusBack(void *pObj)
{
	Frame *p = (Frame *)pObj;
	QApplication::postEvent(p, new NxStatusBackEvent());
}

void Frame::setUIState(UIState state)
{
	m_UIState = state;

	switch (state) {
	case UIState_Playing:
		ui->BUTTON_PLAY_PAUSE->show();
		ui->BUTTON_PLAY_START->hide();
		break;

	case UIState_Paused:
	case UIState_Stopped:
		ui->BUTTON_PLAY_START->show();
		ui->BUTTON_PLAY_PAUSE->hide();
		break;

	default: break;
	}
}

void Frame::on_BUTTON_PLAY_START_clicked()
{
	emit signalCommandToServer("$AVK#PLAY START\n");
}

void Frame::on_BUTTON_PLAY_PAUSE_clicked()
{
	emit signalCommandToServer("$AVK#PLAY PAUSE\n");
}

void Frame::on_BUTTON_PLAY_PREV_clicked()
{
	emit signalCommandToServer("$AVK#PLAY PREV\n");
}

void Frame::on_BUTTON_PLAY_NEXT_clicked()
{
	emit signalCommandToServer("$AVK#PLAY NEXT\n");
}
#include <QDebug>
void Frame::slotCommandFromServer(QString command)
{
	NXLOGI("Receive Command = %s", command.toStdString().c_str());
	int stx = command.indexOf("$");
	int etx = command.indexOf("\n");

	// assume unknown command format.
	if (stx < 0 || etx < 0) {
		return;
	}

	// body = remove STX and ETX from command
	stx++;
	QString body = command.mid(stx, etx-stx);

	// example) command = "$OK#MGT#LOCAL DEVICE NAME\n"
	// example) body    = "OK#MGT#LOCAL DEVICE NAME"

	QStringList tokens = body.split("#");

	// failure conditions
	if (tokens.size() < 3) {
		return;
	} else if (tokens[0] == "NG") {
		return;
	} else if (tokens[1] != "AVK") {
		return;
	}

	// valid commands
	if (tokens[2] == "UPDATE MEDIA ELEMENT") {
		updateToUIForMediaElements(tokens);
	} else if (tokens[2] == "UPDATE PLAY POSITION") {
		updateToUIForPlayPosition(tokens);
	} else if (tokens[2] == "PLAY START") {
		setUIState(UIState_Playing);
	} else if (tokens[2] == "PLAY PAUSE") {
		setUIState(UIState_Paused);
	} else if (tokens[2] == "PLAY STOP") {
		setUIState(UIState_Stopped);
	} else if (tokens[2] == "UPDATE PLAY STATUS") {
		updateToUIForPlayStatus(tokens);
	} else if (tokens[2] == "PLAY STATUS") {
		updateToUIForPlayStatus(tokens);
	} else if (tokens[2] == "PLAY INFO") {
		updateToUIForPlayInformation(tokens);
	} else if (tokens[2] == "IS CONNECTED") {
		if (tokens.size() >= 4 && tokens[3] == "CONNECTED")
			emit signalCommandToServer("$AVK#PLAY START\n");
	}
}

void Frame::updateToUIForMediaElements(QStringList& tokens)
{
	// example) "$OK#AVK#UPDATE MEDIA ELEMENT#[TITLE]#[ARTIST]#[ALBUM]#[GENRE]#[DURATION]\n"
	if (tokens.size() == 8) {
		 // update title
		ui->LABEL_MDEDIA_TITLE->setText(tokens[3]);
		// update artist, album, genre
		ui->LABEL_MDEDIA_ETC->setText(QString("%1\n%2\n%3").arg(tokens[4]).arg(tokens[5]).arg(tokens[6]));
		// duration (play time)

		int duration = tokens[7].toInt(); // mili second
		std::string s_duration = convertToRealtimeString(duration);
		ui->LABEL_PLAY_DURATION->setText(QString::fromStdString(s_duration));
		ui->SLIDER_PLAY_POSITION->setRange(0, duration);

		// iphone: play position is unknown.
		if (duration == 0) {
			ui->LABEL_PLAY_POSITION->setText("00:00");
			ui->SLIDER_PLAY_POSITION->setValue(0);
		}
	}
}

void Frame::updateToUIForPlayPosition(QStringList& tokens)
{
	// example) $OK#AVK#UPDATE PLAY POSITION#12345\n"
	if (tokens.size() == 4) {
		int play_position = tokens[3].toInt();
		std::string s_play_position = convertToRealtimeString(play_position);
		ui->LABEL_PLAY_POSITION->setText(QString::fromStdString(s_play_position));
		ui->SLIDER_PLAY_POSITION->setValue(play_position);
	}
}

void Frame::updateToUIForPlayStatus(QStringList& tokens)
{
	// example) "$OK#AVK#UPDATE PLAY STATUS#[STATUS]\n"

	if (tokens.size() == 4) {
		if (tokens[3] == "PLAYING") {
			setUIState( UIState_Playing );

//			if (isHidden()) {

//				if (m_pRequestShow)
//				{
//					bool bOk = false;
//					m_pRequestShow(&bOk);

//					if (!bOk)
//						NXLOGE("[%s] RequestShow", Q_FUNC_INFO);
//				}
//			}
		} else if (tokens[3] == "PAUSED") {
			setUIState(UIState_Paused);
		} else if (tokens[3] == "STOPPED") {
			setUIState(UIState_Paused);
		}
	}
}

void Frame::updateToUIForPlayInformation(QStringList& tokens)
{
	// example) "$OK#AVK#PLAY INFO#[title]#[artist]#[album]#[genre]#[duration]#[play position]\n"
	// OK, AVK, PLAY INFO, title, artist, album, genre, duration, play position

	if (tokens.size() == 9) {
		int value;
		std::string s_value;

		// update title
		ui->LABEL_MDEDIA_TITLE->setText(tokens[3]);

		// update artist, album, genre
		ui->LABEL_MDEDIA_ETC->setText(QString("%1\n%2\n%3").arg(tokens[4]).arg(tokens[5]).arg(tokens[6]));

		// update duration
		value = tokens[7].toInt(); // mili second
		s_value = convertToRealtimeString(value);
		ui->LABEL_PLAY_DURATION->setText(QString::fromStdString(s_value));
		ui->SLIDER_PLAY_POSITION->setRange(0, value);

		// update play position
		value = tokens[8].toInt(); // mili second
		s_value = convertToRealtimeString(value);
		ui->LABEL_PLAY_POSITION->setText(QString::fromStdString(s_value));
		ui->SLIDER_PLAY_POSITION->setValue(value);
	}
}

/************************************************************************************\
 * D-AUDIO INTERFACE - Init
 *
 * Description
 *  -
 ************************************************************************************/
void Frame::Init(void *pObj)
{
	Frame::GetInstance(pObj);
}

/************************************************************************************\
 * D-AUDIO INTERFACE - Init
 *
 * Description
 *  -
 ************************************************************************************/
void Frame::IsInit(bool *bOk)
{
	*bOk = Frame::GetInstance() ? true : false;
}

/************************************************************************************\
 * D-AUDIO INTERFACE - Init
 *
 * Description
 *  -
 ************************************************************************************/
void Frame::deInit()
{
	Frame::DestroyInstance();
}

/************************************************************************************\
 * D-AUDIO INTERFACE - Init
 *
 * Description
 *  -
 ************************************************************************************/
void Frame::Show()
{
	Frame *p = Frame::GetInstance();
	if (p)
		p->show();
}

/************************************************************************************\
 * D-AUDIO INTERFACE - Init
 *
 * Description
 *  -
 ************************************************************************************/
void Frame::Hide()
{
	Frame *p = Frame::GetInstance();
	if (p)
		p->hide();
}

/************************************************************************************\
 * D-AUDIO INTERFACE - Init
 *
 * Description
 *  -
 ************************************************************************************/
void Frame::Raise()
{
	Frame *p = Frame::GetInstance();
	if (p)
		p->raise();
}

/************************************************************************************\
 * D-AUDIO INTERFACE - Init
 *
 * Description
 *  -
 ************************************************************************************/
void Frame::Lower()
{
	Frame *p = Frame::GetInstance();
	if (p)
		p->lower();
}

void Frame::RegisterRequestLauncherShow(void (*cbFunc)(bool *bOk))
{
	if (cbFunc)
		m_pRequestLauncherShow = cbFunc;
}
/************************************************************************************\
 * D-AUDIO INTERFACE - Init
 *
 * Description
 *  -
 ************************************************************************************/
void Frame::RegisterRequestSendMessage(void (*cbFunc)(const char *pDst, const char *pMsg, int32_t iMsgSize))
{
	if (cbFunc)
		m_pRequestSendMessage = cbFunc;
//		m_pCommandProcessor->RegisterRequestSendMessage(cbFunc);
}

/************************************************************************************\
 * D-AUDIO INTERFACE - Init
 *
 * Description
 *  -
 ************************************************************************************/
void Frame::SendMessage(QString msg)
{
	NXLOGI("[%s] %s", __FUNCTION__, msg.toStdString().c_str());
	m_pCommandProcessor->Push(msg);
}

/************************************************************************************\
 * D-AUDIO INTERFACE - Init
 *
 * Description
 *  -
 ************************************************************************************/
void Frame::RequestVideoFocus(FocusType eType, FocusPriority ePriority, bool *bOk)
{
	if (eType == FocusType_Get)
	{
		FocusPriority eCurrPriority = FocusPriority_Normal;
		if (eCurrPriority > ePriority)
			*bOk = false;
		else
			*bOk = true;
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

/************************************************************************************\
 * D-AUDIO INTERFACE - Init
 *
 * Description
 *  -
 ************************************************************************************/
void Frame::RegisterRequestVideoFocus(void (*cbFunc)(FocusPriority ePriority, bool *bOk))
{
	if (cbFunc)
		m_pRequestVideoFocus = cbFunc;
}

/************************************************************************************\
 * D-AUDIO INTERFACE - RequestVideoFocusTransient
 *
 * Description
 *  -
 ************************************************************************************/
void Frame::RequestVideoFocusTransient(FocusPriority ePriority, bool *bOk)
{
	FocusPriority eCurrPriority = FocusPriority_Normal;

	if (eCurrPriority > ePriority)
		*bOk = false;
	else
		*bOk = true;

	m_bHasVideoFocus = *bOk ? false : true;
}

/************************************************************************************\
 * D-AUDIO INTERFACE - RegisterRequestVideoFocusTransient
 *
 * Description
 *  -
 ************************************************************************************/
void Frame::RegisterRequestVideoFocusTransient(void (*cbFunc)(FocusPriority ePriority, bool *bOk))
{
	if (cbFunc)
		m_pRequestVideoFocusTransient = cbFunc;
}

/************************************************************************************\
 * D-AUDIO INTERFACE - RegisterRequestVideoFocusLoss
 *
 * Description
 *  -
 ************************************************************************************/
void Frame::RegisterRequestVideoFocusLoss(void (*cbFunc)(void))
{
	if (cbFunc)
		m_pRequestVideoFocusLoss = cbFunc;
}

/************************************************************************************\
 * D-AUDIO INTERFACE - RegisterRequestVideoFocusLoss
 *
 * Description
 *  -
 ************************************************************************************/
void Frame::RegisterRequestTerminate(void (*cbFunc)(void))
{
	if (cbFunc)
		m_pRequestTerminate = cbFunc;
}
