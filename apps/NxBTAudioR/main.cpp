#include "Dialog.h"
#include "LogUtility.hpp"
#include <QApplication>

#include <signal.h>

static void signal_handler(int id)
{
	(void)id;
	NX_RequestCommand(NX_REQUEST_PROCESS_REMOVE);
	exit(EXIT_FAILURE);
}

#ifdef CONFIG_NX_DAUDIO_MANAGER
#   include <NX_Type.h>
#	include <INX_IpcManager.h>
#   include <NX_IpcPacket.h>
#   include <NX_IpcUtils.h>

#ifdef CONFIG_TEST_FLAG
bool g_first_shown = false;
#endif

//--------------------------------------------------------------------------------
// Nx IPC Manager - callback functions
int32_t callbackFromNxIPCServer(int32_t sock, uint8_t* send, uint8_t* receive, int32_t max_buffer_size, void* obj)
{
//    NX_APP_INFO* application_info   = (NX_APP_INFO*)obj;
//    NX_IIpcManager* ipc_manager     = (NX_IIpcManager*)application_info->pIpcManager;
	Dialog* window = (Dialog*)obj;
//    Dialog* self = (Dialog*)application_info->pObj;
    int32_t read_size, write_size, payload_size;
    uint32_t key;
    void* payload;
    uint8_t message[1024] = {0,};

	read_size = GetIpcManagerHandle()->Read(sock, receive, max_buffer_size);

    NX_IpcParsePacket(receive, read_size, &key, &payload, &payload_size);

	switch (key) {
	case NX_EVENT_BROADCAST:
		memcpy(message, payload, payload_size);
		break;

	default:
		break;
	}

    NxEvent *event = new NxEvent(QEvent::Type(NX_QT_CUSTOM_EVENT_TYPE));
    event->m_iEventType = key;
    event->m_szData     = (key == NX_EVENT_BROADCAST) ? QString((char*)message) : QString("");

    QCoreApplication::postEvent(window, reinterpret_cast<QEvent*>(event));
#ifdef CONFIG_TEST_FLAG
	if (key == NX_REQUEST_PROCESS_SHOW && g_first_shown)
	{
		NX_ReplyWait();
	}
#endif
	write_size = NX_IpcMakePacket(NX_REPLY_DONE, NULL, 0, send, max_buffer_size);
	write_size = GetIpcManagerHandle()->Write(sock, send, write_size);

    return write_size;
}
#endif

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

	signal(SIGINT, signal_handler);
	signal(SIGTERM, signal_handler);
	signal(SIGABRT, signal_handler);

    Dialog w;
#ifdef CONFIG_NX_DAUDIO_MANAGER
	NX_PROCESS_INFO process_info;
	NX_GetProcessInfo(&process_info);

	GetIpcManagerHandle()->RegServerCallbackFunc(&callbackFromNxIPCServer, (void*)&w);
	GetIpcManagerHandle()->StartServer(process_info.szSockName);

	if (NX_REPLY_FAIL == NX_RequestCommand(NX_REQUEST_PROCESS_ADD, &process_info)) {
		printf("@Fail, Nx_RequestCommand(). command = NX_REQUEST_PROCESS_ADD\n");
        goto request_failed;
    }

	if (NX_REPLY_FAIL == NX_RequestCommand(NX_REQUEST_FOCUS_VIDEO_TRANSIENT)) {
		printf("@Fail, Nx_RequestCommand(). command = NX_REQUEST_FOCUS_VIDEO_TRANSIENT\n");
		goto request_failed;
	}

	w.show();
	// if (NX_REPLY_FAIL == NX_RequestCommand(NX_REQUEST_PROCESS_SHOW)) {
	// 	printf("@Fail, NX_RequestCommand(). command = NX_REQUEST_PROCESS_SHOW\n");
 //        goto request_failed;
 //    }

    return a.exec();

request_failed:
	if (NX_REPLY_FAIL == NX_RequestCommand(NX_REQUEST_PROCESS_REMOVE)) {
		printf("@Fail, NX_RequestCommand(). command = NX_REQUEST_PROCESS_REMOVE\n");
    }

    return -1;
#else
    w.show();
    return a.exec();
#endif
}
