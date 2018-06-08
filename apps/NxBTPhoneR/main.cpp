#include "MainDialog.h"
#include <QApplication>
#include <QTranslator>
#include <getopt.h>	// for getopt_long()
#include "defines.h"
using namespace Nexell::BTPhone;

#ifndef BUFFER_SIZE
#	define BUFFER_SIZE	1024
#endif

#ifndef MIN
#define MIN(A, B) A > B ? B : A
#endif

#ifdef CONFIG_NX_DAUDIO_MANAGER
#   include <NX_Type.h>
#	include <INX_IpcManager.h>
#   include <NX_IpcPacket.h>
#   include <NX_IpcUtils.h>
#endif

Menu g_current_menu, g_previous_menu;
bool g_calling_end_is_exit;
#ifdef CONFIG_TEST_FLAG
bool g_first_shown = false;
#endif

#ifdef CONFIG_NX_DAUDIO_MANAGER
//--------------------------------------------------------------------------------
// Nx IPC Manager - callback functions
int32_t callbackFromNxIPCServer(int32_t sock, uint8_t* send, uint8_t* receive, int32_t max_buffer_size, void* obj)
{
	MainDialog* self = (MainDialog*)obj;
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

	QCoreApplication::postEvent(self, reinterpret_cast<QEvent*>(event));
#ifdef CONFIG_TEST_FLAG
	if (key == NX_REQUEST_PROCESS_SHOW && g_first_shown)
	{
		NX_ReplyWait();
	}
#endif
	write_size = NX_IpcMakePacket(NX_REPLY_DONE, NULL, 0, send, max_buffer_size);
	write_size = GetIpcManagerHandle()->Write(sock, send, write_size);

	return 0;
}
#endif /* CONFIG_NX_DAUDIO_MANAGER */

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

	// default menu is select menu.
	g_current_menu = Menu_Select;
	g_calling_end_is_exit = false;

	static struct option option_table[] =
	{
		{ .name = "help",   .has_arg = no_argument,         .flag = 0,  .val = 'h' },
		{ .name = "menu",   .has_arg = required_argument,   .flag = 0,  .val = 'm' },
		{ .name = 0,        .has_arg = 0,                   .flag = 0,  .val = 0   }
	};

	int32_t option = 0;
	// getopt_long stores the option index here
	int32_t index = 0;
	char buffer[BUFFER_SIZE] = {0,};

	while ( ( option = getopt_long(argc, argv, "m:h", option_table, &index) ) != -1)
	{
		switch (option)
		{
		case 'm':
			memcpy( buffer, optarg, MIN( strlen(optarg), BUFFER_SIZE ) );
			printf("menu options : %s\n", buffer);

			if ( strcmp( buffer, "select" ) == 0 ) {
				g_current_menu = Menu_Select;
			} else if ( strcmp( buffer, "call" ) == 0 ) {
				g_current_menu = Menu_Call;
			} else if ( strcmp( buffer, "message" ) == 0 ) {
				g_current_menu = Menu_Message;
			} else if ( strcmp (buffer, "calling" ) == 0 ) {
				g_current_menu = Menu_Calling;
				g_calling_end_is_exit = true;
			}

			break;

		case 'h':
			break;

		case '?':
			break;

		default:
			break;
		}
	}

    QTranslator* translator = new QTranslator();
    translator->load("lang_ko.qm");
    a.installTranslator(translator);

    MainDialog w;
	NX_PROCESS_INFO process_info;
	NX_GetProcessInfo(&process_info);

	GetIpcManagerHandle()->RegServerCallbackFunc(&callbackFromNxIPCServer, (void*)&w);
	GetIpcManagerHandle()->StartServer(process_info.szSockName);

#ifdef CONFIG_NX_DAUDIO_MANAGER
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
	// 	goto request_failed;
	// }

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
