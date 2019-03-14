#ifndef	NXIPCSERVER_H
#define	NXIPCSERVER_H

#include <pthread.h>
#include <NX_UDS_UDP_Server.h>

#define NX_CARPLAY_SERVER		"/tmp/nxcarplay.server"
#define NX_ANDROIDAUTO_SERVER	"/tmp/nxandroidauto.server"
#define NX_BT_SERVICE_SERVER	"/tmp/nxbtservice.server"

enum MirroringType {
	MirroringType_Carplay,
	MirroringType_AndroidAuto,
	MirroringType_Count
};

struct MirroringInfo {
	bool connected;	// device connected
	bool avk;		// AVK service connected
	bool hs;		// HS service connected
	string macId;	// device BT Mac Id

	MirroringInfo() {
		connected = false;
		avk = false;
		hs = false;
		macId = "";
	}
};

class NxIPCServer
{
public:
	NxIPCServer();

	~NxIPCServer();

	void Start();

	void Stop();

	void Write(const char* pDestinatation, char* pCommand);

private:
	static void* ThreadStub(void *pObj);

	void ThreadProc();

	void RunCommand(const char* command);

	void RunCommand_QueryBTMacId(vector<string>& tokens);

	void RunCommand_QueryIsPaired(vector<string>& tokens);

	void RunCommand_ConnectHS(vector<string>& tokens);

	void RunCommand_AAPConnected(vector<string>& tokens);

	void RunCommand_AAPDisconnected(vector<string>& tokens);

	void RunCommand_CPConnected(vector<string>& tokens);

	void RunCommand_CPDisconnected(vector<string>& tokens);

private:
	UDS_Server m_Server;

	pthread_t m_hThread;
};

#endif /* NXIPCSERVER_H */

