#include "Dialog.h"
#include "ui_Dialog.h"
#include <QProcess>
#include "TimeUtility.hpp"
#include "LogUtility.hpp"

void Dialog::callbackStatusHomeButtonClicked(void* obj)
{
	// to avoid compiler warning.
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

void Dialog::callbackStatusBackButtonClicked(void* obj)
{
	Dialog* self = (Dialog*)obj;

	self->m_pCommandProcessor->commandToServer("$AVK#PLAY STOP\n");

#ifdef CONFIG_NX_DAUDIO_MANAGER
	self->close();
#else
	exit(0);
#endif
}

Dialog::Dialog(QWidget *parent) :
	QDialog(parent, Qt::FramelessWindowHint),
	ui(new Ui::Dialog)
{
	ui->setupUi(this);

	// settings for NxStatusBar
	m_pNxStatusBar = new CNX_StatusBar(this);
	m_pNxStatusBar->move(0, 0);
	m_pNxStatusBar->resize(width(), height() / 10);
	m_pNxStatusBar->RegOnClickedHome(callbackStatusHomeButtonClicked);
	m_pNxStatusBar->RegOnClickedBack(callbackStatusBackButtonClicked);
	m_pNxStatusBar->SetTitleName("Nexell BT Audio Player");

	m_pCommandProcessor = BTCommandProcessor::instance();
	if (m_pCommandProcessor) {
		m_pCommandProcessor->start();

		connect(m_pCommandProcessor, SIGNAL(signalCommandProcessorEnabled()), this, SLOT(slotCommandProcessorEnabled()));
		connect(m_pCommandProcessor, SIGNAL(signalCommandFromServer(QString)), this, SLOT(slotCommandFromServer(QString)));
		connect(this, SIGNAL(signalCommandToServer(QString)), m_pCommandProcessor, SLOT(slotCommandToServer(QString)));
	}

	setUIState(UIState_Paused);
	setUIState(UIState_BluetoothDisabled);

	m_pCommandProcessor->commandToServer("$AVK#IS CONNECTED\n");

	NX_PacpClientStart(this);

	installEventFilter(this);
}

Dialog::~Dialog()
{
	NX_PacpClientStop();

	delete ui;
}
#ifdef CONFIG_NX_DAUDIO_MANAGER
bool Dialog::eventFilter(QObject *watched, QEvent *event)
{
	if (watched == this && event->type() == NX_QT_CUSTOM_EVENT_TYPE) {
		NxEvent* nx_event = reinterpret_cast<NxEvent*>(event);

		switch (nx_event->m_iEventType) {
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
#endif
void Dialog::slotCommandProcessorEnabled()
{
	m_pCommandProcessor->commandToServer("$AVK#PLAY STATUS\n");
	m_pCommandProcessor->commandToServer("$AVK#PLAY INFO\n");
}

void Dialog::slotCommandFromServer(QString command)
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
		if (updateToUIForBluetoothEnable(tokens)) {
			m_pCommandProcessor->commandToServer("$AVK#PLAY START\n");
		}

	} else if (tokens[2] == "CONNECTION STATUS") {
		updateToUIForBluetoothEnable(tokens);
	}
}

bool Dialog::updateToUIForBluetoothEnable(QStringList &tokens)
{
	if (tokens.size() >= 4) {
		if (tokens[3] == "CONNECTED") {
			setUIState(UIState_BluetoothEnabled);
			return true;
		} else {
			setUIState(UIState_BluetoothDisabled);
			return false;
		}
	}

	return false;
}

void Dialog::updateToUIForMediaElements(QStringList& tokens)
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

void Dialog::updateToUIForPlayPosition(QStringList& tokens)
{
	// example) $OK#AVK#UPDATE PLAY POSITION#12345\n"
	if (tokens.size() == 4) {
		int play_position = tokens[3].toInt();
		std::string s_play_position = convertToRealtimeString(play_position);
		ui->LABEL_PLAY_POSITION->setText(QString::fromStdString(s_play_position));
		ui->SLIDER_PLAY_POSITION->setValue(play_position);
	}
}

void Dialog::updateToUIForPlayStatus(QStringList& tokens)
{
	// example) "$OK#AVK#UPDATE PLAY STATUS#[STATUS]\n"

	if (tokens.size() == 4) {
		if (tokens[3] == "PLAYING") {
			setUIState( UIState_Playing );

			if (isHidden()) {
				if ( NX_REPLY_FAIL == NX_RequestCommand(NX_REQUEST_PROCESS_SHOW)) {
					LOGQ(STR(NX_REQUEST_PROCESS_SHOW) << "[FAILED]");
				}
			}
		} else if (tokens[3] == "PAUSED") {
			setUIState(UIState_Paused);
		} else if (tokens[3] == "STOPPED") {
			setUIState(UIState_Paused);
		}
	}
}

void Dialog::updateToUIForPlayInformation(QStringList& tokens)
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

void Dialog::on_BUTTON_PLAY_PREV_clicked()
{
	m_pCommandProcessor->commandToServer("$AVK#PLAY PREV\n");
}

void Dialog::on_BUTTON_PLAY_NEXT_clicked()
{
	m_pCommandProcessor->commandToServer("$AVK#PLAY NEXT\n");
}

void Dialog::on_BUTTON_PLAY_START_clicked()
{
	m_pCommandProcessor->commandToServer("$AVK#PLAY START\n");
}

void Dialog::on_BUTTON_PLAY_PAUSE_clicked()
{
	m_pCommandProcessor->commandToServer("$AVK#PLAY PAUSE\n");
}

void Dialog::setUIState(UIState state)
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

	case UIState_BluetoothEnabled:
//        ui->LABEL_BLUETOOTH_ENABLE->show();
//        ui->LABEL_BLUETOOTH_DISABLE->hide();
		break;

	case UIState_BluetoothDisabled:
//        ui->LABEL_BLUETOOTH_DISABLE->show();
//        ui->LABEL_BLUETOOTH_ENABLE->hide();
		break;
	}
}
#ifdef CONFIG_NX_DAUDIO_MANAGER
//void Dialog::showEvent(QShowEvent *)
//{
//    if (!isHidden())
//        return;

//    QDialog::show();
//}

//void Dialog::hideEvent(QHideEvent *)
//{
//    if (isHidden())
//        return;

//    QDialog::hide();
//}

void Dialog::closeEvent(QCloseEvent *)
{
	QDialog::close();

	if (NX_REPLY_FAIL == NX_RequestCommand(NX_REQUEST_PROCESS_REMOVE)) {
		printf("@Fail, NX_RequestCommand(). command = NX_REQUEST_PROCESS_REMOVE\n");
	}
}
#endif
