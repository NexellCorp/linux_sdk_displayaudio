#include "NxBTService.h"
#if 0
#   include "ExceptionHandler.h"
#endif

#include <NX_Type.h>
#include <INX_IpcManager.h>
#include <NX_IpcPacket.h>
#include <NX_IpcUtils.h>

NX_PROCESS_INFO g_process_info;
//NX_APP_INFO g_application_info;
INX_IpcManager* g_ipc_manager_handle;

bool g_calling_mode_on = false;
bool g_has_audio_focus = false;
bool g_has_audio_focus_transient = false;



int main(int argc, char** argv)
{
    // to avoid compiler warning message
    (void)argc;
    (void)argv;

	NxBTService service;


	if (NX_REPLY_FAIL == NX_RequestCommand(NX_REQUEST_PROCESS_ADD, &g_process_info)) {
		LOGT("NX_REQUEST_PROCESS_ADD [FAILED]");
		goto loop_finished;
	}

//	if (NX_REPLY_FAIL == NX_RequestCommand(NX_REQUEST_PROCESS_HIDE, &g_process_info)) {
//		LOGT("NX_REQUEST_PROCESS_HIDE [FAILED]");
//		goto loop_finished;
//	}

	service.start();
	if (!service.initialize()) {
		LOGT("NxBTService initialize => [ FAILED ]");
		goto loop_finished;
	}

	// yield cpu resource
	while (1) {
		usleep(1000); // 1ms sleep
	}

loop_finished:
	if (NX_REPLY_FAIL == NX_RequestCommand(NX_REQUEST_PROCESS_REMOVE, &g_process_info)) {
		LOGT("NX_REQUEST_PROCESS_REMOVE [FAILED]");
	}

    return 0;
}
