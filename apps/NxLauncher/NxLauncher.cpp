#include "NxLauncher.h"
#include "ui_NxLauncher.h"

#include <QQuickView>
#include <QObject>
#include <QWidget>
#include <QQuickItem>
#include <QQmlProperty>
#include <QProcess>
#include <QDirIterator>
#include <QQmlContext>
#include <QQuickWidget>

#include <execinfo.h>

#include <DAudioKeyDef.h>
#include "NX_DAudioUtils.h"

#ifdef Q_PROCESSOR_X86
//#define NX_APP_PATH         "/home/doriya/working/solution/display-audio/bin"
#define NX_APP_PATH	"/home/daegeun/workspace/daudio-5.6/displayaudio/result/nexell/daudio"
#else
// #define NX_APP_PATH         "/podo/apps"
#define NX_APP_PATH         "/nexell/daudio"
#endif

// for dlopen
#include <dlfcn.h>

#define NX_LAUNCHER "NxLauncher"

#define LOG_TAG "[NxLauncher]"
#include <NX_Log.h>

NxLauncher* NxLauncher::m_spInstance = NULL;
QQueue<QString> NxLauncher::m_AudioFocusQueue = QQueue<QString>();
QQueue<QString> NxLauncher::m_VideoFocusQueue = QQueue<QString>();

NxLauncher::NxLauncher(QWidget *parent) :
	QDialog(parent, Qt::FramelessWindowHint),
	ui(new Ui::NxLauncher)
{
	ui->setupUi(this);

	m_spInstance = this;
	installEventFilter( this );

	// scan applications
	m_pPackageManager = new NxPackageScanner(NX_APP_PATH, &m_PlugIns);
	m_pProcessManager = new NxProcessManager( this );

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
		// show for gui application
		psInfo->m_pShow = (void (*)())dlsym(psInfo->m_pHandle, "Show");
		// hide for gui application
		psInfo->m_pHide = (void (*)())dlsym(psInfo->m_pHandle, "Hide");
		// raise for gui application - change z-order
		psInfo->m_pRaise = (void (*)())dlsym(psInfo->m_pHandle, "Raise");
		// lower for gui application - change z-order
		psInfo->m_pLower = (void (*)())dlsym(psInfo->m_pHandle, "Lower");
		// launcher topmost
		psInfo->m_pRegisterLauncherShow = (void (*)(void(*)(bool*)))dlsym(psInfo->m_pHandle, "RegisterRequestLauncherShow");
		// ???
		psInfo->m_pRegisterShow = (void (*)(void(*)()))dlsym(psInfo->m_pHandle, "RegisterShow");
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
		psInfo->m_pRegisterRequestPluginTerminate = (void (*)(void (*)(const char*)))dlsym(psInfo->m_pHandle, "RegisterRequestPlugInRun");
		// send message
		psInfo->m_pSendMessage = (void (*)(const char*, int32_t))dlsym(psInfo->m_pHandle, "SendMessage");
		// register callback for request send message
		psInfo->m_pRegisterRequestMessage = (void (*)(void (*)(const char*, const char*, int32_t)))dlsym(psInfo->m_pHandle, "RegisterRequestSendMessage");
		// popup message
		psInfo->m_pRegisterRequestPopupMessage = (void (*)(void (*)(PopupMessage*, bool*)))dlsym(psInfo->m_pHandle, "RegisterRequestPopupMessage");
		psInfo->m_pRegisterRequestExpirePopupMessage = (void (*)(void (*)()))dlsym(psInfo->m_pHandle, "RegisterRequestExpirePopupMessage");
		psInfo->m_pPopupMessageResponse = (void (*)(bool))dlsym(psInfo->m_pHandle, "PopupMessageResponse");
		// terminate
		psInfo->m_pRegisterRequestTerminate = (void (*)(void (*)(void)))dlsym(psInfo->m_pHandle, "RegisterRequestTerminate");

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

	NXLOGI("[%s] step3", __FUNCTION__);
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
		// message
		if (psInfo->m_pRegisterRequestMessage)
			psInfo->m_pRegisterRequestMessage(RequestSendMessage);
		// run/terminate from other plugin
		if (psInfo->m_pRegisterRequestPluginRun)
			psInfo->m_pRegisterRequestPluginRun(RequestPlugInRun);
		if (psInfo->m_pRegisterRequestPluginTerminate)
			psInfo->m_pRegisterRequestPluginTerminate(RequestPlugInTerminate);
		if (psInfo->m_pRegisterRequestTerminate)
			psInfo->m_pRegisterRequestTerminate(RequestTerminate);
	}

	foreach (NxPluginInfo *psInfo, m_PlugIns) {
		if (psInfo->getType().toLower() == "service" && psInfo->getEnabled())
		{
			if (psInfo->m_pInit)
				psInfo->m_pInit(this, "");
		}
	}

	m_VideoFocusQueue.push_front("NxLauncher");

	// set application name
	ui->statusBar->SetTitleName("Home");

	// binding between qml and widget
	ui->launcherWidget->setSource(QUrl("qrc:/qml/NxLauncher.qml"));
	ui->launcherWidget->rootContext()->setContextProperty("gui", this);
	if (ui->launcherWidget->rootObject())
	{
		connect(ui->launcherWidget->rootObject(), SIGNAL(launchProgram(QString)), this, SLOT(slotExecute(QString)));
	}

	connect(ui->messageFrame, SIGNAL(signalOk()), this, SLOT(slotAccept()));
	connect(ui->messageFrame, SIGNAL(signalCancel()), this, SLOT(slotReject()));

	connect(&m_Timer, SIGNAL(timeout()), this, SLOT(slotTimer()));
	m_Timer.start(5000);
}

NxLauncher::~NxLauncher()
{
	delete ui;
}

void NxLauncher::RequestLauncherShow(bool *bOk)
{
	NXLOGI("[%s]", __FUNCTION__);
	if (m_spInstance)
		m_spInstance->LauncherShow(bOk);
}

void NxLauncher::RequestShow()
{

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


	NXLOGI("[%s] %s -> %s %s", __FUNCTION__, from.toStdString().c_str(), pDst, pMsg);

	if (m_spInstance->m_PlugIns.find(key) != m_spInstance->m_PlugIns.end())
	{
		NXLOGI("[%s] %s -> %s %d %d ", __FUNCTION__, from.toStdString().c_str(), pDst, !!m_spInstance->m_PlugIns[key]->m_pIsInit, !!m_spInstance->m_PlugIns[key]->m_pSendMessage);
		if (m_spInstance->m_PlugIns[key]->m_pIsInit && m_spInstance->m_PlugIns[key]->m_pSendMessage)
		{
			bool bOk = false;
			m_spInstance->m_PlugIns[key]->m_pIsInit(&bOk);
			NXLOGI("[%s] key = %s <%s>", __FUNCTION__, key.toStdString().c_str(), bOk ? "OK" : "NG");
			if (bOk)
				m_spInstance->m_PlugIns[key]->m_pSendMessage(pMsg, iMsgSize);

			NXLOGI("[%s] %s -> %s %s <%s>", __FUNCTION__, from.toStdString().c_str(), pDst, pMsg, bOk ? "OK" : "NG");
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
	NxLauncher *p = m_spInstance;
	QString key = QString::fromLatin1(pPlugin);
	bool bOk = false;

	NXLOGI("[%s] plugin(%s)", __FUNCTION__, pPlugin);

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

	if (!(m_spInstance->m_PlugIns[owner]->m_pIsInit) || !(m_spInstance->m_PlugIns[owner]->m_pRequestAudioFocus))
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

		NXLOGI("[%s] isInit ? <%s>", __FUNCTION__, bOk ? "OK" : "NG");

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

void NxLauncher::RequestAudioFocusTransient(FocusPriority ePriority, bool *bOk)
{
	QString caller = FindCaller(2);
	QString owner = m_AudioFocusQueue.size() ? m_AudioFocusQueue.first() : "";

	NXLOGI("[%s] <TRY> owner(%s), caller(%s)", __FUNCTION__, owner.toStdString().c_str(), caller.toStdString().c_str());

	if (!(m_spInstance->m_PlugIns[owner]->m_pIsInit) || !(m_spInstance->m_PlugIns[owner]->m_pRequestAudioFocusTransient))
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

	NXLOGI("[%s] <DONE> owner(%s)", __FUNCTION__, m_AudioFocusQueue.first().toStdString().c_str());
}

// Video Focus Management
void NxLauncher::RequestVideoFocus(FocusPriority ePriority, bool *bOk)
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

		NXLOGI("[%s] isInit ? <%s>", __FUNCTION__, bOk ? "OK" : "NG");

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
		m_VideoFocusQueue.removeAll(caller);
		m_VideoFocusQueue.push_front(caller);
	}
}

void NxLauncher::RequestVideoFocusTransient(FocusPriority ePriority, bool *bOk)
{
	QString caller = FindCaller(2);
	QString owner = m_VideoFocusQueue.size() ? m_VideoFocusQueue.first() : "";

	NXLOGI("[%s] <TRY> owner(%s), caller(%s)", __FUNCTION__, owner.toStdString().c_str(), caller.toStdString().c_str());


	if (!(m_spInstance->m_PlugIns[owner]->m_pIsInit) || !(m_spInstance->m_PlugIns[owner]->m_pRequestVideoFocusTransient))
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
		m_VideoFocusQueue.removeAll(caller);
		m_VideoFocusQueue.push_front(caller);
	}
}

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
		m_VideoFocusQueue.pop_front();

		// 2. check whether the next owner exists.
		if (m_VideoFocusQueue.size())
		{
			curr = m_VideoFocusQueue.first();
			if (prev != curr)
			{
				// 2.1. pass the focus to the next owner.
				bool bOk = false;
				m_spInstance->m_PlugIns[curr]->m_pRequestVideoFocus(FocusType_Set, FocusPriority_Normal, &bOk);
			}
		}
	}

	NXLOGI("[%s] <DONE> owner(%s)", __FUNCTION__, m_AudioFocusQueue.first().toStdString().c_str());
}

void NxLauncher::RequestPopupMessage(PopupMessage *psPopup, bool *bOk)
{
	QString caller = FindCaller(2);

	*bOk = true;
	QApplication::postEvent(m_spInstance, new NxPopupMessageEvent(psPopup, caller));
}

void NxLauncher::RequestExpirePopupMessage()
{
	m_spInstance->ExpirePopupMessage();
}

void NxLauncher::ExpirePopupMessage()
{
	ui->messageFrame->lower();
	NextVideoFocus();
}

void NxLauncher::RequestTerminate()
{
	QString requestor = FindCaller(2);
	NXLOGI("[%s] !!!", __FUNCTION__);
	if (m_spInstance)
	{
		NXLOGI("[%s] @@@", __FUNCTION__);
		m_spInstance->Terminate(requestor);
	}
}

void NxLauncher::Terminate(QString requestor)
{
	QString owner;
	bool bOk = false;

	if (m_PlugIns[requestor]->m_pdeInit)
	{
		m_PlugIns[requestor]->m_pdeInit();

		owner = m_VideoFocusQueue.size() ? m_VideoFocusQueue.first() : "NxLauncher";
		m_VideoFocusQueue.removeAll(owner);
		if (owner == requestor)
		{
			if (m_VideoFocusQueue.size())
			{
				owner = m_VideoFocusQueue.first();
				if (owner != "NxLauncher")
					m_PlugIns[owner]->m_pRequestVideoFocus(FocusType_Get, FocusPriority_Normal, &bOk);
			}
		}

		owner = m_AudioFocusQueue.size() ? m_AudioFocusQueue.first() : "NxLauncher";
		m_AudioFocusQueue.removeAll(owner);
		if (owner == requestor)
		{
			if (m_AudioFocusQueue.size())
			{
				owner = m_AudioFocusQueue.first();
				m_PlugIns[owner]->m_pRequestAudioFocus(FocusType_Get, FocusPriority_Normal, &bOk);
			}
		}
	}
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

	default:
		break;
	}
	return QDialog::event(event);
}

void NxLauncher::KeyEvent(NxKeyEvent* e)
{
	static uint32_t bluetoothMode = 0;	//	0 : Phone, 1 : Audio, 2 : Setting

	switch (e->m_iKey) {
	case DAUD_KEY_MODE_AUDIO:
		m_pProcessManager->execute("/nexell/daudio/NxAudioPlayer/NxAudioPlayer");
		break;
	case DAUD_KEY_MODE_VIDEO:
		m_pProcessManager->execute("/nexell/daudio/NxVideoPlayer/NxVideoPlayer");
		break;
	case DAUD_KEY_MODE_BLUETOOTH:
		switch (bluetoothMode)
		{
		case 0:	//	Phone
			m_pProcessManager->execute("/nexell/daudio/NxBTPhoneR/NxBTPhoneR");
			break;
		case 1:	//	Audio
			m_pProcessManager->execute("/nexell/daudio/NxBTAudioR/NxBTAudioR");
			break;
		case 2:	//	Settings
			m_pProcessManager->execute("/nexell/daudio/NxBTSettingsR/NxBTSettingsR");
			break;
		}
		bluetoothMode = (bluetoothMode+1) % 3;
		break;
	case DAUD_KEY_MODE_AVIN:
	case DAUD_KEY_MODE_3DAVM:
	case DAUD_KEY_MODE_3DAVM_CLOSE:
		break;

	default: break;
	}
}

void NxLauncher::PopupMessageEvent(NxPopupMessageEvent *e)
{
	QString owner = m_VideoFocusQueue.size() ? m_VideoFocusQueue.first() : "";
	bool bOk = false;

	if (owner.isEmpty())
		bOk = true;
	else
	{
		m_PlugIns[owner]->m_pRequestVideoFocusTransient(FocusPriority_Normal, &bOk);

		if (!bOk)
			NXLOGE("[%s] REQUEST VIDEO FOCUS TRANSIENT", __FUNCTION__);
	}

	if (bOk)
	{
		// VideoFocus's owner = Launcher
		m_VideoFocusQueue.push_front("NxLauncher");

		ui->messageFrame->SetRequestor(e->m_Requestor);
		ui->messageFrame->SetButtonVisibility(e->m_eButtonVisibility);
		ui->messageFrame->SetButtonLocation(e->m_eButtonLocation);
		ui->messageFrame->SetButonStyleSheet(ButtonType_Ok, e->m_ButtonStylesheet[ButtonType_Ok]);
		ui->messageFrame->SetButonStyleSheet(ButtonType_Cancel, e->m_ButtonStylesheet[ButtonType_Cancel]);
		ui->messageFrame->SetTimeout(e->m_uiTimeout);
		ui->messageFrame->SetMessageTitle(e->m_MsgTitle);
		ui->messageFrame->SetMessageBody(e->m_MsgBody);
		ui->messageFrame->raise();
	}
}

void NxLauncher::slotExecute(QString plugin)
{
	int etx = plugin.lastIndexOf("/");
	int stx = plugin.lastIndexOf("/", etx-1);

	if (stx > -1 && etx > -1)
	{
		QString key = plugin.mid(stx+1, etx-stx-1);
		bool bOk = false;
		if (m_PlugIns.find(key) == m_PlugIns.end())
		{
			NXLOGE("[%s] Plug-in <NG>", Q_FUNC_INFO);
			return;
		}

		if (m_PlugIns[key]->m_pIsInit)
			m_PlugIns[key]->m_pIsInit(&bOk);

		if (!bOk)
		{
			if (m_PlugIns[key]->m_pInit)
			{
				m_PlugIns[key]->m_pInit(this, "");
			}
		}
		else
		{
			if (m_PlugIns[key]->m_pRequestVideoFocus)
			{
				bool bOk = false;
//				CollectVideoFocus();
				m_PlugIns[key]->m_pRequestVideoFocus(FocusType_Set, FocusPriority_Normal, &bOk);
				if (!bOk)
				{
					NXLOGE("[%s] REQUEST VIDEO FOCUS from <%s>", __FUNCTION__, key.toStdString().c_str());
					return;
				}

				m_VideoFocusQueue.removeAll(key);
				m_VideoFocusQueue.push_front(key);
			}
		}
	}
}

/************************************************************************************\
 * D-AUDIO INTERFACE WRAPPER
 *
 * Description
 *  - Try to set the launcher to the top level.
 ************************************************************************************/
void NxLauncher::LauncherShow(bool *bOk)
{
	QString owner = m_VideoFocusQueue.first();

	if (m_PlugIns[owner]->m_pRequestVideoFocus)
		m_PlugIns[owner]->m_pRequestVideoFocus(FocusType_Get, FocusPriority_Normal, bOk);

	if (!*bOk)
	{
		NXLOGE("[%s] ", __FUNCTION__);
		return;
	}

	m_VideoFocusQueue.removeAll(NX_LAUNCHER);
	m_VideoFocusQueue.push_front(NX_LAUNCHER);
	ui->launcher->raise();
}

/************************************************************************************\
 * D-AUDIO INTERFACE WRAPPER
 *
 * Description
 *  - Collects the focus from owner of Video focus
 ************************************************************************************/
bool NxLauncher::CollectVideoFocus()
{
	QString owner = m_VideoFocusQueue.size() ? m_VideoFocusQueue.first() : "";
	bool bOk = false;

	if (owner.isEmpty() || owner == "NxLauncher")
		bOk = true;
	else if (m_PlugIns[owner]->m_pIsInit && m_PlugIns[owner]->m_pRequestVideoFocus)
	{
		m_PlugIns[owner]->m_pIsInit(&bOk);

		//
		if (bOk)
		{
			m_PlugIns[owner]->m_pRequestVideoFocus(FocusType_Get, FocusPriority_Normal, &bOk);
		}

		if (!bOk)
			NXLOGE("[%s] REQUEST VIDEO FOCUS from %s", __FUNCTION__, owner.toStdString().c_str());
	}

	if (bOk && m_VideoFocusQueue.size())
		m_VideoFocusQueue.pop_front();

	return bOk;
}

void NxLauncher::NextVideoFocus()
{
	if (m_VideoFocusQueue.size() == 0)
		return;

	m_VideoFocusQueue.pop_front();
	if (m_VideoFocusQueue.size())
	{
		QString key = m_VideoFocusQueue.first();
		bool bOk = false;

		if (m_PlugIns[key]->m_pIsInit && m_PlugIns[key]->m_pRequestVideoFocus)
			m_PlugIns[key]->m_pRequestVideoFocus(FocusType_Set, FocusPriority_Normal, &bOk);

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
void NxLauncher::slotAccept()
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
void NxLauncher::slotReject()
{
	NXLOGI("[%s]", __FUNCTION__);
	QString requestor = ui->messageFrame->GetRequestor();
	bool bOk = false;

	qDebug() << __FUNCTION__ << requestor;

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
