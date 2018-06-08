#include "AdvancedMenuWidget.h"
#include "ui_AdvancedMenuWidget.h"

AdvancedMenuWidget::AdvancedMenuWidget(QWidget *parent) :
    QWidget(parent, Qt::FramelessWindowHint),
    ui(new Ui::AdvancedMenuWidget)
{
    ui->setupUi(this);

    setUIState(UIState_Initializing);

    m_pCommandProcessor = BTCommandProcessor::instance();
    if (m_pCommandProcessor) {
        connect(m_pCommandProcessor, SIGNAL(signalCommandFromServer(QString)), this, SLOT(slotCommandFromServer(QString)));
        connect(this, SIGNAL(signalCommandToServer(QString)), m_pCommandProcessor, SLOT(slotCommandToServer(QString)));
    }


}

AdvancedMenuWidget::~AdvancedMenuWidget()
{
    delete ui;
}

void AdvancedMenuWidget::setUIState(UIState state)
{
    m_UIState = state;
}

void AdvancedMenuWidget::setUIStateForAutoSettings(AutoSettingsList type, AutoSettingsState state)
{
    switch (type) {
    case AutoSettingsList_AutoConnection:
        m_AutoConnectionState = state;

        switch (state) {
        case AutoSettingsState_ON:
            ui->BUTTON_AUTO_CONNECTION_ON->show();
            ui->BUTTON_AUTO_CONNECTION_OFF->hide();
            break;

        case AutoSettingsState_OFF:
            ui->BUTTON_AUTO_CONNECTION_OFF->show();
            ui->BUTTON_AUTO_CONNECTION_ON->hide();
            break;
        }
        break;

    case AutoSettingsList_AutoPairing:
        m_AutoPairingState = state;

        switch (state) {
        case AutoSettingsState_ON:
            ui->BUTTON_AUTO_PAIRING_ON->show();
            ui->BUTTON_AUTO_PAIRING_OFF->hide();
            break;

        case AutoSettingsState_OFF:
            ui->BUTTON_AUTO_PAIRING_OFF->show();
            ui->BUTTON_AUTO_PAIRING_ON->hide();
            break;
        }
        break;
    }
}

void AdvancedMenuWidget::slotCommandFromServer(QString command)
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
    } else if (tokens[1] != "MGT") {
        return;
    }

    // valid commands
	if (tokens[2] == "PING" && m_UIState == UIState_Initializing) {
		QStringList commands;
		// 1. initialize
		// 1.1. auto connection state
		commands.append("$MGT#AUTO CONNECTION\n");
		// 1.2. auto pairing state
		commands.append("$MGT#AUTO PAIRING\n");

		m_pCommandProcessor->commandListToServer(commands);

		setUIState(UIState_Initialized);
	} else if (tokens[2] == "AUTO CONNECTION") {
        updateToUIForAutoConnection(tokens);
    } else if (tokens[2].startsWith("ENABLE AUTO CONNECTION")) {
        updateToUIForAutoConnection(tokens);
		sync();
    } else if (tokens[2] == "AUTO PAIRING") {
        updateToUIForAutoPairing(tokens);
    } else if (tokens[2].startsWith("ENABLE AUTO PAIRING")) {
        updateToUIForAutoPairing(tokens);
		sync();
    }
}

void AdvancedMenuWidget::updateToUIForAutoConnection(QStringList& tokens)
{
    if (tokens.size() == 4 && tokens[2].startsWith("AUTO")) {
        if (tokens[3] == "ON")
            setUIStateForAutoSettings(AutoSettingsList_AutoConnection, AutoSettingsState_ON);
        else if (tokens[3] == "OFF")
            setUIStateForAutoSettings(AutoSettingsList_AutoConnection, AutoSettingsState_OFF);

    } else if (tokens.size() == 3 && tokens[2].startsWith("ENABLE")) {
        if (tokens[2].right(2) == "ON")
            setUIStateForAutoSettings(AutoSettingsList_AutoConnection, AutoSettingsState_ON);
        else if (tokens[2].right(3) == "OFF")
            setUIStateForAutoSettings(AutoSettingsList_AutoConnection, AutoSettingsState_OFF);
    }
}

void AdvancedMenuWidget::updateToUIForAutoPairing(QStringList& tokens)
{
    if (tokens.size() == 4 && tokens[2].startsWith("AUTO")) {
        if (tokens[3] == "ON")
            setUIStateForAutoSettings(AutoSettingsList_AutoPairing, AutoSettingsState_ON);
        else if (tokens[3] == "OFF")
            setUIStateForAutoSettings(AutoSettingsList_AutoPairing, AutoSettingsState_OFF);

    } else if (tokens.size() == 3 && tokens[2].startsWith("ENABLE")) {
        if (tokens[2].right(2) == "ON")
            setUIStateForAutoSettings(AutoSettingsList_AutoPairing, AutoSettingsState_ON);
        else if (tokens[2].right(3) == "OFF")
            setUIStateForAutoSettings(AutoSettingsList_AutoPairing, AutoSettingsState_OFF);
    }
}

void AdvancedMenuWidget::on_BUTTON_AUTO_CONNECTION_OFF_clicked()
{
    m_pCommandProcessor->commandToServer("$MGT#ENABLE AUTO CONNECTION ON\n");
}

void AdvancedMenuWidget::on_BUTTON_AUTO_CONNECTION_ON_clicked()
{
    m_pCommandProcessor->commandToServer("$MGT#ENABLE AUTO CONNECTION OFF\n");
}

void AdvancedMenuWidget::on_BUTTON_AUTO_PAIRING_OFF_clicked()
{
    m_pCommandProcessor->commandToServer("$MGT#ENABLE AUTO PAIRING ON\n");
}

void AdvancedMenuWidget::on_BUTTON_AUTO_PAIRING_ON_clicked()
{
    m_pCommandProcessor->commandToServer("$MGT#ENABLE AUTO PAIRING OFF\n");
}
