#include "NxLauncher.h"
#include "ui_NxLauncher.h"

#ifndef CONFIG_USE_NO_QML
#include <QQuickView>
#include <QQuickItem>
#include <QQmlProperty>
#include <QQmlContext>
#include <QQuickWidget>
#endif
#include <QObject>
#include <QWidget>
#include <QProcess>
#include <QDirIterator>
#include <QDesktopWidget>

#include <execinfo.h>
#include <signal.h>

#include <DAudioKeyDef.h>
#include "NX_DAudioUtils.h"

#ifdef Q_PROCESSOR_X86
#define NX_APP_PATH	"/home/daegeun/workspace/daudio-5.6/displayaudio/result/nexell/daudio"
#else
#define NX_APP_PATH "/nexell/daudio"
#endif

// for dlopen
#include <dlfcn.h>

#define NX_LAUNCHER "NxLauncher"
#define NX_LAUNCHER_P "NxLauncherP" // for Popup

#define LOG_TAG "[NxLauncher]"
#include <NX_Log.h>

#define TEST_COMMAND_PATH "/home/root/cmd"

#define DEFAULT_WIDTH	1024
#define DEFAULT_HEIGHT	600

NxLauncher* NxLauncher::m_spInstance = NULL;
QQueue<QString> NxLauncher::m_AudioFocusQueue = QQueue<QString>();
QQueue<QString> NxLauncher::m_VideoFocusQueue = QQueue<QString>();

static void signal_handler(int sig)
{
	NXLOGI("[%s] %d", __FUNCTION__, sig);
	void *array[10];
	size_t i, size;
	char **strings;

	size = backtrace(array, 10);
	strings = backtrace_symbols(array, size);

	for (i = 0; i < size; ++i)
	{
		NXLOGE("[%s] %s", __FUNCTION__, strings[i]);
	}

	NxLauncher *pInstance = NxLauncher::GetInstance();
//	if (pInstance)
//		delete pInstance;

//	exit(EXIT_FAILURE);
	if (sig == SIGTERM || sig == SIGINT)
	{
		if (pInstance)
		{
			QApplication::postEvent(pInstance, new QEvent((QEvent::Type)E_NX_EVENT_TERMINATE));
		}
	}

	signal(SIGABRT, NULL);
	signal(SIGSEGV, NULL);
	signal(SIGINT, NULL);
	signal(SIGTERM, NULL);
}

NxLauncher* NxLauncher::GetInstance()
{
	return m_spInstance;
}

NxLauncher::NxLauncher(QWidget *parent) :
	QDialog(parent, Qt::FramelessWindowHint),
	ui(new Ui::NxLauncher)
{
	ui->setupUi(this);

	m_spInstance = this;
	installEventFilter( this );

	signal(SIGABRT, signal_handler);
	signal(SIGSEGV, signal_handler);
	signal(SIGINT, signal_handler);
	signal(SIGTERM, signal_handler);

	// scan applications
	m_pPackageManager = new NxPackageScanner(NX_APP_PATH, &m_PlugIns);
	connect(m_pPackageManager, SIGNAL(signalPlugInUpdated(QString)), this, SLOT(slotPlugInUpdated(QString)));

	// step2. scan applications - extract function pointer
	foreach (NxPluginInfo *psInfo, m_PlugIns)
	{
		QDir dir(psInfo->getPath());
		QFileInfoList plugins = dir.entryInfoList(QStringList() << "*.so", QDir::Files);
		if (!plugins.size())
			continue;

		psInfo->m_pHandle = dlopen(plugins[0].filePath().toStdString().c_str(), RTLD_LAZY);
		if (!psInfo->m_pHandle)
		{
			NXLOGE("[%s] dlopen(): %s ", __FUNCTION__, dlerror());
			continue;
		}

		// initialize
		psInfo->m_pInit = (void (*)(void *, const char *))dlsym(psInfo->m_pHandle, "Init");
		// check if the instance is initialized
		psInfo->m_pIsInit = (void (*)(bool *))dlsym(psInfo->m_pHandle, "IsInit");
		// deInitialize
		psInfo->m_pdeInit = (void (*)())dlsym(psInfo->m_pHandle, "deInit");
		// launcher topmost
		psInfo->m_pRegisterLauncherShow = (void (*)(void(*)(bool*)))dlsym(psInfo->m_pHandle, "RegisterRequestLauncherShow");
		// request audio focus
		psInfo->m_pRequestAudioFocus = (void (*)(FocusType, FocusPriority, bool*))dlsym(psInfo->m_pHandle, "RequestAudioFocus");
		// register callback for request audio focus
		psInfo->m_pRegisterRequestAudioFocus = (void (*)(void(*)(FocusPriority, bool*)))dlsym(psInfo->m_pHandle, "RegisterRequestAudioFocus");
		// request audio focus transient
		psInfo->m_pRequestAudioFocusTransient = (void (*)(FocusPriority,bool*))dlsym(psInfo->m_pHandle, "RequestAudioFocusTransient");
		// register callback for request audio focus transient
		psInfo->m_pRegisterRequestAudioFocusTransient = (void (*)(void(*)(FocusPriority,bool*)))dlsym(psInfo->m_pHandle, "RegisterRequestAudioFocusTransient");
		// register callback for request audio focus loss
		psInfo->m_pRegisterRequestAudioFocusLoss = (void (*)(void (*)()))dlsym(psInfo->m_pHandle, "RegisterRequestAudioFocusLoss");
		// request video focus
		psInfo->m_pRequestVideoFocus = (void (*)(FocusType, FocusPriority, bool*))dlsym(psInfo->m_pHandle, "RequestVideoFocus");
		// register callback for request video focus
		psInfo->m_pRegisterRequestVideoFocus = (void (*)(void(*)(FocusPriority,bool*)))dlsym(psInfo->m_pHandle, "RegisterRequestVideoFocus");
		// request video focus transient
		psInfo->m_pRequestVideoFocusTransient = (void (*)(FocusPriority, bool*))dlsym(psInfo->m_pHandle, "RequestVideoFocusTransient");
		// register callback for video focus transient
		psInfo->m_pRegisterRequestVideoFocusTransient = (void (*)(void(*)(FocusPriority, bool*)))dlsym(psInfo->m_pHandle, "RegisterRequestVideoFocusTransient");
		// register callback for request video focus loss
		psInfo->m_pRegisterRequestVideoFocusLoss = (void (*)(void (*)()))dlsym(psInfo->m_pHandle, "RegisterRequestVideoFocusLoss");
		psInfo->m_pRegisterRequestPluginRun = (void (*)(void (*)(const char*, const char*)))dlsym(psInfo->m_pHandle, "RegisterRequestPlugInRun");
		psInfo->m_pRegisterRequestPluginTerminate = (void (*)(void (*)(const char*)))dlsym(psInfo->m_pHandle, "RegisterRequestPlugInTerminate");
		psInfo->m_pRegisterRequestPluginIsRunning = (void (*)(void (*)(const char*, bool*)))dlsym(psInfo->m_pHandle, "RegisterRequestPlugInIsRunning");
		// send message
		psInfo->m_pSendMessage = (void (*)(const char *, const char*, int32_t))dlsym(psInfo->m_pHandle, "SendMessage");
		// register callback for request send message
		psInfo->m_pRegisterRequestMessage = (void (*)(void (*)(const char*, const char*, int32_t)))dlsym(psInfo->m_pHandle, "RegisterRequestSendMessage");
		// popup message
		psInfo->m_pRegisterRequestPopupMessage = (void (*)(void (*)(PopupMessage*, bool*)))dlsym(psInfo->m_pHandle, "RegisterRequestPopupMessage");
		psInfo->m_pRegisterRequestExpirePopupMessage = (void (*)(void (*)()))dlsym(psInfo->m_pHandle, "RegisterRequestExpirePopupMessage");
		psInfo->m_pPopupMessageResponse = (void (*)(bool))dlsym(psInfo->m_pHandle, "PopupMessageResponse");
		// notification
		psInfo->m_pRegisterRequestNotification = (void (*)(void (*)(PopupMessage*)))dlsym(psInfo->m_pHandle, "RegisterRequestNotification");
		psInfo->m_pRegisterRequestExpireNotification = (void (*)(void (*)()))dlsym(psInfo->m_pHandle, "RegisterRequestExpireNotification");
		psInfo->m_pNotificationResponse = (void (*)(bool))dlsym(psInfo->m_pHandle, "NotificationResponse");
		// terminate
		psInfo->m_pRegisterRequestTerminate = (void (*)(void (*)(void)))dlsym(psInfo->m_pHandle, "RegisterRequestTerminate");
		psInfo->m_pRegisterRequestVolume = (void (*)(void (*)(void)))dlsym(psInfo->m_pHandle, "RegisterRequestVolume");
		// back button clicked
		psInfo->m_pBackButtonClicked = (void (*)())dlsym(psInfo->m_pHandle, "BackButtonClicked");
		// media event
		psInfo->m_pMediaEventChanged = (void (*)(NxMediaEvent))dlsym(psInfo->m_pHandle, "MediaEventChanged");

#if 0
		qDebug() << plugins[0].filePath();
		qDebug() << "m_pInit" << psInfo->m_pInit;
		qDebug() << "m_pIsInit" << psInfo->m_pIsInit;
		qDebug() << "m_pdeInit" << psInfo->m_pdeInit;
		qDebug() << "m_pShow" << psInfo->m_pShow;
		qDebug() << "m_pHide" << psInfo->m_pHide;
		qDebug() << "m_pRaise" << psInfo->m_pRaise;
		qDebug() << "m_pLower" << psInfo->m_pLower;
		qDebug() << "m_pRegisterLauncherShow" << psInfo->m_pRegisterLauncherShow;
		qDebug() << "m_pRegisterShow" << psInfo->m_pRegisterShow;
		qDebug() << "m_pRequestAudioFocus" << psInfo->m_pRequestAudioFocus;
		qDebug() << "m_pRegisterRequestAudioFocus" << psInfo->m_pRegisterRequestAudioFocus;
		qDebug() << "m_pRequestAudioFocusTransient" << psInfo->m_pRequestAudioFocusTransient;
		qDebug() << "m_pRegisterRequestAudioFocusTransient" << psInfo->m_pRegisterRequestAudioFocusTransient;
		qDebug() << "m_pRegisterRequestAudioFocusLoss" << psInfo->m_pRegisterRequestAudioFocusLoss;
		qDebug() << "m_pRequestVideoFocus" << psInfo->m_pRequestVideoFocus;
		qDebug() << "m_pRegisterRequestVideoFocus" << psInfo->m_pRegisterRequestVideoFocus;
		qDebug() << "m_pRequestVideoFocusTransient" << psInfo->m_pRequestVideoFocusTransient;
		qDebug() << "m_pRegisterRequestVideoFocusTransient" << psInfo->m_pRegisterRequestVideoFocusTransient;
		qDebug() << "m_pRegisterRequestVideoFocusLoss" << psInfo->m_pRegisterRequestVideoFocusLoss;
		qDebug() << "m_pRegisterRequestPluginRun" << psInfo->m_pRegisterRequestPluginRun;
		qDebug() << "m_pRegisterRequestPluginTerminate" << psInfo->m_pRegisterRequestPluginTerminate;
		qDebug() << "m_pSendMessage" << psInfo->m_pSendMessage;
		qDebug() << "m_pRegisterRequestMessage" << psInfo->m_pRegisterRequestMessage;
		qDebug() << "m_pRegisterRequestPopupMessage" << psInfo->m_pRegisterRequestPopupMessage;
		qDebug() << "m_pPopupMessageResponse" << psInfo->m_pPopupMessageResponse;
#endif
	}

	// step3. register callback to applications.
	foreach (NxPluginInfo *psInfo, m_PlugIns)
	{
		// launcher show
		if (psInfo->m_pRegisterLauncherShow)
			psInfo->m_pRegisterLauncherShow(RequestLauncherShow);
		// audio focus
		if (psInfo->m_pRegisterRequestAudioFocus)
			psInfo->m_pRegisterRequestAudioFocus(RequestAudioFocus);
		if (psInfo->m_pRegisterRequestAudioFocusTransient)
			psInfo->m_pRegisterRequestAudioFocusTransient(RequestAudioFocusTransient);
		if (psInfo->m_pRegisterRequestAudioFocusLoss)
			psInfo->m_pRegisterRequestAudioFocusLoss(RequestAudioFocusLoss);
		// video focus
		if (psInfo->m_pRegisterRequestVideoFocus)
			psInfo->m_pRegisterRequestVideoFocus(RequestVideoFocus);
		if (psInfo->m_pRegisterRequestVideoFocusTransient)
			psInfo->m_pRegisterRequestVideoFocusTransient(RequestVideoFocusTransient);
		if (psInfo->m_pRegisterRequestVideoFocusLoss)
			psInfo->m_pRegisterRequestVideoFocusLoss(RequestVideoFocusLoss);
		// popup
		if (psInfo->m_pRegisterRequestPopupMessage)
			psInfo->m_pRegisterRequestPopupMessage(RequestPopupMessage);
		if (psInfo->m_pRegisterRequestExpirePopupMessage)
			psInfo->m_pRegisterRequestExpirePopupMessage(RequestExpirePopupMessage);
		// notification
		if (psInfo->m_pRegisterRequestNotification)
			psInfo->m_pRegisterRequestNotification(RequestNotification);
		if (psInfo->m_pRegisterRequestExpireNotification)
			psInfo->m_pRegisterRequestExpireNotification(RequestExpireNotification);
		// message
		if (psInfo->m_pRegisterRequestMessage)
			psInfo->m_pRegisterRequestMessage(RequestSendMessage);
		// run/terminate from other plugin
		if (psInfo->m_pRegisterRequestPluginRun)
			psInfo->m_pRegisterRequestPluginRun(RequestPlugInRun);
		if (psInfo->m_pRegisterRequestPluginTerminate)
			psInfo->m_pRegisterRequestPluginTerminate(RequestPlugInTerminate);
		if (psInfo->m_pRegisterRequestPluginIsRunning)
			psInfo->m_pRegisterRequestPluginIsRunning(RequestPlugInIsRunning);

		if (psInfo->m_pRegisterRequestTerminate)
			psInfo->m_pRegisterRequestTerminate(RequestTerminate);
		if (psInfo->m_pRegisterRequestVolume)
			psInfo->m_pRegisterRequestVolume(RequestVolume);
	}

	foreach (NxPluginInfo *psInfo, m_PlugIns) {
		if (psInfo->getAutoStart())
		{
			if (psInfo->m_pInit)
				psInfo->m_pInit(this, "");
		}
	}

	m_VideoFocusQueue.push_front(NX_LAUNCHER);

	// set application name
	ui->statusBar->SetTitleName("Home");
	ui->statusBar->RegOnClickedHome(cbStatusHome);
	ui->statusBar->RegOnClickedBack(cbStatusBack);
	ui->statusBar->RegOnClickedVolume(cbStatusVolume);

	connect(ui->volumeBar, SIGNAL(signalSetVolume(int)), this, SLOT(slotSetVolume(int)));

	const QRect screen = QApplication::desktop()->screenGeometry();
	if ((width() != screen.width()) || (height() != screen.height()))
	{
		setFixedSize(screen.width(), screen.height());
	}

#ifndef CONFIG_USE_NO_QML
	// binding between qml and widget
	m_pLauncherWidget = new QQuickWidget(ui->launcher);
	m_pLauncherWidget->setSource(QUrl("qrc:/qml/NxLauncher.qml"));
	m_pLauncherWidget->rootContext()->setContextProperty("gui", this);
	m_pLauncherWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
	m_pLauncherWidget->move(0, ui->statusBar->height());
	m_pLauncherWidget->resize(1024, 540);

	if (m_pLauncherWidget->rootObject())
	{
		connect(m_pLauncherWidget->rootObject(), SIGNAL(launchProgram(QString)), this, SLOT(slotExecute(QString)));
	}
#else
	m_pPrevPageButton = new QPushButton(ui->launcher);
	connect(m_pPrevPageButton, SIGNAL(clicked(bool)), this, SLOT(onPrevPageButtonClicked()));
	m_pPrevPageButton->setFixedSize(50, 100);
	m_pPrevPageButton->move(0, height()/2-m_pPrevPageButton->height()/2);
	m_pPrevPageButton->hide();

	m_pNextPageButton = new QPushButton(ui->launcher);
	connect(m_pNextPageButton, SIGNAL(clicked(bool)), this, SLOT(onNextPageButtonClicked()));
	m_pNextPageButton->setFixedSize(50, 100);
	m_pNextPageButton->move(width()-m_pNextPageButton->width(), height()/2-m_pNextPageButton->height()/2);
	m_pNextPageButton->hide();

	ui->launcher->setStyleSheet("background:rgb(195,195,195);");
	m_pPageStackFrame = new PageStackFrame(ui->launcher);
	connect(m_pPageStackFrame, SIGNAL(signalResizeItemDone()), this, SLOT(slotResizeItemDone()));
	connect(m_pPageStackFrame, SIGNAL(onButtonClicked(NxPluginInfo*)), this, SLOT(onExecute(NxPluginInfo*)));
	connect(this, SIGNAL(signalStateChanged(NxPluginInfo*)), m_pPageStackFrame, SLOT(onButtonStateChnaged(NxPluginInfo*)));
	m_pPageStackFrame->setCellSize(QSize(200, 200));
	m_pPageStackFrame->setSpacing(25);
	m_pPageStackFrame->setStyleSheet("background:rgb(195,195,195);");
	m_pPageStackFrame->move(m_pPrevPageButton->width(), this->size().height() * 1 / 10);
	m_pPageStackFrame->resize( this->size().width()-m_pPrevPageButton->width()-m_pNextPageButton->width(), this->size().height() * 9 / 10 );
	m_pPageStackFrame->setFrameShape(QFrame::NoFrame);
	m_pPageStackFrame->setFrameShadow(QFrame::Plain);

	foreach (NxPluginInfo *psInfo, m_PlugIns) {
		if (!psInfo->getVisible())
		{
			continue;
		}

		m_pPageStackFrame->pushItem(psInfo);
	}
#endif
	connect(ui->messageFrame, SIGNAL(signalOk()), this, SLOT(slotPopupMessageAccept()));
	connect(ui->messageFrame, SIGNAL(signalCancel()), this, SLOT(slotPopupMessageReject()));
	connect(ui->notificationBar, SIGNAL(signalOk()), this, SLOT(slotNotificationAccept()));
	connect(ui->notificationBar, SIGNAL(signalCancel()), this, SLOT(slotNotificationReject()));

	m_pMediaScanner = new MediaScanner();
	connect(m_pMediaScanner, SIGNAL(signalMediaEvent(NxEventTypes)), this, SLOT(slotMediaEvent(NxEventTypes)));

	connect(&m_CommandTimer, SIGNAL(timeout()), this, SLOT(slotCommandTimer()));
	m_CommandTimer.start(100);

	connect(&m_Timer, SIGNAL(timeout()), this, SLOT(slotTimer()));
//	m_Timer.start(10000);

//	QTimer::singleShot(2000, this, SLOT(slotStartSerivceTimer()));
}

NxLauncher::~NxLauncher()
{
	delete ui;
}

void NxLauncher::slotStartSerivceTimer()
{
	foreach (NxPluginInfo *psInfo, m_PlugIns) {
		if (psInfo->getType().toLower() == "service" && psInfo->getEnabled())
		{
			if (psInfo->m_pInit)
				psInfo->m_pInit(this, "");
		}
	}
}

void NxLauncher::slotMediaEvent(NxEventTypes eType)
{
	NxMediaEvent eEvent = NX_EVENT_MEDIA_UNKNOWN;

	switch (eType) {
	case E_NX_EVENT_SDCARD_INSERT:
		eEvent = NX_EVENT_MEDIA_SDCARD_INSERT;
		NXLOGI("[%s] NX_EVENT_MEDIA_SDCARD_INSERT", __FUNCTION__);
		break;

	case E_NX_EVENT_SDCARD_REMOVE:
		eEvent = NX_EVENT_MEDIA_SDCARD_REMOVE;
		NXLOGI("[%s] NX_EVENT_MEDIA_SDCARD_REMOVE", __FUNCTION__);
		break;

	case E_NX_EVENT_USB_INSERT:
		eEvent = NX_EVENT_MEDIA_USB_INSERT;
		NXLOGI("[%s] NX_EVENT_MEDIA_USB_INSERT", __FUNCTION__);
		break;

	case E_NX_EVENT_USB_REMOVE:
		eEvent = NX_EVENT_MEDIA_USB_REMOVE;
		NXLOGI("[%s] NX_EVENT_MEDIA_USB_REMOVE", __FUNCTION__);
		break;

	case E_NX_EVENT_MEDIA_SCAN_DONE:
		eEvent = NX_EVENT_MEDIA_SCAN_DONE;
		NXLOGI("[%s] NX_EVENT_MEDIA_SCAN_DONE", __FUNCTION__);
		break;

	default:
		eEvent = NX_EVENT_MEDIA_UNKNOWN;
		break;
	}

	foreach (NxPluginInfo *plugin, m_PlugIns) {
		if (plugin->m_pMediaEventChanged)
			plugin->m_pMediaEventChanged(eEvent);
	}
}

void NxLauncher::cbStatusHome(void *)
{
	if (m_spInstance)
	{
		bool bOk = false;
		m_spInstance->LauncherShow(&bOk, true);
	}
}

void NxLauncher::cbStatusBack(void *)
{
	if (m_spInstance)
	{
		m_spInstance->BackButtonClicked();
	}
}

void NxLauncher::BackButtonClicked()
{
	QString owner = m_VideoFocusQueue.first();

	if (m_PlugIns.find(owner) != m_PlugIns.end())
	{
		if (m_PlugIns[owner]->m_pBackButtonClicked)
			m_PlugIns[owner]->m_pBackButtonClicked();
	}
}

void NxLauncher::cbStatusVolume(void *)
{
	QApplication::postEvent(m_spInstance, new NxVolumeControlEvent());
}

void NxLauncher::RequestLauncherShow(bool *bOk)
{
	NXLOGI("[%s]", __FUNCTION__);
	if (m_spInstance)
		m_spInstance->LauncherShow(bOk, true);
}

QString NxLauncher::FindCaller(uint32_t uiLevel)
{
	void *array[10];
	int i = 0;
	size_t size, count = 0;
	char **strings;
	char modules[256] = {0,};

	size = backtrace(array, 10);
	strings = backtrace_symbols(array, size);

#if 0
	for (size_t k = 0; k < size; ++k)
	{
		NXLOGI("[%s] %s", __FUNCTION__, strings[k]);
	}
#endif

	if (size > uiLevel)
	{
		int32_t last = -1;
		int32_t stx = -1, etx = -1;
		for (i = 0; i < (int32_t)strlen(strings[uiLevel]); ++i)
		{
			if (strings[uiLevel][i] == '(')
				last = i;
		}

		for (i = (size_t)last-1; i >= 0; --i)
		{
			if (strings[uiLevel][i] == '/')
			{
				if (++count == 1)
				{
					etx = i;
				}
				else
				{
					stx = i;
					break;
				}
			}
		}

		if (stx >= 0 && etx >= 0)
		{
			strncpy(modules, strings[uiLevel]+stx+1, etx-stx-1);
		}

		free(strings);
	}

	return modules;
}

void NxLauncher::RequestSendMessage(const char *pDst, const char *pMsg, int32_t iMsgSize)
{
	QString from = FindCaller(2);
	QString key = QString::fromLatin1(pDst);

	if (m_spInstance->m_PlugIns.find(key) != m_spInstance->m_PlugIns.end())
	{
		if (m_spInstance->m_PlugIns[key]->m_pIsInit && m_spInstance->m_PlugIns[key]->m_pSendMessage)
		{
			bool bOk = false;
			m_spInstance->m_PlugIns[key]->m_pIsInit(&bOk);

			if (bOk)
				m_spInstance->m_PlugIns[key]->m_pSendMessage(from.toStdString().c_str(), pMsg, iMsgSize);
		}
	}
}

/************************************************************************************\
 * D-AUDIO INTERFACE - RequestPlugInRun
 *
 * Description
 *  - This function in used when you want to run a specific plugin.
 *
 * Parameters
 *  - pPlugin : Name of the plugin you want to run.
 *  - pArgs   : Arguments of plugin.
 *
 * For example)
 *  - When the call status of HFP is changed to INCOMING with no NxBTPhone instance,
 *    NxBTService must execute NxBTPhone plugin.
 *    - pPlugin : "libnxbtphone"
 *    - pArgs   : "--menu calling"
 ************************************************************************************/
void NxLauncher::RequestPlugInRun(const char *pPlugin, const char *pArgs)
{
	NxLauncher *p = m_spInstance;
	QString key = QString::fromLatin1(pPlugin);
	bool bOk = false;

	NXLOGI("[%s] plugin(%s), argument(%s)", __FUNCTION__, pPlugin, pArgs);

	if (p->m_PlugIns.find(key) == p->m_PlugIns.end())
	{
		NXLOGE("[%s] The plug-in '%s' does not exist.", __FUNCTION__, pPlugin);
		return;
	}

	if (!(p->m_PlugIns[key]->m_pInit) || !(p->m_PlugIns[key]->m_pIsInit))
	{
		NXLOGE("[%s] The D-Audio Interface has a missing function.", __FUNCTION__);
		NXLOGE("[%s] Make sure the Init and IsInit functions are defined.", __FUNCTION__);
		return;
	}

	// check if the instance exists.
	p->m_PlugIns[key]->m_pIsInit(&bOk);
	if (!bOk)
		p->m_PlugIns[key]->m_pInit(p, pArgs);
}

/************************************************************************************\
 * D-AUDIO INTERFACE - RequestPlugInTerminate
 *
 * Description
 *  - It is used to terminate the currently running plugin.
 *
 * Parameters
 *  - pPlugin : Name of the plugin you want to terminate.
 *
 * For example)
 *  - When the service loses audio focus while music is playing,
 *    NxBTAudio should be terminated.
 *    - pPlugin : "libnxbtaudio"
 ************************************************************************************/
void NxLauncher::RequestPlugInTerminate(const char *pPlugin)
{
	QString caller = FindCaller(2);
	NxLauncher *p = m_spInstance;
	QString key = QString::fromLatin1(pPlugin);
	bool bOk = false;

	NXLOGI("[%s] plugin(%s) from %s", __FUNCTION__, pPlugin, caller.toStdString().c_str());

	if (p->m_PlugIns.find(key) == p->m_PlugIns.end())
	{
		NXLOGE("[%s] The plug-in '%s' does not exist.", __FUNCTION__, pPlugin);
		return;
	}

	if (!(p->m_PlugIns[key]->m_pdeInit) || !(p->m_PlugIns[key]->m_pIsInit))
	{
		NXLOGE("[%s] The D-Audio Interface has a missing function.", __FUNCTION__);
		NXLOGE("[%s] Make sure the deInit and IsInit functions are defined.", __FUNCTION__);
		return;
	}

	// check if the instance exists.
	p->m_PlugIns[key]->m_pIsInit(&bOk);
	if (bOk)
		p->m_PlugIns[key]->m_pdeInit();
}

/************************************************************************************\
 * D-AUDIO INTERFACE - RequestPlugInIsRunning
 *
 * Description
 *  - Check that the plug-in is running.
 *
 * Parameters
 *  - pPlugin : The plug-in you want to check.
 *  - bOk : If the plug-in is running, set 'bOk' to true. Otherwise, set 'bOk' to false.
 ************************************************************************************/
void NxLauncher::RequestPlugInIsRunning(const char *pPlugin, bool *bOk)
{
	NxLauncher *p = m_spInstance;
	QString key = QString::fromLatin1(pPlugin);
	*bOk = false;

	if (p->m_PlugIns.find(key) == p->m_PlugIns.end())
	{
		NXLOGE("[%s] The plug-in '%s' does not exist.", __FUNCTION__, pPlugin);
		return;
	}

	if (!(p->m_PlugIns[key]->m_pIsInit))
	{
		NXLOGE("[%s] The D-Audio Interface has a missing function.", __FUNCTION__);
		NXLOGE("[%s] Make sure the IsInit functions are defined.", __FUNCTION__);
		return;
	}

	// check if the instance exists.
	p->m_PlugIns[key]->m_pIsInit(bOk);
}

/************************************************************************************\
 * D-AUDIO INTERFACE - RequestAudioFocus
 *
 * Description
 *  - Gets the focus from the focus owner and passes the focus to the requestor.
 *  - If the focus of the current owner is higher than the priority of the requestor,
 *    it always fails.
 *
 * Return value
 *  - If successful, 'bOk' is set to 'true';
 *  - Otherwise, 'bOk' is set to 'false'.
 ************************************************************************************/
void NxLauncher::RequestAudioFocus(FocusPriority ePriority, bool *bOk)
{
	QString caller = FindCaller(2);
	QString owner = m_AudioFocusQueue.size() ? m_AudioFocusQueue.first() : "";

	NXLOGI("[%s] <TRY> owner(%s), caller(%s)", __FUNCTION__, owner.toStdString().c_str(), caller.toStdString().c_str());

	if (!owner.isEmpty() && (!(m_spInstance->m_PlugIns[owner]->m_pIsInit) || !(m_spInstance->m_PlugIns[owner]->m_pRequestAudioFocus)))
	{
		NXLOGE("[%s] The D-Audio Interface has a missing function.", __FUNCTION__);
		NXLOGE("[%s] Make sure the IsInit and RequestAudioFocus functions are defined.", __FUNCTION__);
		return;
	}

	if (caller == owner)
	{
		*bOk = true;
		return;
	}
	else if (owner.isEmpty())
	{
		*bOk = true;
	}
	else
	{
		// Action
		//  - if the owner of the video focus exists, the focus is taken from the owner.

		// 1. check if the instance exists.
		m_spInstance->m_PlugIns[owner]->m_pIsInit(bOk);

		// 2. if the instance exists, request the command for VideoFocus.
		if (*bOk)
		{
			NXLOGI("[%s] <TRY> RequestAudioFocus -> [%s]", __FUNCTION__, owner.toStdString().c_str());
			m_spInstance->m_PlugIns[owner]->m_pRequestAudioFocus(FocusType_Get, ePriority, bOk);
			NXLOGI("[%s] <DONE> RequestAudioFocus -> [%s] <%s>", __FUNCTION__, owner.toStdString().c_str(), bOk ? "OK" : "NG");
		}
	}

	if (*bOk)
	{
		if (m_AudioFocusQueue.size())
			m_AudioFocusQueue.removeAll(caller);
		m_AudioFocusQueue.push_front(caller);
	}
}

/************************************************************************************\
 * D-AUDIO INTERFACE - RequestAudioFocusTransient
 *
 * Description
 *  - Gets the focus from the focus owner for a moment, uses it, and then passes
 *    the focus back to the original owner.
 *  - If the focus of the current owner is higher than the priority of the requestor,
 *    it always fails.
 *
 * Return value
 *  - If successful, 'bOk' is set to 'true';
 *  - Otherwise, 'bOk' is set to 'false'.
 ************************************************************************************/
void NxLauncher::RequestAudioFocusTransient(FocusPriority ePriority, bool *bOk)
{
	QString caller = FindCaller(2);
	QString owner = m_AudioFocusQueue.size() ? m_AudioFocusQueue.first() : "";

	NXLOGI("[%s] <TRY> owner(%s), caller(%s)", __FUNCTION__, owner.toStdString().c_str(), caller.toStdString().c_str());

	if (!owner.isEmpty() && (!(m_spInstance->m_PlugIns[owner]->m_pIsInit) || !(m_spInstance->m_PlugIns[owner]->m_pRequestAudioFocusTransient)))
	{
		NXLOGE("[%s] The D-Audio Interface has a missing function.", __FUNCTION__);
		NXLOGE("[%s] Make sure the IsInit and RequestAudioFocusTransient functions are defined.", __FUNCTION__);
		return;
	}

	if (caller == owner)
	{
		*bOk = true;
		return;
	}
	else if (owner.isEmpty())
	{
		*bOk = true;
	}
	else
	{
		// Action
		//  -

		// 1. check if the instance exists.
		m_spInstance->m_PlugIns[owner]->m_pIsInit(bOk);

		// 2. if the instance exists, request the command for AudioFocusTransient.
		if (bOk)
			m_spInstance->m_PlugIns[owner]->m_pRequestAudioFocusTransient(ePriority, bOk);
	}

	if (*bOk)
	{
		if (m_AudioFocusQueue.size())
			m_AudioFocusQueue.removeAll(caller);
		m_AudioFocusQueue.push_front(caller);
	}
}

/************************************************************************************\
 * D-AUDIO INTERFACE - RequestAudioFocusLoss
 *
 * Description
 *  - After the focus has been used, it should be called.
 ************************************************************************************/
void NxLauncher::RequestAudioFocusLoss()
{
	QString caller = FindCaller(2);
	QString prev = m_AudioFocusQueue.first();
	QString curr;

	NXLOGI("[%s] <TRY> owner(%s), caller(%s)", __FUNCTION__, prev.toStdString().c_str(), caller.toStdString().c_str());

	// if the focus owner has called this function, remove it from the queue.
	// then, it gives the focus to the next owner.
	if (prev == caller)
	{
		// 1. remove from the queue.
		m_AudioFocusQueue.pop_front();

		// 2. check whether the next owner exists.
		if (m_AudioFocusQueue.size())
		{
			curr = m_AudioFocusQueue.first();
			if (prev != curr)
			{
				// 2.1. pass the focus to the next owner.
				bool bOk = false;
				m_spInstance->m_PlugIns[curr]->m_pRequestAudioFocus(FocusType_Set, FocusPriority_Normal, &bOk);
			}
		}
	}

	curr = m_AudioFocusQueue.size() ? m_AudioFocusQueue.first() : "";
	NXLOGI("[%s] <DONE> owner(%s)", __FUNCTION__, curr.toStdString().c_str());
}

/************************************************************************************\
 * D-AUDIO INTERFACE - RequestVideoFocus
 *
 * Description
 *  - Gets the focus from the focus owner and passes the focus to the requestor.
 *  - If the focus of the current owner is higher than the priority of the requestor,
 *    it always fails.
 *
 * Return value
 *  - If successful, 'bOk' is set to 'true';
 *  - Otherwise, 'bOk' is set to 'false'.
 ************************************************************************************/
void NxLauncher::RequestVideoFocus(FocusPriority ePriority, bool *bOk)
{
	if (m_spInstance)
		m_spInstance->VideoFocus(ePriority, bOk);
}

void NxLauncher::VideoFocus(FocusPriority ePriority, bool *bOk)
{
	QString caller = FindCaller(2);
	QString owner = m_VideoFocusQueue.first();

	NXLOGI("[%s] <TRY> owner(%s), caller(%s)", __FUNCTION__, owner.toStdString().c_str(), caller.toStdString().c_str());

	if (owner != NX_LAUNCHER && (!(m_spInstance->m_PlugIns[owner]->m_pIsInit) || !(m_spInstance->m_PlugIns[owner]->m_pRequestVideoFocus)))
	{
		NXLOGE("[%s] The D-Audio Interface has a missing function.", __FUNCTION__);
		NXLOGE("[%s] Make sure the IsInit and RequestVideoFocus functions are defined.", __FUNCTION__);
		return;
	}

	if (caller == owner)
	{
		*bOk = true;
		return;
	}
	else if (owner == NX_LAUNCHER)
	{
		*bOk = true;
	}
	else
	{
		// Action
		//  - if the owner of the video focus exists, the focus is taken from the owner.

		// 1. check if the instance exists.
		m_spInstance->m_PlugIns[owner]->m_pIsInit(bOk);

		// 2. if the instance exists, request the command for VideoFocus.
		if (*bOk)
		{
			NXLOGI("[%s] <TRY> RequestAudioFocus -> [%s]", __FUNCTION__, owner.toStdString().c_str());
			m_spInstance->m_PlugIns[owner]->m_pRequestVideoFocus(FocusType_Get, ePriority, bOk);
			NXLOGI("[%s] <DONE> RequestAudioFocus -> [%s] <%s>", __FUNCTION__, owner.toStdString().c_str(), bOk ? "OK" : "NG");
		}
	}

	if (*bOk)
	{
		AddVideoFocus(caller);
	}
}

/************************************************************************************\
 * D-AUDIO INTERFACE - RequestVideoFocusTransient
 *
 * Description
 *  - Gets the focus from the focus owner for a moment, uses it, and then passes
 *    the focus back to the original owner.
 *  - If the focus of the current owner is higher than the priority of the requestor,
 *    it always fails.
 *
 * Return value
 *  - If successful, 'bOk' is set to 'true';
 *  - Otherwise, 'bOk' is set to 'false'.
 ************************************************************************************/
void NxLauncher::RequestVideoFocusTransient(FocusPriority ePriority, bool *bOk)
{
	QString caller = FindCaller(2);
	QString owner = m_VideoFocusQueue.first();

	NXLOGI("[%s] <TRY> owner(%s), caller(%s)", __FUNCTION__, owner.toStdString().c_str(), caller.toStdString().c_str());

	if (owner != NX_LAUNCHER && (!(m_spInstance->m_PlugIns[owner]->m_pIsInit) || !(m_spInstance->m_PlugIns[owner]->m_pRequestVideoFocusTransient)))
	{
		NXLOGE("[%s] The D-Audio Interface has a missing function.", __FUNCTION__);
		NXLOGE("[%s] Make sure the IsInit and RequestVideoFocusTransient functions are defined.", __FUNCTION__);
		return;
	}

	if (caller == owner)
	{
		*bOk = true;
		return;
	}
	else if (owner == NX_LAUNCHER)
	{
		*bOk = true;
	}
	else
	{
		// Action
		//  -

		// 1. check if the instance exists.
		m_spInstance->m_PlugIns[owner]->m_pIsInit(bOk);

		// 2. if the instance exists, request the command for AudioFocusTransient.
		if (bOk)
			m_spInstance->m_PlugIns[owner]->m_pRequestVideoFocusTransient(ePriority, bOk);
	}

	if (*bOk)
	{
#if 0
		m_VideoFocusQueue.removeAll(caller);
		m_VideoFocusQueue.push_front(caller);
#else
		int i = m_VideoFocusQueue.lastIndexOf(caller);
		for ( ; i > 0; --i)
		{
			m_VideoFocusQueue.swap(i, i-1);
		}
#endif
	}
}

/************************************************************************************\
 * D-AUDIO INTERFACE - RequestVideoFocusLoss
 *
 * Description
 *  - After the focus has been used, it should be called.
 ************************************************************************************/
void NxLauncher::RequestVideoFocusLoss()
{
	QString caller = FindCaller(2);
	QString prev = m_VideoFocusQueue.first();
	QString curr;

	NXLOGI("[%s] <TRY> owner(%s), caller(%s)", __FUNCTION__, prev.toStdString().c_str(), caller.toStdString().c_str());

	// if the focus owner has called this function, remove it from the queue.
	// then, it gives the focus to the next owner.
	if (prev == caller)
	{
		// 1. remove from the queue.
#if 0
		m_VideoFocusQueue.removeAll(prev);
		m_VideoFocusQueue.push_back(prev);
#else
		for (int i = 0; i < m_VideoFocusQueue.size()-1; ++i)
		{
			m_VideoFocusQueue.swap(i, i+1);
		}
#endif

		// 2. check whether the next owner exists.
		curr = m_VideoFocusQueue.first();
		m_spInstance->UpdateTitle(curr);

		if (NX_LAUNCHER == curr)
		{
			bool bOk = false;
			m_spInstance->LauncherShow(&bOk, false);
		}
		else
		{
			// 2.1. pass the focus to the next owner.
			bool bOk = false;
			m_spInstance->m_PlugIns[curr]->m_pRequestVideoFocus(FocusType_Set, FocusPriority_Normal, &bOk);
		}
	}

	curr = m_VideoFocusQueue.size() ? m_VideoFocusQueue.first() : "";
	NXLOGI("[%s] <DONE> owner(%s)", __FUNCTION__, curr.toStdString().c_str());
}

/************************************************************************************\
 * D-AUDIO INTERFACE - RequestPopupMessage
 *
 * Description
 *  - Request a pop-up message.
 ************************************************************************************/
void NxLauncher::RequestPopupMessage(PopupMessage *psPopup, bool *bOk)
{
	QString caller = FindCaller(2);
	QString owner = m_VideoFocusQueue.first();

	if (owner == NX_LAUNCHER)
		*bOk = true;
	else
	{
		m_spInstance->m_PlugIns[owner]->m_pRequestVideoFocusTransient(FocusPriority_Normal, bOk);

		if (!*bOk)
		{
			NXLOGE("[%s] REQUEST VIDEO FOCUS TRANSIENT", __FUNCTION__);
			return;
		}
	}

	QApplication::postEvent(m_spInstance, new NxPopupMessageEvent(psPopup, caller));
}

/************************************************************************************\
 * D-AUDIO INTERFACE - RequestExpirePopupMessage
 *
 * Description
 *  - A pop-up message is requested to expire.
 ************************************************************************************/
void NxLauncher::RequestExpirePopupMessage()
{
	m_spInstance->ExpirePopupMessage();
}

/************************************************************************************\
 * D-AUDIO INTERFACE Wrapper - ExpirePopupMessage
 *
 * Description
 *  - Close the pop-up message.
 *  - Restores the original video focus.
 ************************************************************************************/
void NxLauncher::ExpirePopupMessage()
{
	ui->messageFrame->Lower();
	NextVideoFocus();
}

/************************************************************************************\
 * D-AUDIO INTERFACE - RequestNotification
 *
 * Description
 *  - Request a notification.
 *  - The notification is at the top regardless of the video focus.
 ************************************************************************************/
void NxLauncher::RequestNotification(PopupMessage *psPopup)
{
	QString caller = FindCaller(2);
	QApplication::postEvent(m_spInstance, new NxNotificationEvent(psPopup, caller));
}

/************************************************************************************\
 * D-AUDIO INTERFACE - RequestExpireNotification
 *
 * Description
 *  - A notification is requested to expire.
 ************************************************************************************/
void NxLauncher::RequestExpireNotification()
{
	m_spInstance->ExpireNotification();
}

/************************************************************************************\
 * D-AUDIO INTERFACE Wrapper - ExpireNotification
 *
 * Description
 *  - Close the notification.
 ************************************************************************************/
void NxLauncher::ExpireNotification()
{
	ui->notificationBar->Lower();
}

/************************************************************************************\
 * D-AUDIO INTERFACE - RequestTerminate
 *
 * Description
 *  - The plug-in requests termination.
 ************************************************************************************/
void NxLauncher::RequestTerminate()
{
	QString requestor = FindCaller(2);
	if (m_spInstance)
	{
		m_spInstance->Terminate(requestor);
	}
}

/************************************************************************************\
 * D-AUDIO INTERFACE Wrapper - Terminate
 *
 * Description
 *  - Call the deInit function of the plugin that requested termination.
 *  - Then, it recovers the focus of the plugin requesting termination.
 ************************************************************************************/
void NxLauncher::Terminate(QString requestor)
{
	QString owner;
	bool bOk = false;

	if (m_PlugIns[requestor]->m_pdeInit)
	{
		m_PlugIns[requestor]->m_pdeInit();

		RemoveVideoFocus(requestor);

		if (owner == requestor)
		{
			if (m_VideoFocusQueue.size())
			{
				owner = m_VideoFocusQueue.first();
				if (owner != NX_LAUNCHER)
					m_PlugIns[owner]->m_pRequestVideoFocus(FocusType_Set, FocusPriority_Normal, &bOk);
			}
		}

		owner = m_AudioFocusQueue.size() ? m_AudioFocusQueue.first() : "";
		if (m_AudioFocusQueue.size())
			m_AudioFocusQueue.removeAll(requestor);
		if (owner == requestor)
		{
			if (m_AudioFocusQueue.size())
			{
				owner = m_AudioFocusQueue.first();
				m_PlugIns[owner]->m_pRequestAudioFocus(FocusType_Set, FocusPriority_Normal, &bOk);
			}
		}
	}
}

void NxLauncher::RequestVolume()
{
	QApplication::postEvent(m_spInstance, new NxVolumeControlEvent());
}

QVariantList NxLauncher::getPluginInfoList()
{
	QVariantList plugins;

	foreach (NxPluginInfo* psInfo, m_PlugIns)
	{
		plugins << qVariantFromValue((QObject *)psInfo);
	}

	return plugins;
}

bool NxLauncher::event(QEvent *event)
{
	switch ((int32_t)event->type()) {
	case E_NX_EVENT_KEY:
	{
		NxKeyEvent *e = static_cast<NxKeyEvent *>(event);
		KeyEvent(e);
		return true;
	}

	case E_NX_EVENT_POPUP_MESSAGE:
	{
		NxPopupMessageEvent *e = static_cast<NxPopupMessageEvent *>(event);
		PopupMessageEvent(e);
		return true;
	}

	case E_NX_EVENT_NOTIFICATION:
	{
		NxNotificationEvent *e = static_cast<NxNotificationEvent *>(event);
		NotificationEvent(e);
		return true;
	}

	case E_NX_EVENT_VOLUME_CONTROL:
	{
		NxVolumeControlEvent *e = static_cast<NxVolumeControlEvent *>(event);
		VolumeControlEvent(e);
		return true;
	}

	case E_NX_EVENT_TERMINATE:
	{
		m_Timer.stop();

		foreach (NxPluginInfo *psInfo, m_PlugIns) {
			if (psInfo->m_pIsInit && psInfo->m_pdeInit)
			{				
				bool bOk = false;
				psInfo->m_pIsInit(&bOk);

				if (bOk)
					psInfo->m_pdeInit();

				NXLOGI("[%s] dlclose %p 1 ", __FUNCTION__, psInfo->m_pHandle);
				dlclose(psInfo->m_pHandle);
				NXLOGI("[%s] dlclose %p 2 ", __FUNCTION__, psInfo->m_pHandle);
			}
		}

		exit(0);
		return true;
	}

	case QEvent::WindowActivate:
	{
		printf("NX_LAUNCHER SHOWN\n");
		break;
	}

	default:
		break;
	}
	return QDialog::event(event);
}

void NxLauncher::KeyEvent(NxKeyEvent* e)
{
	static uint32_t bluetoothMode = 0;	//	0 : Phone, 1 : Audio, 2 : Setting
	QString key;

	switch (e->m_iKey) {
	case DAUD_KEY_MODE_AUDIO:
		key = "NxAudioPlayer";
		break;

	case DAUD_KEY_MODE_VIDEO:
		key = "NxVideoPlayer";
		break;

	case DAUD_KEY_MODE_BLUETOOTH:
		switch (bluetoothMode)
		{
		case 0:	//	Phone
			key = "NxBTPhone";
			break;

		case 1:	//	Audio
			key = "NxBTAudio";
			break;

		case 2:	//	Settings
			key = "NxBTSettings";
			break;
		}
		bluetoothMode = (bluetoothMode+1) % 3;
		break;

	case DAUD_KEY_MODE_AVIN:
	case DAUD_KEY_MODE_3DAVM:
	case DAUD_KEY_MODE_3DAVM_CLOSE:
		break;

	default:
		NXLOGI("[%s] %d", Q_FUNC_INFO, e->m_iKey);
		break;
	}

	if (key.isEmpty())
	{
		NXLOGW("[%s] Unsupported key(%d)", __FUNCTION__, e->m_iKey);
		return;
	}

	Execute(key);
}

void NxLauncher::PopupMessageEvent(NxPopupMessageEvent *e)
{
#if 0
	QString owner = m_VideoFocusQueue.first();
	bool bOk = false;

	if (owner == NX_LAUNCHER)
		bOk = true;
	else
	{
		m_PlugIns[owner]->m_pRequestVideoFocusTransient(FocusPriority_Normal, &bOk);

		if (!bOk)
			NXLOGE("[%s] REQUEST VIDEO FOCUS TRANSIENT", __FUNCTION__);
	}

	if (bOk)
	{
#endif
		m_VideoFocusQueue.removeAll(NX_LAUNCHER_P);
		m_VideoFocusQueue.push_front(NX_LAUNCHER_P);

		ui->messageFrame->SetRequestor(e->m_Requestor);
		ui->messageFrame->SetButtonVisibility(e->m_eButtonVisibility);
		ui->messageFrame->SetButonStyleSheet(ButtonType_Ok, e->m_ButtonStylesheet[ButtonType_Ok]);
		ui->messageFrame->SetButonStyleSheet(ButtonType_Cancel, e->m_ButtonStylesheet[ButtonType_Cancel]);
		ui->messageFrame->SetTimeout(e->m_uiTimeout);
		ui->messageFrame->SetMessageTitle(e->m_MsgTitle);
		ui->messageFrame->SetMessageBody(e->m_MsgBody);
		ui->messageFrame->Raise();
#if 0
	}
#endif
}

void NxLauncher::NotificationEvent(NxNotificationEvent *e)
{
	ui->notificationBar->SetRequestor(e->m_Requestor);
	ui->notificationBar->SetButtonVisibility(e->m_eButtonVisibility);
	ui->notificationBar->SetButonStyleSheet(ButtonType_Ok, e->m_ButtonStylesheet[ButtonType_Ok]);
	ui->notificationBar->SetButonStyleSheet(ButtonType_Cancel, e->m_ButtonStylesheet[ButtonType_Cancel]);
	ui->notificationBar->SetTimeout(e->m_uiTimeout);
	ui->notificationBar->SetMessageTitle(e->m_MsgTitle);
	ui->notificationBar->SetMessageBody(e->m_MsgBody);
	ui->notificationBar->Raise();
}

void NxLauncher::VolumeControlEvent(NxVolumeControlEvent *)
{
	ui->volumeBar->SetValue(ui->statusBar->GetVolume());
	ui->volumeBar->Raise();
}

void NxLauncher::Execute(QString plugin)
{
	bool bOk = false;
	if (m_PlugIns.find(plugin) == m_PlugIns.end())
	{
		NXLOGE("[%s] Plug-in : %s <NG>", Q_FUNC_INFO, plugin.toStdString().c_str());
		return;
	}

	if (m_PlugIns[plugin]->m_pIsInit)
		m_PlugIns[plugin]->m_pIsInit(&bOk);

	if (!bOk)
	{
		if (m_PlugIns[plugin]->m_pInit)
		{
			m_PlugIns[plugin]->m_pInit(this, "");
		}
	}
	else
	{
		if (m_PlugIns[plugin]->m_pRequestVideoFocus)
		{
			bool bOk = false;
//				CollectVideoFocus();
			m_PlugIns[plugin]->m_pRequestVideoFocus(FocusType_Set, FocusPriority_Normal, &bOk);
			if (!bOk)
			{
				NXLOGE("[%s] REQUEST VIDEO FOCUS from <%s>", __FUNCTION__, plugin.toStdString().c_str());
				return;
			}

			AddVideoFocus(plugin);
		}
	}
}

void NxLauncher::slotExecute(QString plugin)
{
	int etx = plugin.lastIndexOf("/");
	int stx = plugin.lastIndexOf("/", etx-1);

	if (stx > -1 && etx > -1)
	{
		QString key = plugin.mid(stx+1, etx-stx-1);
		Execute(key);
	}
}

void NxLauncher::slotPlugInUpdated(QString path)
{
	NXLOGI("[%s] path = %s", __FUNCTION__, path.toStdString().c_str());
	int etx = path.lastIndexOf("/");
	int stx = path.lastIndexOf("/", etx-1);

	if (stx > -1 && etx > -1)
	{
		QString key = path.mid(stx+1, etx-stx-1);

		if (m_PlugIns.find(key) == m_PlugIns.end())
			return;

		if (m_PlugIns[key]->getType().toLower() == "service" && m_PlugIns[key]->getEnabled())
		{
			bool bOk = false;

			if (!(m_PlugIns[key]->m_pInit) || !(m_PlugIns[key]->m_pIsInit))
				return;

			m_PlugIns[key]->m_pIsInit(&bOk);

			if (!bOk)
				m_PlugIns[key]->m_pInit(this, "");
		}
		else
		{
#ifndef CONFIG_USE_NO_QML
			QMetaObject::invokeMethod(m_pLauncherWidget->rootObject(), "activeChanged");
#else
			// fill
			foreach (NxPluginInfo *pInfo, m_PlugIns) {

			}
#endif
		}
	}
}

void NxLauncher::slotSetVolume(int value)
{
	ui->statusBar->SetVolume(value);
}

/************************************************************************************\
 * D-AUDIO INTERFACE WRAPPER
 *
 * Description
 *  - Try to set the launcher to the top level.
 ************************************************************************************/
void NxLauncher::LauncherShow(bool *bOk, bool bRequireRequestFocus)
{
	*bOk = !bRequireRequestFocus;
	if (bRequireRequestFocus)
	{
		QString owner = m_VideoFocusQueue.first();
		if (m_PlugIns.find(owner) == m_PlugIns.end())
		{
			if (owner != NX_LAUNCHER)
				NXLOGE("[%s] The plug-in '%s' does not exist.", __FUNCTION__, owner.toStdString().c_str());
			return;
		}

		if (m_PlugIns[owner]->m_pRequestVideoFocus)
			m_PlugIns[owner]->m_pRequestVideoFocus(FocusType_Get, FocusPriority_Normal, bOk);
	}

	if (!*bOk)
	{
		NXLOGE("[%s] ", __FUNCTION__);
		return;
	}

	if (AddVideoFocus(NX_LAUNCHER))
	{
		ui->launcher->raise();
	}
}

void NxLauncher::UpdateTitle(QString owner)
{
	if (owner == NX_LAUNCHER)
		ui->statusBar->SetTitleName("Home");
	else
		ui->statusBar->SetTitleName(m_PlugIns[owner]->getTitle());
}

void NxLauncher::RemoveVideoFocus(QString requestor)
{
	QString prev, curr;
	prev = m_VideoFocusQueue.first();
	m_VideoFocusQueue.removeAll(requestor);
	curr = m_VideoFocusQueue.first();

	if (prev != curr)
	{
		UpdateTitle(curr);
	}
}

bool NxLauncher::AddVideoFocus(QString owner)
{
	int i = 0;

	if (m_VideoFocusQueue.first() == owner)
		return false;

	i = m_VideoFocusQueue.lastIndexOf(owner);
	if (i < 0)
	{
		m_VideoFocusQueue.push_front(owner);
	}
	else
	{
		for ( ; i > 0; --i)
		{
			m_VideoFocusQueue.swap(i, i-1);
		}
	}

	if (owner == NX_LAUNCHER)
		ui->statusBar->SetTitleName("Home");
	else
		ui->statusBar->SetTitleName(m_PlugIns[owner]->getTitle());

	return true;
}

void NxLauncher::NextVideoFocus()
{
	QString owner;

	owner = m_VideoFocusQueue.first();
	if (owner == NX_LAUNCHER_P)
		m_VideoFocusQueue.removeAll(NX_LAUNCHER_P);

	owner = m_VideoFocusQueue.first();
	if (owner != NX_LAUNCHER)
	{
		bool bOk = false;

		if (m_PlugIns[owner]->m_pIsInit && m_PlugIns[owner]->m_pRequestVideoFocus)
			m_PlugIns[owner]->m_pRequestVideoFocus(FocusType_Set, FocusPriority_Normal, &bOk);

		if (!bOk)
			NXLOGE("[%s]", __FUNCTION__);
	}
}

void NxLauncher::slotTimer()
{
	m_Timer.stop();

	QString msg = "|";
	for (int i = 0; i < m_AudioFocusQueue.size(); ++i)
	{
		msg += m_AudioFocusQueue.at(i) + "|";
	}

	NXLOGI("[%s] AUDIO QUEUE < %s >", __FUNCTION__, msg.toStdString().c_str());

	msg = "|";
	for (int i = 0; i < m_VideoFocusQueue.size(); ++i)
	{
		msg += m_VideoFocusQueue.at(i) + "|";
	}

	NXLOGI("[%s] VIDEO QUEUE < %s >", __FUNCTION__, msg.toStdString().c_str());

	m_Timer.start(5000);
}

/************************************************************************************\
 * Qt Slot Functions
 *
 * Description
 *  - This is the Qt Slot function that occurs when the 'Ok' Button in the
 *    'Message Widget' or 'Notification Bar' is clicked.
 ************************************************************************************/
void NxLauncher::slotPopupMessageAccept()
{
	NXLOGI("[%s]", __FUNCTION__);
	QString requestor = ui->messageFrame->GetRequestor();
	bool bOk = false;

	if (!(m_PlugIns[requestor]->m_pIsInit) || !(m_PlugIns[requestor]->m_pPopupMessageResponse))
	{
		NXLOGE("[%s] The D-Audio Interface has a missing function.", __FUNCTION__);
		NXLOGE("[%s] Make sure the IsInit and PopupMessageResponse functions are defined.", __FUNCTION__);
		return;
	}

	m_PlugIns[requestor]->m_pIsInit(&bOk);

	if (!bOk)
	{
		NXLOGW("[%s] <%s> does not exists.", __FUNCTION__, requestor.toStdString().c_str());
		return;
	}

	m_PlugIns[requestor]->m_pPopupMessageResponse(true);

	NextVideoFocus();
}

/************************************************************************************\
 * Qt Slot Functions
 *
 * Description
 *  - This is the Qt Slot function that occurs when the 'Cancel' Button in the
 *    'Message Widget' or 'Notification Bar' is clicked.
 ************************************************************************************/
void NxLauncher::slotPopupMessageReject()
{
	NXLOGI("[%s]", __FUNCTION__);
	QString requestor = ui->messageFrame->GetRequestor();
	bool bOk = false;

	if (!(m_PlugIns[requestor]->m_pIsInit) || !(m_PlugIns[requestor]->m_pPopupMessageResponse))
	{
		NXLOGE("[%s] The D-Audio Interface has a missing function.", __FUNCTION__);
		NXLOGE("[%s] Make sure the IsInit and PopupMessageResponse functions are defined.", __FUNCTION__);
		return;
	}

	m_PlugIns[requestor]->m_pIsInit(&bOk);

	if (!bOk)
	{
		NXLOGW("[%s] <%s> does not exists.", __FUNCTION__, requestor.toStdString().c_str());
		return;
	}

	m_PlugIns[requestor]->m_pPopupMessageResponse(false);

	NextVideoFocus();
}

void NxLauncher::slotNotificationAccept()
{
	NXLOGI("[%s]", __FUNCTION__);
	QString requestor = ui->notificationBar->GetRequestor();
	bool bOk = false;

	if (!(m_PlugIns[requestor]->m_pIsInit) || !(m_PlugIns[requestor]->m_pNotificationResponse))
	{
		NXLOGE("[%s] The D-Audio Interface has a missing function.", __FUNCTION__);
		NXLOGE("[%s] Make sure the IsInit and NotificationResponse functions are defined.", __FUNCTION__);
		return;
	}

	m_PlugIns[requestor]->m_pIsInit(&bOk);

	if (!bOk)
	{
		NXLOGW("[%s] <%s> does not exists.", __FUNCTION__, requestor.toStdString().c_str());
		return;
	}

	m_PlugIns[requestor]->m_pNotificationResponse(true);
}

void NxLauncher::slotNotificationReject()
{
	NXLOGI("[%s]", __FUNCTION__);
	QString requestor = ui->notificationBar->GetRequestor();
	bool bOk = false;

	if (!(m_PlugIns[requestor]->m_pIsInit) || !(m_PlugIns[requestor]->m_pNotificationResponse))
	{
		NXLOGE("[%s] The D-Audio Interface has a missing function.", __FUNCTION__);
		NXLOGE("[%s] Make sure the IsInit and NotificationResponse functions are defined.", __FUNCTION__);
		return;
	}

	m_PlugIns[requestor]->m_pIsInit(&bOk);

	if (!bOk)
	{
		NXLOGW("[%s] <%s> does not exists.", __FUNCTION__, requestor.toStdString().c_str());
		return;
	}

	m_PlugIns[requestor]->m_pNotificationResponse(false);
}

void NxLauncher::slotCommandTimer()
{
	QDir dir("/home/root");
	QFileInfoList entries = dir.entryInfoList(QStringList() << "cmd", QDir::Files);

	foreach (QFileInfo entry, entries)
	{
		QFile f(entry.filePath());
		if (f.open(QFile::ReadOnly))
		{
			QString data = f.readAll();
			// start;NxCarplay;NxAndroidAuto
			QStringList tokens = data.split(";");
			if (tokens[0].toLower() == "start")
			{
				for (int i = 1; i < tokens.size(); ++i)
				{
					Execute(tokens[i].trimmed());
				}
			}

			f.close();

			f.remove();
		}
	}
}

#ifdef CONFIG_USE_NO_QML
void NxLauncher::onExecute(NxPluginInfo* pInfo)
{
	QString plugin = pInfo->getPath() + "/" + pInfo->getName();
	int etx = plugin.lastIndexOf("/");
	int stx = plugin.lastIndexOf("/", etx-1);

	if (stx > -1 && etx > -1)
	{
		QString key = plugin.mid(stx+1, etx-stx-1);
		Execute(key);
	}
}

void NxLauncher::onPrevPageButtonClicked()
{
	m_pPageStackFrame->setPage(m_pPageStackFrame->currentPage()-1);
}

void NxLauncher::onNextPageButtonClicked()
{
	m_pPageStackFrame->setPage(m_pPageStackFrame->currentPage()+1);
}
#endif

void NxLauncher::resizeEvent(QResizeEvent *)
{
	int w = width();
	int h = height();

	if (w == DEFAULT_WIDTH && h == DEFAULT_HEIGHT)
	{
		return;
	}

	ui->launcher->setFixedSize(w, h);
	ui->statusBar->setFixedSize(w, h * 0.1);
	ui->notificationBar->setFixedSize(w, h * 0.1);
#ifndef CONFIG_USE_NO_QML
	m_pLauncherWidget->setGeometry(0, ui->statusBar->height(), w, h * 0.9);
#endif
}
#ifdef CONFIG_USE_NO_QML
void NxLauncher::slotResizeItemDone()
{
	if (m_pPageStackFrame->GetPageCount() > 1)
	{
		m_pPrevPageButton->show();
		m_pNextPageButton->show();
	}
}
#endif
