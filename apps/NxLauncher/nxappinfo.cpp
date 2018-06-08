#include "nxappinfo.h"

NxAppInfo::NxAppInfo() : QObject()
{

}

QString NxAppInfo::getType() const
{
	return m_szType;
}

void NxAppInfo::setType(const QString& szType)
{
	m_szType = szType;
}

QString NxAppInfo::getVersion() const
{
	return m_szVersion;
}

void NxAppInfo::setVersion(const QString& szVersion)
{
	m_szVersion = szVersion;
}

QString NxAppInfo::getName() const
{
	return m_szName;
}

void NxAppInfo::setName(const QString& szName)
{
	m_szName = szName;
}

QString NxAppInfo::getComment() const
{
	return m_szComment;
}

void NxAppInfo::setComment(const QString& szComment)
{
	m_szComment = szComment;
}

QString NxAppInfo::getIcon() const
{
	return m_szIcon;
}

void NxAppInfo::setIcon(const QString& szIcon)
{
	m_szIcon = szIcon;
}

QString NxAppInfo::getDisabledIcon() const
{
	return m_szIconForDisabled;
}

void NxAppInfo::setDisabledIcon(const QString& szIcon)
{
	m_szIconForDisabled = szIcon;
}

QString NxAppInfo::getTryExec() const
{
	return m_szTryExec;
}

void NxAppInfo::setTryExec(const QString szTryExec)
{
	m_szTryExec = szTryExec;
}

QString NxAppInfo::getExec() const
{
	return m_szExec;
}

void NxAppInfo::setExec(const QString szExec)
{
	m_szExec = szExec;
}

QString NxAppInfo::getPath() const
{
	return m_szPath;
}

void NxAppInfo::setPath(const QString szPath)
{
	m_szPath = szPath;
}

bool NxAppInfo::getEnabled()
{
	return m_bEnabled;
}

void NxAppInfo::setEnabled(bool enabled)
{
	m_bEnabled = enabled;
}
