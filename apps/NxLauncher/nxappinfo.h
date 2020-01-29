#ifndef NXAPPINFO_H
#define NXAPPINFO_H

#include <QObject>
#include <NX_Type.h>

class NxPluginInfo : public QObject
{
	Q_OBJECT
public:

	Q_PROPERTY(QString type READ getType WRITE setType)
	Q_PROPERTY(QString version READ getVersion WRITE setVersion)
	Q_PROPERTY(QString name READ getName WRITE setName)
	Q_PROPERTY(QString comment READ getComment WRITE setComment)
	Q_PROPERTY(QString icon READ getIcon WRITE setIcon)
	Q_PROPERTY(QString tryExec READ getTryExec WRITE setTryExec)
	Q_PROPERTY(QString exec READ getExec WRITE setExec)
	Q_PROPERTY(QString path READ getPath WRITE setPath)
	Q_PROPERTY(bool active READ getEnabled WRITE setEnabled)
	Q_PROPERTY(bool visible READ getVisible WRITE setVisible)

public:
	explicit NxPluginInfo();

	void *m_pHandle;

	// plugin function call interface
	void (*m_pInit)(void *, const char *pArgs);

	void (*m_pIsInit)(bool *);

	void (*m_pdeInit)();

	void (*m_pShow)();

	void (*m_pHide)();

	//
	void (*m_pBackButtonClicked)();

	// register callback function
	void (*m_pRegisterLauncherShow)(void(*)(bool *));

	void (*m_pRegisterRequestTerminate)(void(*cbFunc)());

	void (*m_pRegisterRequestVolume)(void(*cbFunc)());

	void (*m_pRequestAudioFocus)(FocusType eType, FocusPriority ePriority, bool* pbOk);

	void (*m_pRegisterRequestAudioFocus)(void (*cbFunc)(FocusPriority ePriority, bool *pbOk));

	void (*m_pRequestAudioFocusTransient)(FocusPriority ePriority, bool* pbOk);

	void (*m_pRegisterRequestAudioFocusTransient)(void (*cbFunc)(FocusPriority ePriority, bool *pbOk));

	void (*m_pRegisterRequestAudioFocusLoss)(void (*cbFunc)());

	void (*m_pRequestVideoFocus)(FocusType eType, FocusPriority ePriority, bool* pbOk);

	void (*m_pRegisterRequestVideoFocus)(void (*cbFunc)(FocusPriority ePriority, bool *pbOk));

	void (*m_pRequestVideoFocusTransient)(FocusPriority ePriority, bool* pbOk);

	void (*m_pRegisterRequestVideoFocusTransient)(void (*cbFunc)(FocusPriority ePriority, bool *pbOk));

	void (*m_pRegisterRequestVideoFocusLoss)(void (*cbFunc)());

	void (*m_pRegisterRequestPluginRun)(void (*cbFunc)(const char *pPlugin, const char *pArgs));

	void (*m_pRegisterRequestPluginTerminate)(void (*cbFunc)(const char *pPlugin));

	void (*m_pRegisterRequestPluginIsRunning)(void (*cbFunc)(const char *pPlugin, bool *bOk));

	void (*m_pRegisterRequestMessage)(void (*cbFunc)(const char* pDst, const char* pMsg, int32_t iMsgSize));

	void (*m_pSendMessage)(const char* pSrc, const char* pMsg, int32_t iMsgSize);

	void (*m_pRegisterRequestPopupMessage)(void (*cbFunc)(PopupMessage*, bool *bOk));

	void (*m_pRegisterRequestExpirePopupMessage)(void (*cbFunc)());

	void (*m_pPopupMessageResponse)(bool bOk);

	void (*m_pRegisterRequestNotification)(void (*cbFunc)(PopupMessage*));

	void (*m_pRegisterRequestExpireNotification)(void (*cbFunc)());

	void (*m_pNotificationResponse)(bool bOk);

	void (*m_pMediaEventChanged)(NxMediaEvent eEvent);

	QString getType() const;
	void setType(const QString& szType );

	QString getVersion() const;
	void setVersion(const QString& szVersion);

	QString getName() const;
	void setName(const QString& szName);

	QString getComment() const;
	void setComment(const QString& szComment);

	QString getIcon() const;
	void setIcon(const QString& szIcon);

	QString getDisabledIcon() const;
	void setDisabledIcon(const QString& szIcon);

	QString getTryExec() const;
	void setTryExec(const QString szTryExec);

	QString getExec() const;
	void setExec(const QString szExec);

	QString getPath() const;
	void setPath(const QString szPath);

	bool getEnabled();
	void setEnabled(bool enabled);

	QString getTitle();
	void setTitle(QString szTitle);

	bool getAutoStart();
	void setAutoStart(bool on);

	bool getVisible();
	void setVisible(bool visible);

	bool useVideoLayer();
	void setUseVideoLayer(bool use);

private:
	// https://specifications.freedesktop.org/desktop-entry-spec/latest/ar01s05.html
	QString m_szType;
	QString m_szVersion;
	QString	m_szName;
	QString m_szComment;
	QString m_szIcon;
	QString m_szIconForDisabled;
	QString m_szTryExec;
	QString m_szExec;
	QString m_szPath;
	QString m_szTitle;
	bool m_bEnabled;
	bool m_bAutoStart;
	bool m_bVisible;
	bool m_bUseVideoLayer;
};

#endif // NXAPPINFO_H
