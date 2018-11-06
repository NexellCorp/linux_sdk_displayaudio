#include "CallMenuWidget.h"
#include "ui_CallMenuWidget.h"

// custom widgets
#include <gui/phonebook/PhoneBookItem.h>
#include <gui/CallLog/CallLogItem.h>

#include <QDebug>

CallMenuWidget::CallMenuWidget(QWidget *parent) :
    QWidget(parent, Qt::FramelessWindowHint),
    ui(new Ui::CallMenuWidget)
{
    ui->setupUi(this);

    // command response timer
    connect(&m_ResponseTimer, SIGNAL(timeout()), this, SLOT(slotCommandResponseTimer()));

	qRegisterMetaType<vector<CallLogInfo>>("vector<CallLogInfo>");
	connect(&m_UpdateCallLogThread, SIGNAL(signalAdd(vector<CallLogInfo>)), this, SLOT(slotAdd(vector<CallLogInfo>)));
	connect(&m_UpdateCallLogThread, SIGNAL(signalCompleted()), this, SLOT(slotCompleted()));

    // gif
    m_pAnimation = new QMovie("://loading/loading3_100x100.gif");
    ui->LABEL_LOADING->setMovie(m_pAnimation);

    setCurrentMenu(CurrentMenu_Keypad);
    setUIState(UIState_DownloadCompleted);
    setUIState(UIState_BluetoothDisabled);
}

CallMenuWidget::~CallMenuWidget()
{
    delete ui;
}

void CallMenuWidget::Initialize()
{
	commandToServer("$HS#IS CONNECTED\n");
}

void CallMenuWidget::setCurrentMenu(CurrentMenu menu)
{
    m_CurrentMenu = menu;

    switch (menu) {
    case CurrentMenu_PhoneBook:
        ui->WIDGET_PHONEBOOK->show();
        ui->WIDGET_KEYPAD->hide();
        ui->WIDGET_LOG->hide();
        break;

    case CurrentMenu_Keypad:
        ui->WIDGET_PHONEBOOK->hide();
        ui->WIDGET_KEYPAD->show();
        ui->WIDGET_LOG->hide();
        break;

    case CurrentMenu_Log:
        ui->WIDGET_PHONEBOOK->hide();
        ui->WIDGET_KEYPAD->hide();
        ui->WIDGET_LOG->show();
        break;

    case CurrentMenu_Calling:
        break;
    }
}

void CallMenuWidget::setUIState(UIState state)
{
    m_UIState = state;

    switch (state) {
    case UIState_DownloadCompleted:
        ui->BUTTON_SYNC->show();
        ui->LABEL_LOADING->hide();
        m_pAnimation->stop();
        break;

    case UIState_Downloading:
        m_pAnimation->start();
        ui->LABEL_LOADING->show();
        ui->BUTTON_SYNC->hide();
        break;

    case UIState_BluetoothEnabled:
        ui->BUTTON_SYNC->setEnabled(true);
        break;

    case UIState_BluetoothDisabled:
        ui->BUTTON_SYNC->setEnabled(false);
        break;
    }

}

void CallMenuWidget::inputKey(int key)
{
    QKeySequence k(key);
    QString text = k.toString();
    Qt::KeyboardModifier modifier = Qt::NoModifier;

    QApplication::sendEvent(ui->EDIT_INPUT, new QKeyEvent(QEvent::KeyPress, key, modifier, text));
    QApplication::sendEvent(ui->EDIT_INPUT, new QKeyEvent(QEvent::KeyRelease, key, modifier, text));
}

void CallMenuWidget::on_BUTTON_PHONEBOOK_MENU_clicked()
{
    setCurrentMenu(CurrentMenu_PhoneBook);
}

void CallMenuWidget::on_BUTTON_KEYPAD_MENU_clicked()
{
    setCurrentMenu(CurrentMenu_Keypad);
}

void CallMenuWidget::on_BUTTON_LOG_MENU_clicked()
{
    setCurrentMenu(CurrentMenu_Log);
}

void CallMenuWidget::on_BUTTON_KEY_1_clicked()
{
    inputKey(Qt::Key_1);
}

void CallMenuWidget::on_BUTTON_KEY_2_clicked()
{
    inputKey(Qt::Key_2);
}

void CallMenuWidget::on_BUTTON_KEY_3_clicked()
{
    inputKey(Qt::Key_3);
}

void CallMenuWidget::on_BUTTON_KEY_4_clicked()
{
    inputKey(Qt::Key_4);
}

void CallMenuWidget::on_BUTTON_KEY_5_clicked()
{
    inputKey(Qt::Key_5);
}

void CallMenuWidget::on_BUTTON_KEY_6_clicked()
{
    inputKey(Qt::Key_6);
}

void CallMenuWidget::on_BUTTON_KEY_7_clicked()
{
    inputKey(Qt::Key_7);
}

void CallMenuWidget::on_BUTTON_KEY_8_clicked()
{
    inputKey(Qt::Key_8);
}

void CallMenuWidget::on_BUTTON_KEY_9_clicked()
{
    inputKey(Qt::Key_9);
}

void CallMenuWidget::on_BUTTON_KEY_0_clicked()
{
    inputKey(Qt::Key_0);
}

void CallMenuWidget::on_BUTTON_KEY_BACKSPACE_clicked()
{
    inputKey(Qt::Key_Backspace);
}

void CallMenuWidget::on_BUTTON_DIAL_clicked()
{
    //
    commandToServer("$HS#DIAL " + ui->EDIT_INPUT->text() + "\n");
}

void CallMenuWidget::on_BUTTON_RE_DIAL_clicked()
{
    commandToServer("$HS#REDIAL\n");
}

void CallMenuWidget::slotCommandFromServer(QString command)
{
    int stx = command.indexOf("$");
    int etx = command.indexOf("\n");

    if (stx < 0 || etx < 0) {
        return;
    }

    // body = remove STX and ETX from command
    stx++;
	QString body = command.mid(stx, etx-stx);
//	qDebug() << Q_FUNC_INFO << body;

    // example) command = "$OK#HS#CALL STATUS#INCOMMING CALL\n"
    // example) body    = "OK#HS#CALL STATUS#INCOMMING CALL"

    QStringList tokens = body.split("#");
    QString temp; // output origin command from command(return)

    // failure conditions
    if (tokens.size() < 3) {
        return;
    } else if (tokens[0] == "NG") {
        return;
    } else if (!(tokens[1] == "HS" || tokens[1] == "PBC")) {
        return;
    }

    temp = "$" + tokens[1] + "#" + tokens[2] + "\n";

    for (int i = 0; i < m_CommandQueue.count(); i++) {
        // 1. find send command
        if (m_CommandQueue.at(i).first == temp) {
            // 2. check return(receive) command
            if (command.indexOf(m_CommandQueue.at(i).second) >= 0) {
                m_CommandQueue.removeAt(i);
                if (m_ResponseTimer.isActive())
                    m_ResponseTimer.stop();
                break;
            }
        }
    }

    // valid commands
    if (tokens[1] == "HS") {
		if (tokens[2] == "CONNECTION STATUS") {
			updateForBluetoothEnable(tokens);
		} else if (tokens[2] == "IS CONNECTED") {
			updateForBluetoothEnable(tokens);
		}
    } else { // PBC
        // example) "$OK#PBC#DOWNLAOD PHONEBOOK#FILE CREATED#/etc/bluetooth/pb_data.vcf"
        if (tokens[2] == "DOWNLOAD PHONEBOOK") {
            updateForPhoneBook(tokens);
        } else if (tokens[2] == "DOWNLOAD CALL LOG") {
            updateForCallLog(tokens);
        }
    }
}

void CallMenuWidget::on_BUTTON_SYNC_clicked()
{
    switch (m_CurrentMenu) {
    case CurrentMenu_PhoneBook:
        setUIState(UIState_Downloading);
        commandToServer("$PBC#DOWNLOAD PHONEBOOK\n", "$OK#PBC#DOWNLOAD PHONEBOOK#FILE CREATED");
        break;

    case CurrentMenu_Log:
        setUIState(UIState_Downloading);
		clearCallLog();
        commandToServer("$PBC#DOWNLOAD CALL LOG\n", "$OK#PBC#DOWNLOAD CALL LOG#FILE CREATED");
        break;

    default:
        break;
    }
}

bool CallMenuWidget::updateForBluetoothEnable(QStringList& tokens)
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

bool CallMenuWidget::updateForPhoneBook(QStringList &tokens)
{
    // example) "$OK#PBC#DOWNLAOD PHONEBOOK#FILE CREATED#/etc/bluetooth/pb_data.vcf"
    if (tokens.size() == 5) {
        if (tokens[3] == "FILE CREATED") {
            ui->LISTWIDGET_PHONEBOOK->clear();

            VCardReader reader;
            if (reader.read(tokens[4].toStdString())) {
                m_PhoneBook = reader.properties();

                for (size_t i = 0; i < m_PhoneBook.size(); i++) {
                    ui->LISTWIDGET_PHONEBOOK->addItem(m_PhoneBook[i].FN.c_str());
                }
            }

            setUIState(UIState_DownloadCompleted);
            return true;
        }
    }

    return false;
}

bool CallMenuWidget::updateForCallLog(QStringList &tokens)
{
    // example) "$OK#PBC#DOWNLAOD CALL LOG#FILE CREATED#/etc/bluetooth/pb_data.vcf"
	if (tokens.size() == 5) {
		if (tokens[3] == "FILE CREATED") {
#if 0
            VCardReader reader;
            if (reader.read(tokens[4].toStdString())) {
                m_CallLog = reader.properties();

                QListWidgetItem* item = NULL;
                CallLogItem* custom = NULL;
                QString direction;
                QString owner;
                QString type;

                for (size_t i = 0; i < m_CallLog.size(); i++) {

                    if (m_CallLog[i].TEL.size() == 0) {
                        continue;
                    }

                    switch (m_CallLog[i].X_IRMC_CALL_DATETIME.type) {
                    case VCardReader::CallType_Dialed:
                        direction = "S";
                        break;

                    case VCardReader::CallType_Receivced:
                        direction = "R";
                        break;

                    case VCardReader::CallType_Missed:
                        direction = "M";
                        break;

                    default: // assume call log empty!
                        continue;
                    }

                    if (m_CallLog[i].FN.empty()) {
                        owner = QString::fromStdString(m_CallLog[i].TEL[0].number);
                    } else {
                        owner = QString::fromStdString(m_CallLog[i].FN);
                    }

                    switch (m_CallLog[i].TEL[0].type) {
                    case VCardReader::TelephoneType_Voice:
                        type = tr("Voice");
                        break;

                    case VCardReader::TelephoneType_Home:
                        type = tr("Home");
                        break;

                    case VCardReader::TelephoneType_Cell:
                        type = tr("Cell");
                        break;

                    case VCardReader::TelephoneType_Work:
                        type = tr("Work");
                        break;

                    case VCardReader::TelephoneType_Tel:
                        type = tr("Tel");
                        break;

                    case VCardReader::TelephoneType_Etc:
                        type = tr("Etc");
                        break;

                    default:
                        type = tr("Unknown");
                        break;
                    }

                    item = new QListWidgetItem;
                    item->setSizeHint(QSize(ui->LISTWIDGET_CALL_LOG->width()-50, 100));
                    custom = new CallLogItem;
                    custom->setCallDirection(direction);
                    custom->setCallNumberOwner(owner);
                    custom->setCallNumberType(type);

					qDebug() << 0 << tm.elapsed();
                    ui->LISTWIDGET_CALL_LOG->addItem(item);
					qDebug() << 1 << tm.elapsed();
					ui->LISTWIDGET_CALL_LOG->setItemWidget(item, custom);
					qDebug() << 2 << tm.elapsed();

                }
            }


			setUIState(UIState_DownloadCompleted);
			qDebug() << 3 << tm.elapsed() << ui->LISTWIDGET_CALL_LOG->count();
#else
			m_UpdateCallLogThread.SetFile(tokens[4]);
			m_UpdateCallLogThread.Start();
#endif
            return true;
        }
    }

    return false;
}

void CallMenuWidget::clearCallLog()
{
	// clear list widget
	QListWidgetItem *item = NULL;
	CallLogItem *custom = NULL;

	for (int i = 0; i < ui->LISTWIDGET_CALL_LOG->count(); i++) {
		item = ui->LISTWIDGET_CALL_LOG->item(i);
		if (item) {
			custom = (CallLogItem*)ui->LISTWIDGET_CALL_LOG->itemWidget(item);
			if (custom)
				delete custom;

			delete item;
		}
	}

	ui->LISTWIDGET_CALL_LOG->clear();
}

void CallMenuWidget::slotAdd(vector<CallLogInfo> sInfoList)
{
	QListWidgetItem *item = NULL;
	CallLogItem *custom = NULL;

	for (size_t i = 0; i < sInfoList.size(); ++i)
	{
		item = new QListWidgetItem;
		item->setSizeHint(QSize(ui->LISTWIDGET_CALL_LOG->width()-50, 100));
		custom = new CallLogItem;
		custom->setCallDirection(sInfoList[i].direction);
		custom->setCallNumberOwner(sInfoList[i].owner);
		custom->setCallNumberType(sInfoList[i].type);

		ui->LISTWIDGET_CALL_LOG->addItem(item);
		ui->LISTWIDGET_CALL_LOG->setItemWidget(item, custom);
	}
}

void CallMenuWidget::slotCompleted()
{
	setUIState(UIState_DownloadCompleted);
}

void CallMenuWidget::on_LISTWIDGET_PHONEBOOK_currentRowChanged(int currentRow)
{
    // check range
    if (currentRow < 0 || (int)m_PhoneBook.size() <= currentRow)
        return;

    QListWidgetItem* item = NULL;
    PhoneBookItem* custom = NULL;

    // clear list widget
    for (int i = 0; i < ui->LISTWIDGET_PHONEBOOK_DETAIL->count(); i++) {
        item = ui->LISTWIDGET_PHONEBOOK_DETAIL->item(i);
        if (item) {
            custom = (PhoneBookItem*)ui->LISTWIDGET_PHONEBOOK_DETAIL->itemWidget(item);
            if (custom)
                delete custom;

            delete item;
        }
    }

    ui->LISTWIDGET_PHONEBOOK_DETAIL->clear();

    // phone number
    if (m_PhoneBook[currentRow].TEL.size()) {
        for (size_t i = 0; i < m_PhoneBook[currentRow].TEL.size(); i++) {
            item = new QListWidgetItem();
            item->setSizeHint(QSize(ui->LISTWIDGET_PHONEBOOK_DETAIL->width()-50, 100));
            custom = new PhoneBookItem(ui->LISTWIDGET_PHONEBOOK_DETAIL->width()-50, 100);

            switch (m_PhoneBook[currentRow].TEL[i].type) {
            case VCardReader::TelephoneType_Voice:
                custom->setTitle(tr("Voice"));
                break;

            case VCardReader::TelephoneType_Cell:
                custom->setTitle(tr("Cell"));
                break;

            case VCardReader::TelephoneType_Home:
                custom->setTitle(tr("Home"));
                break;

            case VCardReader::TelephoneType_Work:
                custom->setTitle(tr("Work"));
                break;

            case VCardReader::TelephoneType_Tel:
                custom->setTitle(tr("Tel"));
                break;

            case VCardReader::TelephoneType_Etc:
                custom->setTitle(tr("Etc"));
                break;

            default:
                break;
            }

            custom->setData(m_PhoneBook[currentRow].TEL[i].number.c_str());

            ui->LISTWIDGET_PHONEBOOK_DETAIL->addItem(item);
            ui->LISTWIDGET_PHONEBOOK_DETAIL->setItemWidget(item, custom);
        }
    }

    // e-mail
    if (m_PhoneBook[currentRow].EMAIL.size()) {
        item = new QListWidgetItem();        
        item->setSizeHint(QSize(ui->LISTWIDGET_PHONEBOOK_DETAIL->width()-50, 100));
        custom = new PhoneBookItem(ui->LISTWIDGET_PHONEBOOK_DETAIL->width()-50, 100);

        custom->setTitle(tr("E-Mail"));
        custom->setData(m_PhoneBook[currentRow].EMAIL[0].address.c_str());

        ui->LISTWIDGET_PHONEBOOK_DETAIL->addItem(item);
        ui->LISTWIDGET_PHONEBOOK_DETAIL->setItemWidget(item, custom);
    }
}

void CallMenuWidget::on_LISTWIDGET_PHONEBOOK_DETAIL_itemClicked(QListWidgetItem *item)
{
    if (item) {
        PhoneBookItem* custom = (PhoneBookItem*)ui->LISTWIDGET_PHONEBOOK_DETAIL->itemWidget(item);

        if (custom->title() != tr("E-Mail")) {
            QString phonenumber = custom->data().replace("-", "");
            commandToServer("$HS#DIAL " + phonenumber + "\n");
        }
    }
}

//void CallMenuDialog::on_LISTWIDGET_CALL_LOG_itemClicked(QListWidgetItem *item)
//{
//    if (item) {
//        int row = ui->LISTWIDGET_CALL_LOG->row(item);
//        if (m_CallLog[row].TEL.size()) {
//            QString phonenumber = QString::fromStdString(m_CallLog[row].TEL[0].number).replace("-", "");
//            commandToServer("$HS#DIAL " + phonenumber + "\n");
//        }
//    }
//}

void CallMenuWidget::commandToServer(QString command, QString return_command)
{
    if (return_command.isEmpty()) {
        return_command = command.mid(1, command.length()-2); // for "$", "\n" remove
    }

    // 1. make pair
    //  1. first QString  : command for send
    //  2. second QString : command for valid return(receive) command
    QPair<QString, QString> command_set;
    command_set.first = command;
    command_set.second = return_command;

    m_CommandQueue << command_set;
    if (m_ResponseTimer.isActive())
        m_ResponseTimer.stop();
    m_ResponseTimer.start(MAX_RESPONSE_TIME);

	emit signalCommandToServer(command);
}

void CallMenuWidget::slotCommandResponseTimer()
{
    if (m_CommandQueue.count()) {
        m_ResponseTimer.stop();

        // restore ui state
        if (m_UIState == UIState_Downloading)
            setUIState(UIState_DownloadCompleted);
    }
}

void CallMenuWidget::on_LISTWIDGET_CALL_LOG_itemDoubleClicked(QListWidgetItem *item)
{
    if (item) {
        int row = ui->LISTWIDGET_CALL_LOG->row(item);
        if (m_CallLog[row].TEL.size()) {
            QString phonenumber = QString::fromStdString(m_CallLog[row].TEL[0].number).replace("-", "");
            commandToServer("$HS#DIAL " + phonenumber + "\n");
        }
    }
}
