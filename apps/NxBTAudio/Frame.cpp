#include "Frame.h"
#include "ui_Frame.h"
#include "TimeUtility.hpp"
#include <QDesktopWidget>

#define LOG_TAG "[NxBTAudio]"
#include <NX_Log.h>

#define DEFAULT_WIDTH	1024
#define DEFAULT_HEIGHT	540

Frame* Frame::m_spInstance = NULL;

void (*Frame::m_pRequestSendMessage)(const char *pDst, const char *pMsg, int32_t iMsgSize) = NULL;
void (*Frame::m_pRequestVideoFocus)(FocusPriority ePriority, bool *bOk) = NULL;
void (*Frame::m_pRequestVideoFocusTransient)(FocusPriority ePriority, bool *bOk) = NULL;
void (*Frame::m_pRequestVideoFocusLoss)(void) = NULL;
void (*Frame::m_pRequestTerminate)(void) = NULL;

Frame::Frame(QWidget *parent) :
	QFrame(parent),
	ui(new Ui::Frame)
{
	ui->setupUi(this);

	m_bInitialized = false;

	QPixmap icon = QPixmap(":/audio/UI/default.jpg");
	ui->LABEL_ICON->setAlignment(Qt::AlignCenter);
	ui->LABEL_ICON->setPixmap(icon.scaled(ui->LABEL_ICON->width(), ui->LABEL_ICON->height(), Qt::KeepAspectRatio));

	const QRect screen = QApplication::desktop()->screenGeometry();
	move(0, screen.height() * 0.1);

	if ((width() != screen.width()) || (height() != screen.height()))
	{
		setFixedSize(screen.width(), screen.height() * 0.9);
	}

	setUIState(UIState_Stopped);

	m_pCommandProcessor = new BTCommandProcessor();
	connect(m_pCommandProcessor, SIGNAL(signalCommandFromServer(QString)), this, SLOT(slotCommandFromServer(QString)));
	connect(this, SIGNAL(signalCommandToServer(QString)), m_pCommandProcessor, SLOT(slotCommandToServer(QString)));

	if (m_pRequestSendMessage)
		m_pCommandProcessor->RegisterRequestSendMessage(m_pRequestSendMessage);

	m_pCommandProcessor->start();
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

	case E_NX_EVENT_STATUS_VOLUME:
	{
		NxStatusVolumeEvent *e = static_cast<NxStatusVolumeEvent *>(event);
		StatusVolumeEvent(e);
		return true;
	}

	default: break;
	}

	return QFrame::event(event);
}

void Frame::resizeEvent(QResizeEvent *)
{
	if ((width() != DEFAULT_WIDTH) || (height() != DEFAULT_HEIGHT))
	{
		SetupUI();
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

void Frame::BackButtonClicked()
{
	emit signalCommandToServer("$AVK#PLAY STOP\n");
	QApplication::postEvent(this, new NxStatusBackEvent());
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

void Frame::SetupUI()
{
	float widthRatio = (float)width() / DEFAULT_WIDTH;
	float heightRatio = (float)height() / DEFAULT_HEIGHT;
	int rx, ry, rw, rh;

	// play position - slider
	rx = widthRatio * ui->SLIDER_PLAY_POSITION->x();
	ry = heightRatio * ui->SLIDER_PLAY_POSITION->y();
	rw = widthRatio * ui->SLIDER_PLAY_POSITION->width();
	rh = heightRatio * ui->SLIDER_PLAY_POSITION->height();
	ui->SLIDER_PLAY_POSITION->setGeometry(rx, ry, rw, rh);

	// icon
	ry = heightRatio * ui->LABEL_ICON->y();
	rw = widthRatio * ui->LABEL_ICON->width();
	rh = heightRatio * ui->LABEL_ICON->height();
	ui->LABEL_ICON->setGeometry(rx, ry, rw, rh);
	QPixmap icon = QPixmap(":/audio/UI/default.jpg");
	icon = icon.scaled(ui->LABEL_ICON->width(), ui->LABEL_ICON->height(), Qt::KeepAspectRatio);
	ui->LABEL_ICON->setFixedSize(icon.width(), icon.height());
	ui->LABEL_ICON->setAlignment(Qt::AlignCenter);
	ui->LABEL_ICON->setPixmap(icon);

	// title
	rx = ui->LABEL_ICON->x() + ui->LABEL_ICON->width() + 19 * widthRatio;
	ry = heightRatio * ui->LABEL_MDEDIA_TITLE->y();
	rw = widthRatio * ui->LABEL_MDEDIA_TITLE->width();
	rh = heightRatio * ui->LABEL_MDEDIA_TITLE->height();
	ui->LABEL_MDEDIA_TITLE->setGeometry(rx, ry, rw, rh);

	// etc - albums, artists, genres
	ry = heightRatio * ui->LABEL_MDEDIA_ETC->y();
	rh = heightRatio * ui->LABEL_MDEDIA_ETC->height();
	ui->LABEL_MDEDIA_ETC->setGeometry(rx, ry, rw, rh);

	// play position - label
	rx = widthRatio * ui->LABEL_PLAY_POSITION->x();
	ry = heightRatio * ui->LABEL_PLAY_POSITION->y();
	rw = widthRatio * ui->LABEL_PLAY_POSITION->width();
	rh = heightRatio * ui->LABEL_PLAY_POSITION->height();
	ui->LABEL_PLAY_POSITION->setGeometry(rx, ry, rw, rh);

	// duration - label
	rx = widthRatio * ui->LABEL_PLAY_DURATION->x();
	ry = heightRatio * ui->LABEL_PLAY_DURATION->y();
	rw = widthRatio * ui->LABEL_PLAY_DURATION->width();
	rh = heightRatio * ui->LABEL_PLAY_DURATION->height();
	ui->LABEL_PLAY_DURATION->setGeometry(rx, ry, rw, rh);

	// Prev
	rx = widthRatio * ui->BUTTON_PLAY_PREV->x();
	ry = heightRatio * ui->BUTTON_PLAY_PREV->y();
	rw = widthRatio * ui->BUTTON_PLAY_PREV->width();
	rh = heightRatio * ui->BUTTON_PLAY_PREV->height();
	ui->BUTTON_PLAY_PREV->setGeometry(rx, ry, rw, rh);

	// Play/Pause
	rx = widthRatio * ui->BUTTON_PLAY_START->x();
	ry = heightRatio * ui->BUTTON_PLAY_START->y();
	rw = widthRatio * ui->BUTTON_PLAY_START->width();
	rh = heightRatio * ui->BUTTON_PLAY_START->height();
	ui->BUTTON_PLAY_START->setGeometry(rx, ry, rw, rh);
	ui->BUTTON_PLAY_PAUSE->setGeometry(rx, ry, rw, rh);

	// Next
	rx = widthRatio * ui->BUTTON_PLAY_NEXT->x();
	ry = heightRatio * ui->BUTTON_PLAY_NEXT->y();
	rw = widthRatio * ui->BUTTON_PLAY_NEXT->width();
	rh = heightRatio * ui->BUTTON_PLAY_NEXT->height();
	ui->BUTTON_PLAY_NEXT->setGeometry(rx, ry, rw, rh);
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

void Frame::slotCommandFromServer(QString command)
{
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
	} else if (!(tokens[1] == "AVK" || tokens[1] == "MGT")) {
		return;
	}

	// valid commands
	if (tokens[2] == "UPDATE MEDIA ELEMENT") {
		updateToUIForMediaElements(tokens);
	} else if (tokens[2] == "UPDATE PLAY POSITION") {
		updateToUIForPlayPosition(tokens);
//	} else if (tokens[2] == "PLAY START") {
//		setUIState(UIState_Playing);
	} else if (tokens[2] == "STREAMING STARTED") {
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
	} else if (tokens[2] == "IS CONNECTED" || tokens[2] == "CONNECTION STATUS") {
		if (tokens.size() >= 4 && tokens[3] == "CONNECTED") {
			emit signalCommandToServer("$AVK#PLAY START\n");
		}
	} else if (tokens[2] == "BSA SERVER KILLED")
	{
		updateForUIReset();
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
			setUIState(UIState_Playing);
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

void Frame::updateForUIReset()
{
	ui->LABEL_MDEDIA_TITLE->clear();
	ui->LABEL_MDEDIA_ETC->clear();
	ui->LABEL_PLAY_POSITION->setText("00:00");
	ui->LABEL_PLAY_DURATION->setText("00:00");
	ui->SLIDER_PLAY_POSITION->setValue(0);
	setUIState(UIState_Stopped);
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
