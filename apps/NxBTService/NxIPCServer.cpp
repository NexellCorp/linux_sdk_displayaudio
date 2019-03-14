#include "NxIPCServer.h"
#include "NxBTService.h"
#include "NxUtils.h"

#define LOG_TAG "[NxBTService]"
#include <NX_Log.h>

NxIPCServer::NxIPCServer()
{
}

NxIPCServer::~NxIPCServer()
{
	Stop();
}

void NxIPCServer::Start()
{
	m_Server.start(NX_BT_SERVICE_SERVER);
	pthread_create(&m_hThread, NULL, NxIPCServer::ThreadStub, (void *)this);
}

void NxIPCServer::Stop()
{
	if (m_Server.isRunning())
	{
		m_Server.stop();
		pthread_join(m_hThread, NULL);
	}
}

void NxIPCServer::Write(const char* pDestinatation, char* pCommand)
{
	m_Server.write(pDestinatation, pCommand);
}

void* NxIPCServer::ThreadStub(void *pObj)
{
	if (pObj)
	{
		((NxIPCServer *)pObj)->ThreadProc();
	}
}

void NxIPCServer::ThreadProc()
{
	int32_t iBufferSize = 1024;
	char buffer[iBufferSize] = {0,};

	while (m_Server.isRunning())
	{
		m_Server.read(buffer, iBufferSize);

		RunCommand(buffer);
	}
}

void NxIPCServer::RunCommand(const char* command)
{
	vector<string> tokens = CreateTokensFromCommand(command);
	size_t count = tokens.size();

	if (count < 2)
	{
		NXLOGI("[%s] Unknown command format: %s", __FUNCTION__, command);
		return;
	}

	if (tokens[CommandType_Service] == "MGT")
	{
		if (tokens[CommandType_Command].find("LOCAL DEVICE ADDRESS") == 0)
		{
			RunCommand_QueryBTMacId(tokens);
		}
		else if (tokens[CommandType_Command].find("IS PAIRED") == 0)
		{
			RunCommand_QueryIsPaired(tokens);
		}
	}
	else if (tokens[CommandType_Service] == "HS")
	{
		if (tokens[CommandType_Command].find("CONNECT") == 0)
		{
			RunCommand_ConnectHS(tokens);
		}
	}
	else if (tokens[CommandType_Service] == "AAP")
	{
		if (tokens[CommandType_Command].find("CONNECTED") == 0)
		{
			RunCommand_AAPConnected(tokens);
		}
		else if (tokens[CommandType_Command].find("DISCONNECTED") == 0)
		{
			RunCommand_AAPDisconnected(tokens);
		}
	}
	else if (tokens[CommandType_Service] == "CP")
	{
		if (tokens[CommandType_Command].find("CONNECTED") == 0)
		{
			RunCommand_CPConnected(tokens);
		}
		else if (tokens[CommandType_Command].find("DISCONNECTED") == 0)
		{
			RunCommand_CPDisconnected(tokens);
		}
	}
}

void NxIPCServer::RunCommand_QueryBTMacId(vector<string>& tokens)
{
	string macId = NxBTService::GetInstance()->GetLocalAddress(true);
	vector<string> reply;

	reply.push_back(tokens[CommandType_Service]);
	reply.push_back(tokens[CommandType_Command]);
	reply.push_back(macId);

	m_Server.write(const_cast<char *>(MakeReplyCommand(true, reply).c_str()));
}

void NxIPCServer::RunCommand_QueryIsPaired(vector<string> &tokens)
{
	vector<string> reply;
	string macId = FindArgument(&tokens[CommandType_Command]);
	int iRet = NxBTService::GetInstance()->IsPaired(macId);

	reply.push_back(tokens[CommandType_Service]);
	reply.push_back(tokens[CommandType_Command]);
	if (iRet >= 0)
	{
		reply.push_back(std::to_string(iRet));
	}

	m_Server.write(const_cast<char *>(MakeReplyCommand(iRet >= 0, reply).c_str()));
}

void NxIPCServer::RunCommand_ConnectHS(vector<string>& tokens)
{
	vector<string> reply;
	bool result = NxBTService::GetInstance()->connectToHS(tokens[CommandType_Service], tokens[CommandType_Command], false);

	reply.push_back(tokens[CommandType_Service]);
	reply.push_back(tokens[CommandType_Command]);

	m_Server.write(const_cast<char *>(MakeReplyCommand(result, reply).c_str()));
}

void NxIPCServer::RunCommand_AAPConnected(vector<string>& tokens)
{
	vector<string> reply;
	string arg = FindArgument(&tokens[CommandType_Command]);
	string macId = ToLower(arg);

	NxBTService::GetInstance()->MirroringDeviceConnected(MirroringType_AndroidAuto, macId);

	reply.push_back(tokens[CommandType_Service]);
	reply.push_back(tokens[CommandType_Command]);

	m_Server.write(const_cast<char *>(MakeReplyCommand(true, reply).c_str()));
}

void NxIPCServer::RunCommand_AAPDisconnected(vector<string>& tokens)
{
	vector<string> reply;

	NxBTService::GetInstance()->MirroringDeviceDisconnected(MirroringType_AndroidAuto);

	reply.push_back(tokens[CommandType_Service]);
	reply.push_back(tokens[CommandType_Command]);

	m_Server.write(const_cast<char *>(MakeReplyCommand(true, reply).c_str()));
}

void NxIPCServer::RunCommand_CPConnected(vector<string>& tokens)
{
	vector<string> reply;
	string arg = FindArgument(&tokens[CommandType_Command]);
	string macId = ToLower(arg);

	NxBTService::GetInstance()->MirroringDeviceConnected(MirroringType_Carplay, macId);

	reply.push_back(tokens[CommandType_Service]);
	reply.push_back(tokens[CommandType_Command]);

	m_Server.write(const_cast<char *>(MakeReplyCommand(true, reply).c_str()));
}

void NxIPCServer::RunCommand_CPDisconnected(vector<string>& tokens)
{
	vector<string> reply;

	NxBTService::GetInstance()->MirroringDeviceDisconnected(MirroringType_Carplay);

	reply.push_back(tokens[CommandType_Service]);
	reply.push_back(tokens[CommandType_Command]);

	m_Server.write(const_cast<char *>(MakeReplyCommand(true, reply).c_str()));
}
