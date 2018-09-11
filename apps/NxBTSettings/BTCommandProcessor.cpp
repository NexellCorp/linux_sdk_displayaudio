#include "BTCommandProcessor.h"

#define LOG_TAG "[NxBTSettings]"
#include <NX_Log.h>

BTCommandProcessor::BTCommandProcessor()
{
	m_bRunning = false;

	m_pRequestSendMessage = NULL;
}

BTCommandProcessor::~BTCommandProcessor()
{
	if (m_bRunning)
	{
		m_bRunning = false;
		m_Mutex.lock();
		m_Cond.wakeAll();
		m_Mutex.unlock();

		wait();
		quit();
	}
}

void BTCommandProcessor::run()
{
	m_bRunning = true;
	int i, count = 0;

	while (m_bRunning)
	{
		count = m_Commands.size();
		if (0 == count)
		{
			m_Cond.wait(&m_Mutex);
			continue;
		}

		for (i = 0; i < count; ++i)
		{
			emit signalCommandFromServer(m_Commands.takeFirst());
		}
	}
}

void BTCommandProcessor::Push(QString command)
{
	m_Mutex.lock();
	m_Commands.push_back(command);
	m_Cond.wakeAll();
	m_Mutex.unlock();
}

void BTCommandProcessor::slotCommandToServer(QString command)
{
	CommandToServer(command);
}

void BTCommandProcessor::CommandToServer(QString command)
{
	std::string s_command = command.toStdString();
	char c_command[s_command.size()+1]; // + 1 null character
	strcpy(c_command, s_command.c_str());

	NXLOGI("[%s] %d", __FUNCTION__, m_pRequestSendMessage ? 1 : 0);

	if (m_pRequestSendMessage)
		m_pRequestSendMessage("NxBTService", command.toStdString().c_str(), strlen(command.toStdString().c_str()));
}

void BTCommandProcessor::RegisterRequestSendMessage(void (*cbFunc)(const char *pDst, const char *pMsg, int32_t iMsgSize))
{
	if (cbFunc)
		m_pRequestSendMessage = cbFunc;
}
