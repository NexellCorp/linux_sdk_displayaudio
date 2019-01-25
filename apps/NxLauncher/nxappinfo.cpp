#include "nxappinfo.h"

NxPluginInfo::NxPluginInfo() : QObject()
{
	m_pHandle = NULL;

	// plugin function call interface
	//
	m_pInit = NULL;
	m_pIsInit = NULL;
	m_pdeInit = NULL;
	//
	m_pRequestAudioFocus = NULL;
	m_pRequestAudioFocusTransient = NULL;
	//
	m_pRequestVideoFocus = NULL;
	m_pRequestVideoFocusTransient = NULL;
	//
	m_pSendMessage = NULL;

	// register callback functions
	//
	m_pRegisterLauncherShow = NULL;
	m_pRegisterRequestTerminate = NULL;
	//
	m_pRegisterRequestAudioFocus = NULL;
	m_pRegisterRequestAudioFocusTransient = NULL;
	m_pRegisterRequestAudioFocusLoss = NULL;
	//
	m_pRegisterRequestVideoFocus = NULL;
	m_pRegisterRequestVideoFocusTransient = NULL;
	m_pRegisterRequestVideoFocusLoss = NULL;
	//
	m_pRegisterRequestPluginRun = NULL;
	m_pRegisterRequestPluginTerminate = NULL;
	m_pRegisterRequestPluginIsRunning = NULL;
	//
	m_pRegisterRequestMessage = NULL;
	//
	m_pRegisterRequestPopupMessage = NULL;
	m_pRegisterRequestExpirePopupMessage = NULL;
	m_pPopupMessageResponse = NULL;
	//
	m_pRegisterRequestNotification = NULL;
	m_pRegisterRequestExpireNotification = NULL;
	m_pNotificationResponse = NULL;
	// Volume
	m_pRegisterRequestVolume = NULL;
	// Media Event
	m_pMediaEventChanged = NULL;

	m_pBackButtonClicked = NULL;
}

QString NxPluginInfo::getType() const
{
	return m_szType;
}

void NxPluginInfo::setType(const QString& szType)
{
	m_szType = szType;
}

QString NxPluginInfo::getVersion() const
{
	return m_szVersion;
}

void NxPluginInfo::setVersion(const QString& szVersion)
{
	m_szVersion = szVersion;
}

QString NxPluginInfo::getName() const
{
	return m_szName;
}

void NxPluginInfo::setName(const QString& szName)
{
	m_szName = szName;
}

QString NxPluginInfo::getComment() const
{
	return m_szComment;
}

void NxPluginInfo::setComment(const QString& szComment)
{
	m_szComment = szComment;
}

QString NxPluginInfo::getIcon() const
{
	return m_szIcon;
}

void NxPluginInfo::setIcon(const QString& szIcon)
{
	m_szIcon = szIcon;
}

QString NxPluginInfo::getDisabledIcon() const
{
	return m_szIconForDisabled;
}

void NxPluginInfo::setDisabledIcon(const QString& szIcon)
{
	m_szIconForDisabled = szIcon;
}

QString NxPluginInfo::getTryExec() const
{
	return m_szTryExec;
}

void NxPluginInfo::setTryExec(const QString szTryExec)
{
	m_szTryExec = szTryExec;
}

QString NxPluginInfo::getExec() const
{
	return m_szExec;
}

void NxPluginInfo::setExec(const QString szExec)
{
	m_szExec = szExec;
}

QString NxPluginInfo::getPath() const
{
	return m_szPath;
}

void NxPluginInfo::setPath(const QString szPath)
{
	m_szPath = szPath;
}

bool NxPluginInfo::getEnabled()
{
	return m_bEnabled;
}

void NxPluginInfo::setEnabled(bool enabled)
{
	m_bEnabled = enabled;
}

QString NxPluginInfo::getTitle()
{
	return m_szTitle;
}

void NxPluginInfo::setTitle(QString szTitle)
{
	m_szTitle = szTitle;
}

bool NxPluginInfo::getAutoStart()
{
	return m_bAutoStart;
}

void NxPluginInfo::setAutoStart(bool on)
{
	m_bAutoStart = on;
}

bool NxPluginInfo::getVisible()
{
	return m_bVisible;
}

void NxPluginInfo::setVisible(bool visible)
{
	m_bVisible = visible;
}
