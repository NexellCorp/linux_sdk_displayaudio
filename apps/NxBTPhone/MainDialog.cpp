#include "MainDialog.h"
#include "ui_MainDialog.h"

#define MAX_RETRY_COMMAND_COUNT 3

//pthread_cond_t g_pthread_condition;
//pthread_mutex_t g_pthread_mutex;

MainDialog::MainDialog(QWidget *parent) :
	QDialog(parent, Qt::FramelessWindowHint),
	ui(new Ui::MainDialog)
{
	ui->setupUi(this);

//    m_bProtectForPlayStatus = false;
	m_bDisconnectedCall = false;
	m_nRetryCommandCount = 0;


	m_pCallingDialog = new InCommingCallDialog(this);
	connect(m_pCallingDialog, SIGNAL(signalCommandToServer(QString)), m_pBTCommandProcessor, SLOT(slotCommandToServer(QString)));
	connect(m_pBTCommandProcessor, SIGNAL(signalCommandFromServer(QString)), m_pCallingDialog, SLOT(slotCommandFromServer(QString)));


}

MainDialog::~MainDialog()
{
	delete ui;
}


