#include "MainDialog.h"
#include <QApplication>

Menu g_current_menu;

#include <QDebug>

#if 1 // for debugging
#include <execinfo.h>
#include <signal.h>

#define CALLSTACK_SIZE 10

static void backtrace_dump() {
    qint32 i, nptrs;
    void *buf[CALLSTACK_SIZE + 1];
    char **strings;

    nptrs = backtrace(buf, CALLSTACK_SIZE);
    printf("%s: backtrace() returned %d addresses\n", __func__, nptrs);

    strings = backtrace_symbols(buf, nptrs);

    if(strings == NULL) {
        printf("%s: no backtrace captured\n", __func__);
        return;
    }

    for(i = 0; i < nptrs; i++)
        printf("%s\n", strings[i]);

    free(strings);
}

static void sigHandler(int signum)
{
    printf("\n%s: Signal %d\n", __func__, signum);

    switch(signum ) {
    case SIGILL:
    case SIGABRT:
    case SIGSEGV:
        backtrace_dump();
        break;

    case SIGINT:
        exit(0);
        break;

    default:
        break;
    }
}
#endif

#ifdef CONFIG_NX_DAUDIO_MANAGER
#   include <NX_Type.h>
#	include <INX_IpcManager.h>
#   include <NX_IpcPacket.h>
#   include <NX_IpcUtils.h>

#   include <QShowEvent>
#   include <QHideEvent>
#   include <QCloseEvent>

#ifdef CONFIG_TEST_FLAG
bool g_first_shown = false;
#endif

//--------------------------------------------------------------------------------
// Nx IPC Manager - callback functions
int32_t callbackFromNxIPCServer(int32_t sock, uint8_t* send, uint8_t* receive, int32_t max_buffer_size, void* obj)
{
	MainDialog* window = (MainDialog*)obj;
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
#if 1
    // Register signal handler
    signal(SIGILL, sigHandler);
    signal(SIGABRT, sigHandler);
    signal(SIGSEGV, sigHandler);
#endif

    QApplication a(argc, argv);
    MainDialog w;
#ifdef CONFIG_NX_DAUDIO_MANAGER
	NX_PROCESS_INFO process_info;
	NX_GetProcessInfo(&process_info);

	GetIpcManagerHandle()->RegServerCallbackFunc(&callbackFromNxIPCServer, (void*)&w);
	GetIpcManagerHandle()->StartServer(process_info.szSockName);

	if (NX_REPLY_DONE != NX_RequestCommand(NX_REQUEST_PROCESS_ADD, &process_info)) {
		printf("@Fail, Nx_RequestCommand(). NX_REQUEST_PROCESS_ADD\n");
        goto request_failed;
    }

	if (NX_REPLY_FAIL == NX_RequestCommand(NX_REQUEST_FOCUS_VIDEO_TRANSIENT)) {
		printf("@Fail, Nx_RequestCommand(). command = NX_REQUEST_FOCUS_VIDEO_TRANSIENT\n");
		goto request_failed;
	}

	w.show();
	// if (NX_REPLY_DONE != NX_RequestCommand(NX_REQUEST_PROCESS_SHOW)) {
	// 	printf("@Fail, Nx_RequestCommand(). NX_REQUEST_PROCESS_SHOW\n");
 //        goto request_failed;
 //    }

    return a.exec();

request_failed:
	if( NX_REPLY_DONE != NX_RequestCommand(NX_REQUEST_PROCESS_REMOVE)) {
		printf("@Fail, Nx_RequestCommand(). NX_REQUEST_PROCESS_REMOVE\n");
    }

    return -1;
#else
    w.show();
    return a.exec();
#endif
}
