#include "Form.h"
#include "ui_Form.h"

#ifdef CONFIG_TEST2
#	define LOG_TAG "[NxTest2]"
#else
#	define LOG_TAG "[NxTest1]"
#endif
#include <NX_Log.h>

Form* Form::m_spInstance = NULL;

// Message
void (*Form::m_pRequestSendMessage)(const char *pDst, const char *pMsg, int32_t iMsgSize) = NULL;

// Popup Message
void (*Form::m_pRequestPopupMessage)(PopupMessage *, bool *) = NULL;
void (*Form::m_pRequestExpirePopupMessage)() = NULL;

// Notification
void (*Form::m_pRequestNotification)(PopupMessage *) = NULL;
void (*Form::m_pRequestExpireNotification)() = NULL;

// Audio
void (*Form::m_pRequestAudioFocus)(FocusPriority ePriority, bool *bOk) = NULL;
void (*Form::m_pRequestAudioFocusTransient)(FocusPriority ePriority, bool *bOk) = NULL;
void (*Form::m_pRequestAudioFocusLoss)(void) = NULL;

// Video
void (*Form::m_pRequestVideoFocus)(FocusPriority ePriority, bool *bOk) = NULL;
void (*Form::m_pRequestVideoFocusTransient)(FocusPriority ePriority, bool *bOk) = NULL;
void (*Form::m_pRequestVideoFocusLoss)(void) = NULL;

// Terminate
void (*Form::m_pRequestTerminate)(void) = NULL;


Form::Form(QWidget *parent) :
	QFrame(parent),
	ui(new Ui::Form)
{
	ui->setupUi(this);

	move(0, 60);

	// Reset flags
	//  Initialize
	m_bInitialized = false;
	//  Focus
	m_bHasAudioFocus = false;
	m_bHasVideoFocus = false;
}

Form::~Form()
{
	delete ui;
}

bool Form::Initialize()
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
	return true;
}

bool Form::event(QEvent *event)
{
	switch ((int)event->type()) {
	case E_NX_EVENT_TERMINATE:
	{
		NxTerminateEvent *e = static_cast<NxTerminateEvent *>(event);
		TerminateEvent(e);
		return true;
	}

	case QEvent::WindowActivate:
	{
		NXLOGI("[%s] WindowActivate", __FUNCTION__);
		break;
	}

	case QEvent::WindowDeactivate:
	{
		NXLOGI("[%s] WindowDeactivate", __FUNCTION__);
		break;
	}

	default: break;
	}

	return QFrame::event(event);
}

void Form::TerminateEvent(NxTerminateEvent *)
{
	if (m_pRequestTerminate)
		m_pRequestTerminate();
}

Form* Form::GetInstance(void *pObj)
{
	if (!m_spInstance)
		m_spInstance = new Form((QWidget *)pObj);

	return m_spInstance;
}

Form* Form::GetInstance()
{
	return m_spInstance;
}

void Form::DestroyInstance()
{
	if (m_spInstance)
	{
		delete m_spInstance;
		m_spInstance = NULL;
	}
}

// Message
void Form::SendMessage(QString msg)
{
	NXLOGI("[%s] %s", __FUNCTION__, msg.toStdString().c_str());
}

void Form::RegisterRequestSendMessage(void (*cbFunc)(const char *pDst, const char *pMsg, int32_t iMsgSize))
{
	if (cbFunc)
		m_pRequestSendMessage = cbFunc;
}

// Audio Focus
void Form::RequestAudioFocus(FocusType eType, FocusPriority ePriority, bool *bOk)
{
	if (eType == FocusType_Get)
	{
		FocusPriority eCurrPriority;
		if (ui->RADIO_AUDIO_PRIORITY_HIGH->isChecked())
			eCurrPriority = FocusPriority_High;
		else
			eCurrPriority = FocusPriority_Normal;

		if (eCurrPriority > ePriority)
			*bOk = false;
		else
			*bOk = true;

		m_bHasAudioFocus = *bOk ? false : true;

		if (ui->RADIO_TERMINATE->isChecked())
		{
			QApplication::postEvent(this, new NxTerminateEvent());
		}
	}
	else
	{
		m_bHasAudioFocus = true;
	}
}

void Form::RegisterRequestAudioFocus(void (*cbFunc)(FocusPriority ePriority, bool *bOk))
{
	if (cbFunc)
		m_pRequestAudioFocus = cbFunc;
}

void Form::RequestAudioFocusTransient(FocusPriority ePriority, bool *bOk)
{
	FocusPriority eCurrPriority;
	if (ui->RADIO_AUDIO_PRIORITY_HIGH->isChecked())
		eCurrPriority = FocusPriority_High;
	else
		eCurrPriority = FocusPriority_Normal;

	if (eCurrPriority > ePriority)
		*bOk = false;
	else
		*bOk = true;

	m_bHasAudioFocus = *bOk ? false : true;
}

void Form::RegisterRequestAudioFocusTransient(void (*cbFunc)(FocusPriority ePriority, bool *bOk))
{
	if (cbFunc)
		m_pRequestAudioFocusTransient = cbFunc;
}

void Form::RegisterRequestAudioFocusLoss(void (*cbFunc)(void))
{
	if (cbFunc)
		m_pRequestAudioFocusLoss = cbFunc;
}

// Video Focus
void Form::RequestVideoFocus(FocusType eType, FocusPriority ePriority, bool *bOk)
{
	if (eType == FocusType_Get)
	{
		FocusPriority eCurrPriority;
		if (ui->RADIO_VIDEO_PRIORITY_HIGHEST->isChecked())
			eCurrPriority = FocusPriority_Highest;
		else if (ui->RADIO_VIDEO_PRIORITY_HIGH->isChecked())
			eCurrPriority = FocusPriority_High;
		else
			eCurrPriority = FocusPriority_Normal;

		if (eCurrPriority > ePriority)
			*bOk = false;
		else
			*bOk = true;

		m_bHasVideoFocus = *bOk ? false : true;
	}
	else // FocusType_Set
	{
		*bOk = true;
		m_bHasVideoFocus = true;

		if (isHidden())
			show();
		raise();
	}
}

void Form::RegisterRequestVideoFocus(void (*cbFunc)(FocusPriority ePriority, bool *bOk))
{
	if (cbFunc)
		m_pRequestVideoFocus= cbFunc;
}

void Form::RequestVideoFocusTransient(FocusPriority ePriority, bool *bOk)
{
	FocusPriority eCurrPriority;
	if (ui->RADIO_VIDEO_PRIORITY_HIGHEST->isChecked())
		eCurrPriority = FocusPriority_Highest;
	else if (ui->RADIO_VIDEO_PRIORITY_HIGH->isChecked())
		eCurrPriority = FocusPriority_High;
	else
		eCurrPriority = FocusPriority_Normal;

	if (eCurrPriority > ePriority)
		*bOk = false;
	else
		*bOk = true;

	m_bHasVideoFocus = *bOk ? false : true;
}

void Form::RegisterRequestVideoFocusTransient(void (*cbFunc)(FocusPriority ePriority, bool *bOk))
{
	if (cbFunc)
		m_pRequestVideoFocusTransient = cbFunc;
}

void Form::RegisterRequestVideoFocusLoss(void (*cbFunc)(void))
{
	if (cbFunc)
		m_pRequestVideoFocusLoss = cbFunc;
}

void Form::RegisterRequestTerminate(void (*cbFunc)(void))
{
	if (cbFunc)
		m_pRequestTerminate = cbFunc;
}

void Form::RegisterRequestPopupMessage(void (*cbFunc)(PopupMessage *, bool *))
{
	if (cbFunc)
		m_pRequestPopupMessage = cbFunc;
}

void Form::RegisterRequestExpirePopupMessage(void (*cbFunc)())
{
	if (cbFunc)
		m_pRequestExpirePopupMessage = cbFunc;
}

void Form::PopupMessageResponse(bool bOk)
{
	NXLOGI("[%s] <%s>", __FUNCTION__, bOk ? "ACCEPT" : "REJECT");
}

void Form::RegisterRequestNotification(void (*cbFunc)(PopupMessage *))
{
	if (cbFunc)
		m_pRequestNotification = cbFunc;
}

void Form::RegisterRequestExpireNotification(void (*cbFunc)())
{
	if (cbFunc)
		m_pRequestExpireNotification = cbFunc;
}

void Form::NotificationResponse(bool bOk)
{
	NXLOGI("[%s] <%s>", __FUNCTION__, bOk ? "ACCEPT" : "REJECT");
}

void Form::on_BUTTON_CHECK_FOCUS_clicked()
{
	NXLOGI("[%s] AudioFocus<%s> VideoFocus<%s>", __FUNCTION__, m_bHasAudioFocus ? "O" : "X", m_bHasVideoFocus ? "O" : "X");
}

void Form::on_BUTTON_AUDIOFOCUS_clicked()
{
	if (m_pRequestAudioFocus)
	{
		bool bOk = false;
		FocusPriority ePriority = ui->RADIO_AUDIO_PRIORITY_HIGH->isChecked() ? FocusPriority_High : FocusPriority_Normal;
		NXLOGI("[%s] <%s> REQUEST AUDIO FOCUS", __FUNCTION__, "TRY");
		m_pRequestAudioFocus(ePriority, &bOk);
		m_bHasAudioFocus = bOk;
		NXLOGI("[%s] <%s> REQUEST AUDIO FOCUS", __FUNCTION__, bOk ? "OK" : "NG");
	}
}

void Form::on_BUTTON_AUDIOFOCUS_TRANSIENT_clicked()
{
	if (m_pRequestAudioFocusTransient)
	{
		bool bOk = false;
		FocusPriority ePriority = ui->RADIO_AUDIO_PRIORITY_HIGH->isChecked() ? FocusPriority_High : FocusPriority_Normal;
		NXLOGI("[%s] <%s> REQUEST AUDIO FOCUS TRANSIENT", __FUNCTION__, "TRY");
		m_pRequestAudioFocusTransient(ePriority, &bOk);
		m_bHasAudioFocus = bOk;
		NXLOGI("[%s] <%s> REQUEST AUDIO FOCUS TRANSIENT", __FUNCTION__, bOk ? "OK" : "NG");
	}
}

void Form::on_BUTTON_AUDIOFOCUS_LOSS_clicked()
{
	if (m_pRequestAudioFocusLoss)
	{
		NXLOGI("[%s] <%s> REQUEST AUDIO FOCUS LOSS", __FUNCTION__, "TRY");
		m_pRequestAudioFocusLoss();
		m_bHasAudioFocus = false;
	}
}

void Form::on_BUTTON_VIDEOFOCUS_clicked()
{
	if (m_pRequestAudioFocus)
	{
		bool bOk = false;
		FocusPriority ePriority = ui->RADIO_VIDEO_PRIORITY_HIGHEST->isChecked() ? FocusPriority_Highest : ui->RADIO_VIDEO_PRIORITY_HIGH->isChecked() ? FocusPriority_High : FocusPriority_Normal;
		NXLOGI("[%s] <%s> REQUEST VIDEO FOCUS", __FUNCTION__, "TRY");
		m_pRequestAudioFocus(ePriority, &bOk);
		m_bHasVideoFocus = bOk;
		NXLOGI("[%s] <%s> REQUEST VIDEO FOCUS", __FUNCTION__, bOk ? "OK" : "NG");
	}
}

void Form::on_BUTTON_VIDEOFOCUS_TRANSIENT_clicked()
{
	if (m_pRequestVideoFocusTransient)
	{
		bool bOk = false;
		FocusPriority ePriority = ui->RADIO_VIDEO_PRIORITY_HIGHEST->isChecked() ? FocusPriority_Highest : ui->RADIO_VIDEO_PRIORITY_HIGH->isChecked() ? FocusPriority_High : FocusPriority_Normal;

		NXLOGI("[%s] <%s> REQUEST VIDEO FOCUS TRANSIENT", __FUNCTION__, "TRY");
		m_pRequestVideoFocusTransient(ePriority, &bOk);
		m_bHasVideoFocus = bOk;
		NXLOGI("[%s] <%s> REQUEST VIDEO FOCUS TRANSIENT", __FUNCTION__, bOk ? "OK" : "NG");
	}
}

void Form::on_BUTTON_VIDEOFOCUS_LOSS_clicked()
{
	if (m_pRequestVideoFocusLoss)
	{
		NXLOGI("[%s] <%s> REQUEST VIDEO FOCUS LOSS", __FUNCTION__, "TRY");
		m_pRequestVideoFocusLoss();
		m_bHasVideoFocus = false;
	}
}

void Form::on_BUTTON_SEND_MESSAGE_clicked()
{
	const char* pMsgList[] = {
		"Hello, world!\0",
		"QWERTY\0",
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ\0",
		"abcdefghijklmnopqrstuvwxyz\0",
		"1234567890\0",
		"SDFGADFGERXCVCX\0",
		"!@#@#@!#$$%$%^&^&**(*)%%$%^#@%@%\0"
	};

	if (m_pRequestSendMessage)
	{
		int index = qrand() % sizeof(pMsgList);
#ifdef CONFIG_TEST2
		m_pRequestSendMessage("NxTest1", pMsgList[index]);
#else
		m_pRequestSendMessage("NxTest2", pMsgList[index], strlen(pMsgList[index]));
#endif
	}
}

void Form::on_BUTTON_POPUP_MESSAGE_clicked()
{
	PopupMessage sData;
	QString title = "<b>Bluetooth pairing request<b/>";
	QString body = QString("<p align=\"left\">Please check your authorization number to connect with the '%1' device.<br>Device address : %2<br><p align=\"center\"><font size=\"12\" color=\"blue\">%3</font></p>").arg("han  iphone").arg("28:5A:EB:78:2A:63").arg(300608);
	bool bOk = false;

#if 1
	sData.pMsgTitle = new char[title.length()+1];
	sData.pMsgBody = new char[body.length()+1];

	strcpy(sData.pMsgTitle, title.toStdString().c_str());
	strcpy(sData.pMsgBody, body.toStdString().c_str());
#else
	sData.pMsgTitle = title.toStdString().c_str();
	sData.pMsgBody = body.toStdString().c_str();
#endif

	sData.eVisibility = ButtonVisibility_Default;
	sData.uiTimeout = ui->LABEL_TIMEOUT->text().toInt() * 1000;

	if (m_pRequestPopupMessage)
		m_pRequestPopupMessage(&sData, &bOk);

	delete[] sData.pMsgTitle;
	delete[] sData.pMsgBody;
}

void Form::on_BUTTON_EXPIRE_POPUP_MESSAGE_clicked()
{
	if (m_pRequestExpirePopupMessage)
	{
		m_pRequestExpirePopupMessage();
	}
}

void Form::on_BUTTON_NOTIFICATION_clicked()
{
	NXLOGI("[%s] <%s> ", __FUNCTION__, "TRY");
	PopupMessage sData;
	QString body = QString("<p><b>[Bluetooth pairing request] </b>'%1'(%2) PIN CODE: <font color=\"blue\">%3</font></p>").arg("han  iphone").arg("28:5A:EB:78:2A:63").arg(300608);

	sData.pMsgBody = new char[body.length()+1];
	strcpy(sData.pMsgBody, body.toStdString().c_str());
	sData.eVisibility = ButtonVisibility_Default;
	if (ui->RADIO_ONLY_OK->isChecked())
		sData.eVisibility = ButtonVisibility_Ok;
	else if (ui->RADIO_ONLY_CANCEL->isChecked())
		sData.eVisibility = ButtonVisibility_Cencel;
	else
		sData.eVisibility = ButtonVisibility_Default;
	sData.uiTimeout = ui->LABEL_TIMEOUT->text().toInt() * 1000;

	if (m_pRequestNotification)
		m_pRequestNotification(&sData);

	delete[] sData.pMsgBody;
}

void Form::on_BUTTON_EXPIRE_NOTIFICATION_clicked()
{
	if (m_pRequestExpireNotification)
	{
		m_pRequestExpireNotification();
	}
}

void Form::MediaEventChanged(NxMediaEvent eEvent)
{
	switch (eEvent) {
	case NX_EVENT_MEDIA_SDCARD_INSERT:
		NXLOGI("[%s] NX_EVENT_MEDIA_SDCARD_INSERT", __FUNCTION__);
		break;

	case NX_EVENT_MEDIA_SDCARD_REMOVE:
		NXLOGI("[%s] NX_EVENT_MEDIA_SDCARD_REMOVE", __FUNCTION__);
		break;

	case NX_EVENT_MEDIA_USB_INSERT:
		NXLOGI("[%s] NX_EVENT_MEDIA_USB_INSERT", __FUNCTION__);
		break;

	case NX_EVENT_MEDIA_USB_REMOVE:
		NXLOGI("[%s] NX_EVENT_MEDIA_USB_REMOVE", __FUNCTION__);
		break;

	case NX_EVENT_MEDIA_SCAN_DONE:
		NXLOGI("[%s] NX_EVENT_MEDIA_SCAN_DONE", __FUNCTION__);
		break;

	default:
		NXLOGI("[%s] NX_EVENT_MEDIA_UNKNOWN", __FUNCTION__);
		break;
	}
}

void Form::BackButtonClicked()
{
	QApplication::postEvent(this, new NxTerminateEvent());
}

void Form::on_BUTTON_MINUS_clicked()
{
	int value = ui->LABEL_TIMEOUT->text().toInt();
	if (value > 0)
	{
		ui->LABEL_TIMEOUT->setText(QString("%1").arg(--value));
	}
}

void Form::on_BUTTON_PLUS_clicked()
{
	int value = ui->LABEL_TIMEOUT->text().toInt();
	if (value < 10)
	{
		ui->LABEL_TIMEOUT->setText(QString("%1").arg(++value));
	}
}
