#ifndef NXAPPINFO_H
#define NXAPPINFO_H

#include <QObject>

class NxAppInfo : public QObject
{
public:
	Q_OBJECT
	Q_PROPERTY(QString type READ getType WRITE setType)
	Q_PROPERTY(QString version READ getVersion WRITE setVersion)
	Q_PROPERTY(QString name READ getName WRITE setName)
	Q_PROPERTY(QString comment READ getComment WRITE setComment)
	Q_PROPERTY(QString icon READ getIcon WRITE setIcon)
	Q_PROPERTY(QString tryExec READ getTryExec WRITE setTryExec)
	Q_PROPERTY(QString exec READ getExec WRITE setExec)
	Q_PROPERTY(QString path READ getPath WRITE setPath)

public:
	explicit NxAppInfo();

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
	bool m_bEnabled;
};

typedef QList<NxAppInfo*> NxAppInfoList;
typedef QMap<QString, NxAppInfo*> NxAppInfoMap;

#endif // NXAPPINFO_H
