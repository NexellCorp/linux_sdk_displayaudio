#ifndef BTCOMMANDPROCESSOR_H
#define BTCOMMANDPROCESSOR_H

#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QQueue>

class BTCommandProcessor : public QThread
{
	Q_OBJECT

signals:
	void signalCommandFromServer(QString command);

private slots:
	void slotCommandToServer(QString);

public:
	BTCommandProcessor();

	~BTCommandProcessor();

	void Push(QString command);

	void CommandToServer(QString command);

	void RegisterRequestSendMessage(void (*cbFunc)(const char *pDst, const char *pMsg, int32_t iMsgSize));

private:
	void (*m_pRequestSendMessage)(const char *pDst, const char *pMsg, int32_t iMsgSize);

protected:
	void run();

private:
	QQueue<QString> m_Commands;

	bool m_bRunning;

	QMutex m_Mutex;
	QWaitCondition m_Cond;
};

#endif // BTCOMMANDPROCESSOR_H
