#include "NxBTService.h"

INX_BT* NxBTService::m_pModel = NULL;

#define STR(X) #X

#define DEFAULT_LOCAL_NAME "NX-Link"

#define MAX_TIMEOUT 1000

// avk connected index is always '0'.
#define AVK_CONNECTED_INDEX 0

#define DAUDIO_STATUS_DATABASE_PATH "/home/root/daudio.status.db"

#define NX_ALSA_DEV_NAME_P		"plughw:0,0"
#define NX_ALSA_DEV_NAME_C		"plughw:0,0"
#define NX_ALSA_BT_DEV_NAME_P	"plughw:0,2"
#define NX_ALSA_BT_DEV_NAME_C	"plughw:0,2"

#define RET_OK		0
#define RET_FAIL	-1

//------------------------------------------------------------------------------
// NX IPC Manager - callback functions
int32_t NxBTService::callbackFromIPCServer(int32_t sock, uint8_t* send, uint8_t* receive, int32_t max_buffer_size, void* obj)
{
	NxBTService* self = (NxBTService*)obj;
	int32_t read_size, write_size, payload_size;
	uint32_t key;
	void* payload;

	read_size = g_ipc_manager_handle->Read(sock, receive, max_buffer_size);

	NX_IpcParsePacket(receive, read_size, &key, &payload, &payload_size);

	write_size = NX_IpcMakePacket(NX_REPLY_DONE, NULL, 0, send, max_buffer_size);

	switch (key) {
	case NX_REQUEST_FOCUS_AUDIO:
	case NX_REQUEST_FOCUS_AUDIO_TRANSIENT:
	{
		write_size = NX_IpcMakePacket(g_calling_mode_on ? NX_REPLY_FAIL : NX_REPLY_DONE, NULL, 0, send, max_buffer_size);
		g_has_audio_focus = g_calling_mode_on;

		self->closeAudioAVK();

		if (self->m_AVK.status == PlayStatus_Playing) {
			self->playPauseAVK();
		}

#ifdef CONFIG_A2DP_PROCESS_MANAGEMENT
		std::vector<int32_t> pids;
		char command[BUFFER_SIZE] = {0,};

		if (findPID("NxBTAudioR", pids)) {
			for (size_t i = 0; i < pids.size(); ++i) {
				sprintf(command, "kill %d", pids[i]);
				system(command);
			}
		}
#endif
		break;
	}

	default:
		break;
	}

	return g_ipc_manager_handle->Write(sock, send, write_size);
}

/*
 * MGT service callback functions
 */
void NxBTService::sendMGTOpenSucceed_stub(void* pObj, int32_t result)
{
	NxBTService* self = (NxBTService *)pObj;

	if (self) {
		self->setInitialized(result == 0);
	}
}

/* User client callback : MGT is disconnected */
// Stub
void NxBTService::sendMGTDisconnected_stub(void *pObj) {
    NxBTService* self = (NxBTService *)pObj;
    char buffer[BUFFER_SIZE];

    sprintf(buffer, "$OK#MGT#BSA SERVER KILLED\n");
    (void)self;
}

/* User client callback : Pairing is failed */
// Stub
void NxBTService::sendPairingFailed_stub(void *pObj, int32_t fail_reason)
{
    NxBTService* self = (NxBTService*)pObj;
    char buffer[BUFFER_SIZE];

    // example) "$OK#AVK#PAIRING FAILED#5\n"
    sprintf(buffer, "$%s#%s#%s#%d\n", "OK", "AVK", "PAIRING FAILED", fail_reason);
    LOG(buffer);
    self->m_IPCServer.write_broadcast(buffer);
}

void NxBTService::updatePairedDevices_stub(void *pObj)
{
    NxBTService* self = (NxBTService*)pObj;
    char buffer[BUFFER_SIZE];
    int count = m_pModel->getPairedDevCount();
    char name[249] = {0,};
    char bd_addr[6] = {0,};
	char avk_connection[100] = {0,};
	char hs_connection[100] = {0,};
    std::string s_bd_addr;

    sprintf(buffer, "$OK#%s#%s", "MGT", "PAIRED DEVICE INFO ALL LIST");

	for (int i = 0; i < count; i++) {
		if (0 > m_pModel->getPairedDevInfoByIndex(i, name, bd_addr))
			continue;

		if (strcmp(self->m_AVK.connection.bd_addr, bd_addr) == 0 && self->m_AVK.connection.on) {
			strcpy(avk_connection, "CONNECTED");
		} else {
			strcpy(avk_connection, "DISCONNECTED");
		}

		if (strcmp(self->m_HS.hs.bd_addr, bd_addr) == 0 && self->m_HS.hs.on) {
			strcpy(hs_connection, "CONNECTED");
		} else {
			strcpy(hs_connection, "DISCONNECTED");
		}

		s_bd_addr = self->bdAddrToString(bd_addr, DEVICE_ADDRESS_SIZE, ':');

		sprintf(buffer+strlen(buffer), "#<%s,%s,%s,%s>", name, s_bd_addr.c_str(), avk_connection, hs_connection);
	}

    sprintf(buffer+strlen(buffer), "\n");
    LOG(buffer);
    self->m_IPCServer.write_broadcast(buffer);
}

void NxBTService::updateUnpairedDevices_stub(void *pObj)
{
    NxBTService* self = (NxBTService*)pObj;
    char buffer[BUFFER_SIZE];
    int count = m_pModel->getPairedDevCount();
    char name[249] = {0,};
    char bd_addr[6] = {0,};
	char avk_connection[100] = {0,};
	char hs_connection[100] = {0,};
    std::string s_bd_addr;

    sprintf(buffer, "$OK#%s#%s", "MGT", "PAIRED DEVICE INFO ALL LIST");

    for (int i = 0; i < count; i++) {
        if (0 > m_pModel->getPairedDevInfoByIndex(i, name, bd_addr))
            continue;

		if (strcmp(self->m_AVK.connection.bd_addr, bd_addr) == 0 && self->m_AVK.connection.on) {
			strcpy(avk_connection, "CONNECTED");
		} else {
			strcpy(avk_connection, "DISCONNECTED");
		}

		if (strcmp(self->m_HS.hs.bd_addr, bd_addr) == 0 && self->m_HS.hs.on) {
			strcpy(hs_connection, "CONNECTED");
        } else {
			strcpy(hs_connection, "DISCONNECTED");
        }

        s_bd_addr = self->bdAddrToString(bd_addr, DEVICE_ADDRESS_SIZE, ':');

		sprintf(buffer+strlen(buffer), "#<%s,%s,%s,%s>", name, s_bd_addr.c_str(), avk_connection, hs_connection);
    }

    sprintf(buffer+strlen(buffer), "\n");

    LOG(buffer);
    self->m_IPCServer.write_broadcast(buffer);
}

void NxBTService::sendPairingRequest_stub(void *pObj_, bool auto_mode_, char *name_, char *bd_addr_, int32_t pairing_code_)
{
    // example 1) AUTO ON   > $OK#MGT#PAIRING REQUEST#AUTO ON#iPhone6-xxx#18:6D:99:20:09:CB#915112\n
    // example 2) AUTO OFF  > $OK#MGT#PAIRING REQUEST#AUTO OFF#iPhone6-xxx#18:6D:99:20:09:CB#915112\n
    NxBTService* self = (NxBTService*)pObj_;
    char buffer[BUFFER_SIZE] = {0,};
    char name[DEVICE_NAME_SIZE] = {0,};
    char bd_addr[DEVICE_ADDRESS_SIZE] = {0,};

    strcpy(name, name_);
    strcpy(bd_addr, bd_addr_);

    std::string s_bd_addr = self->bdAddrToString(bd_addr, DEVICE_ADDRESS_SIZE, ':');

    if (auto_mode_)
        sprintf(buffer, "$OK#%s#%s#%s#%s#%s#%06d\n", "MGT", "PAIRING REQUEST", "AUTO ON", name, s_bd_addr.c_str(), pairing_code_);
    else
        sprintf(buffer, "$OK#%s#%s#%s#%s#%s#%06d\n", "MGT", "PAIRING REQUEST", "AUTO OFF", name, s_bd_addr.c_str(), pairing_code_);

    self->m_IPCServer.write_broadcast(buffer);
}

void NxBTService::callbackLinkDownEventManager(void* pObj, char* bd_addr, int32_t reason_code)
{
    LOG(__FUNCTION__);
    (void)bd_addr;

    NxBTService* self = (NxBTService*)pObj;
    switch (reason_code) {
    case 0x08:
        if (pthread_attr_init(&self->h_AutoConnectAttribution) == 0) {
            if (pthread_attr_setdetachstate(&self->h_AutoConnectAttribution, PTHREAD_CREATE_DETACHED) != 0) {
                LOGT("pthread_attr_setdetachstate() [FAILED]");
            }

            pthread_create(&self->h_AutoConnectThread, &self->h_AutoConnectAttribution, NxBTService::autoConnectThread, self);
            pthread_attr_destroy(&self->h_AutoConnectAttribution);
        } else {
            LOGT("pthread_attr_init() [FAILED]");
        }
        break;

    case 0x13:
    case 0x16:
        break;

    default:
        break;
    }
}

// AVK
void NxBTService::sendAVKOpenFailed_stub(void *pObj)
{
    NxBTService* self = (NxBTService*)pObj;
    char buffer[BUFFER_SIZE] = {0,};

    // example) "$OK#AVK#OPEN FAILED\n"

    sprintf(buffer, "$OK#AVK#OPEN FAILED\n");
    self->m_IPCServer.write_broadcast(buffer);
}

void NxBTService::sendAVKConnectionStatus_stub(void *pObj, bool is_connected, char *name, char *bd_addr)
{
    NxBTService* self = (NxBTService*)pObj;
    char buffer[BUFFER_SIZE] = {0,};
    string s_bd_addr;

    strcpy(self->m_AVK.connection.bd_addr, bd_addr);
    strcpy(self->m_AVK.connection.name, name);

	// update connection status and connected index
	self->m_AVK.connection.on = is_connected;
	self->m_AVK.connection.index = ( is_connected ? m_pModel->getPairedDevIndexByAddr(bd_addr) : -1 );
	// update connected device name and address
	if (is_connected) {
		m_pModel->getPairedDevInfoByIndex(self->m_AVK.connection.index, self->m_AVK.connection.name, self->m_AVK.connection.bd_addr);
	} else {
		memset(self->m_AVK.connection.name, 0, DEVICE_NAME_SIZE);
		memset(self->m_AVK.connection.bd_addr, 0, DEVICE_ADDRESS_SIZE);
	}

    // reset play information
    memset(&self->m_AVK.info, 0, sizeof(self->m_AVK.info));

    s_bd_addr = self->bdAddrToString(self->m_AVK.connection.bd_addr, DEVICE_ADDRESS_SIZE, ':');

    // example1) CONNECTED    = "$OK#AVK#CONNECTION STATUS#CONNECTED#iphone#18:6D99:20:09:CB\n"
    // example2) DISCONNECTED = "$OK#AVK#CONNECTION STATUS#DISCONNECTED#iphone#18:6D99:20:09:CB\n"
    sprintf(buffer, "$OK#%s#%s#%s#%s#%s\n", "AVK", "CONNECTION STATUS", is_connected ? "CONNECTED" : "DISCONNECTED", self->m_AVK.connection.name, s_bd_addr.c_str());

    self->m_IPCServer.write_broadcast(buffer);

	// update bt connection
	self->m_pDAudioStatus->SetBTConnection((int32_t)(self->m_AVK.connection.on || self->m_HS.hs.on));
}

void NxBTService::sendAVKRCConnectionStatus_stub(void *pObj, bool is_connected)
{
    NxBTService* self = (NxBTService*)pObj;
    char buffer[BUFFER_SIZE] = {0,};
	char bd_addr[DEVICE_ADDRESS_SIZE] = {0,};

    sprintf(buffer, "$OK#%s#%s#%s\n", "AVK", "RC CONNECTION STATUS", is_connected ? "CONNECTED" : "DISCONNECTED");

	// update connection status and connected index
	self->m_AVK.connection.on = is_connected;

	if (is_connected && RET_OK == m_pModel->getConnectionDevAddrAVK(AVK_CONNECTED_INDEX, bd_addr)) {
		self->m_AVK.connection.index = ( is_connected ? m_pModel->getPairedDevIndexByAddr(bd_addr) : -1 );
	} else {
		self->m_AVK.connection.index = -1;
	}

	// update connected device name and address
	if (is_connected) {
		m_pModel->getPairedDevInfoByIndex(self->m_AVK.connection.index, self->m_AVK.connection.name, self->m_AVK.connection.bd_addr);
	} else {
		memset(self->m_AVK.connection.name, 0, DEVICE_NAME_SIZE);
		memset(self->m_AVK.connection.bd_addr, 0, DEVICE_ADDRESS_SIZE);
	}

    self->m_IPCServer.write_broadcast(buffer);

	// update bt connection
	self->m_pDAudioStatus->SetBTConnection((int32_t)(self->m_AVK.connection.on || self->m_HS.hs.on));
}

void NxBTService::updatePlayStatusAVK_stub(void *pObj, int32_t play_status)
{
    NxBTService* self = (NxBTService*)pObj;
    char buffer[BUFFER_SIZE] = {0,};

	PlayStatus temp = self->m_AVK.status;
	self->m_AVK.status = (PlayStatus)play_status;

	//! [1] send play status to clients.
    switch (self->m_AVK.status) {
    case PlayStatus_Stopped: // 0x00
        sprintf(buffer, "$OK#%s#%s#%s\n", "AVK", "UPDATE PLAY STATUS", "STOPPED");
        break;

    case PlayStatus_Playing: // 0x01
        sprintf(buffer, "$OK#%s#%s#%s\n", "AVK", "UPDATE PLAY STATUS", "PLAYING");
        break;

    case PlayStatus_Paused: // 0x02
        sprintf(buffer, "$OK#%s#%s#%s\n", "AVK", "UPDATE PLAY STATUS", "PAUSED");
        break;
    }

    self->m_IPCServer.write_broadcast(buffer);
	//! [1]

	if (temp != self->m_AVK.status && self->m_AVK.status == PlayStatus_Playing) {
		//! [2] try to switching audio focus
		if (!g_has_audio_focus) {
			g_has_audio_focus = (NX_REPLY_DONE == NX_RequestCommand( NX_REQUEST_FOCUS_AUDIO, &g_process_info));
		}
		//! [2]

		//! [3] try to open audio device
		self->openAudioAVK();
		//! [3]

		//! [4] if CONFIG_A2DP_PROCESS_MANAGEMENT option is enabled, run following code.
#ifdef CONFIG_A2DP_PROCESS_MANAGEMENT
//		std::vector<int32_t> pids;
//		char command[BUFFER_SIZE] = {0,};

//		if (!findPID("NxBTAudioR", pids)) {
//			sprintf(command, "%s -platform wayland &", "/nexell/daudio/NxBTAudioR/NxBTAudioR");
//			system(command);
//		}
#endif
		//! [4]
	}
}

void NxBTService::updateMediaElementsAVK_stub(void *pObj, char *mediaTitle, char *mediaArtist, char *mediaAlbum, char *mediaGenre, int32_t duration_msec)
{
    NxBTService* self = (NxBTService*)pObj;
    char buffer[BUFFER_SIZE] = {0,};

    memcpy(self->m_AVK.info.title, mediaTitle, sizeof(self->m_AVK.info.title));
    memcpy(self->m_AVK.info.artist, mediaArtist, sizeof(self->m_AVK.info.artist));
    memcpy(self->m_AVK.info.album, mediaAlbum, sizeof(self->m_AVK.info.album));
    memcpy(self->m_AVK.info.genre, mediaGenre, sizeof(self->m_AVK.info.genre));
    self->m_AVK.info.duration = duration_msec;
    // iphone6: play position and duration time unknown.
    //  - test os : ios10, ios11
    //  - test play app : default music,
    self->m_AVK.info.position = 0;

    // example) "$OK#AVK#UPDATE MEDIA ELEMENT#[TITLE]#[ARTIST]#[ALBUM]#[GENRE]#[DURATION]\n"
    sprintf(buffer, "$OK#%s#%s#%s#%s#%s#%s#%d\n", "AVK", "UPDATE MEDIA ELEMENT", self->m_AVK.info.title, self->m_AVK.info.artist, self->m_AVK.info.album, self->m_AVK.info.genre, self->m_AVK.info.duration);

    self->m_IPCServer.write_broadcast(buffer);
}

void NxBTService::updatePlayPositionAVK_stub(void *pObj, int32_t play_pos_msec)
{
    LOG("NxBTService::updatePlayPositionAVK_stub");
    NxBTService* self = (NxBTService*)pObj;
    char buffer[BUFFER_SIZE] = {0,};

    // example) $OK#AVK#UPDATE PLAY POSITION#12345\n"
    self->m_AVK.info.position = play_pos_msec;

    sprintf(buffer, "$OK#%s#%s#%d\n", "AVK", "UPDATE PLAY POSITION", self->m_AVK.info.position);

    self->m_IPCServer.write_broadcast(buffer);
}

void NxBTService::sendAVKStreamingStarted_stub(void* pObj, bool is_opened)
{
    LOG(__FUNCTION__);
    NxBTService* self = (NxBTService*)pObj;
    char buffer[BUFFER_SIZE] = {0,};

	//! [1] send streaming start status to clients.
    if (is_opened)
        sprintf(buffer, "$OK#%s#%s#%s\n", "AVK", "STREAMING STARTED", "ALSA DEVICE OPENED");
    else
        sprintf(buffer, "$OK#%s#%s#%s\n", "AVK", "STREAMING STARTED", "ALSA DEVICE OPEN FAILED");

	self->m_IPCServer.write_broadcast(buffer);
	//! [1]

	//! [2] switching audio focus
	if (!g_has_audio_focus) {
		g_has_audio_focus = (NX_REPLY_DONE == NX_RequestCommand(NX_REQUEST_FOCUS_AUDIO, &g_process_info));
	}
	//! [2]

	//! [3] try to open audio device
	self->openAudioAVK();
	//! [3]
}

void NxBTService::sendAVKStreamingStopped_stub(void *pObj)
{
    LOG(__FUNCTION__);
    NxBTService* self = (NxBTService*)pObj;
    char buffer[BUFFER_SIZE] = {0,};

	//! [1] send streaming stop status to clients.
    sprintf(buffer, "$OK#%s#%s\n", "AVK", "STREAMING STOPPED");

    self->m_IPCServer.write_broadcast(buffer);
	//! [1]

	//! [2] try to close audio device
//	self->closeAudioAVK();
	//! [2]
}

// phone : HS
void NxBTService::sendHSOpenFailed_stub(void *pObj)
{
    NxBTService* self = (NxBTService*)pObj;
    char buffer[BUFFER_SIZE] = {0,};

    // example) "$OK#HS#OPEN FAILED\n"

    sprintf(buffer, "$OK#HS#OPEN FAILED\n");
    self->m_IPCServer.write_broadcast(buffer);
}

void NxBTService::sendHSConnectionStatus_stub(void *pObj, bool is_connected, char *name, char *bd_addr)
{
    NxBTService* self = (NxBTService*)pObj;
    char buffer[BUFFER_SIZE] = {0,};
    string s_bd_addr;

    // update device address
	if (bd_addr)
		strcpy(self->m_HS.hs.bd_addr, bd_addr);
	else
		memset(self->m_HS.hs.bd_addr, 0, DEVICE_ADDRESS_SIZE);

    // update device name
	if (name)
		strcpy(self->m_HS.hs.name, name);
	else
		self->m_HS.hs.name[0] = '\0';
	// update connection status and connected index
	self->m_HS.hs.on = is_connected;
	self->m_HS.hs.index = ( is_connected ? m_pModel->getPairedDevIndexByAddr(bd_addr) : -1 );

    s_bd_addr = self->bdAddrToString(self->m_HS.hs.bd_addr, DEVICE_ADDRESS_SIZE, ':');

#if 1 // if hs service is connected, sub-profile(service) try to open.
    if (is_connected) {
        self->m_HS.pbc.connection.on = (0 <= m_pModel->connectToPBC(self->m_HS.hs.index));
        self->m_HS.mce.on = (0 <= m_pModel->connectToMCE(self->m_HS.hs.index));
    } else {
        self->m_HS.pbc.connection.on = !(0 <= m_pModel->disconnectFromPBC());
        self->m_HS.mce.on = !(0 <= m_pModel->disconnectFromMCE());
    }
#endif

    // example1) CONNECTED    = "$OK#HS#CONNECTION STATUS#CONNECTED#iphone#18:6D99:20:09:CB\n"
    // example2) DISCONNECTED = "$OK#HS#CONNECTION STATUS#DISCONNECTED#iphone#18:6D99:20:09:CB\n"
    sprintf(buffer, "$OK#%s#%s#%s#%s#%s\n", "HS", "CONNECTION STATUS", is_connected ? "CONNECTED" : "DISCONNECTED", self->m_HS.hs.name, s_bd_addr.c_str());

    self->m_IPCServer.write_broadcast(buffer);

	// update bt connection
	self->m_pDAudioStatus->SetBTConnection((int32_t)(self->m_AVK.connection.on || self->m_HS.hs.on));
}

void NxBTService::sendHSCallStatus_stub(void *pObj, int32_t call_status)
{
    NxBTService* self = (NxBTService*)pObj;
    char buffer[BUFFER_SIZE] = {0,};
    char status[100] = {0,};

	//! [1] send 'CALL STATUS' command to client
    // example) $OK#HS#CALL STATUS#[STATUS]\n
    switch (call_status) {
    case HANG_UP_CALL:
        strcpy(status, "HANG UP CALL");
        break;

    case INCOMMING_CALL:
        strcpy(status, "INCOMMING CALL");
        break;

    case READY_OUTGOING_CALL:
        strcpy(status, "READY OUTGOING CALL");
        break;

    case OUTGOING_CALL:
        strcpy(status, "OUTGOING CALL");
        break;

    case PICK_UP_CALL:
        strcpy(status, "PICK UP CALL");
        break;

    case DISCONNECTED_CALL:
        strcpy(status, "DISCONNECTED CALL");
        break;

    default: /* UNKNOWN_CALL */
        strcpy(status, "UNKNOWN CALL");
        break;
    }

	sprintf(buffer, "$OK#%s#%s#%s\n", "HS", "CALL STATUS", status);

	self->m_IPCServer.write_broadcast(buffer);
	//! [1]

	//! [2] switching audio focus
	switch (call_status) {
	case HANG_UP_CALL:
	case DISCONNECTED_CALL:
		// switching calling mode
		g_calling_mode_on = false;

		if ( !m_pModel->isOpenedAudioHS() && g_has_audio_focus_transient ) {
			// release audio focus
			g_has_audio_focus_transient = (NX_REPLY_FAIL == NX_RequestCommand(NX_REQUEST_FOCUS_AUDIO_LOSS, &g_process_info));
		}
		break;

	case INCOMMING_CALL:
	case READY_OUTGOING_CALL:
	{
		// switching calling mode
		g_calling_mode_on = true;

		// switching audio focus (get)
		if ( !g_has_audio_focus ) {
			g_has_audio_focus_transient = (NX_REPLY_DONE == NX_RequestCommand(NX_REQUEST_FOCUS_AUDIO_TRANSIENT, &g_process_info));
		}

#ifdef CONFIG_HSP_PROCESS_MANAGEMENT
		std::vector<int32_t> pids;
		char command[BUFFER_SIZE] = {0,};

		if (!findPID("NxBTPhoneR", pids)) {
			sprintf(command, "%s --menu calling -platform wayland &", "/nexell/daudio/NxBTPhoneR/NxBTPhoneR");
			system(command);
		}
#endif
		break;
	}
	default:
		break;

	}
	//! [2]

}

void NxBTService::sendHSBatteryStatus_stub(void *pObj, int32_t batt_status)
{
    NxBTService* self = (NxBTService*)pObj;
    char buffer[BUFFER_SIZE] = {0,};

    sprintf(buffer, "$OK#%s#%s#%d\n", "HS", "BATTERY STATUS", batt_status);
    self->m_IPCServer.write_broadcast(buffer);
}

void NxBTService::sendHSCallOperName_stub(void *pObj, char *name)
{
    NxBTService* self = (NxBTService*)pObj;
    char buffer[BUFFER_SIZE] = {0,};

    sprintf(buffer, "$OK#%s#%s#%s\n", "HS", "CALL OPER NAME", name);
    self->m_IPCServer.write_broadcast(buffer);
}

void NxBTService::sendHSAudioMuteStatus_stub(void *pObj, bool is_muted, bool is_opened)
{
    NxBTService* self = (NxBTService*)pObj;
    char buffer[BUFFER_SIZE] = {0,};
    char audio[BUFFER_SIZE] = {0,};
    char microphone[BUFFER_SIZE] = {0,};

	//! [1] send Audio open/close and Microphone mute status to clients
    strcpy(audio, is_opened ? "AUDIO OPENED" : "AUDIO CLOSED");
	strcpy(microphone, is_muted ? "MICROPHONE MUTE ON" : "MICROPHONE MUTE OFF");

    sprintf(buffer, "$OK#HS#AUDIO MUTE STATUS#%s#%s\n", audio, microphone);

    self->m_IPCServer.write_broadcast(buffer);
	//! [1]

	//! [2] If all of the following conditions are met, the audio focus is returned.
	//!     condition 1 : audio focus transient flag	'ON'
	//!		condition 2 : calling mode flag				'OFF'
	//!		condition 3 : audio open / close status		'CLOSE'
	if ( g_has_audio_focus_transient && !g_calling_mode_on && !is_opened ) {
		// release audio focus
		g_has_audio_focus_transient = (NX_REPLY_FAIL == NX_RequestCommand(NX_REQUEST_FOCUS_AUDIO_LOSS, &g_process_info));
	}
	//! [2]
}

void NxBTService::sendHSIncommingCallNumber_stub(void *pObj, char *number)
{
    NxBTService* self = (NxBTService*)pObj;
    char buffer[BUFFER_SIZE] = {0,};

    /* [TODO]: what is 129? unknown...
     * NXBT HS Event : Send incomming call number to UI =>  "0316987429",129,,,"      "
     * [sendHSIncommingCallNumber_stub] number =  "0316987429",129,,,"      "
     */

    sprintf(buffer, "$OK#%s#%s#%s\n", "HS", "INCOMMING CALL NUMBER", number);
    self->m_IPCServer.write_broadcast(buffer);
}

// phone : PBC (phone book)
void NxBTService::sendPBCOpenFailed_stub(void *pObj)
{
    NxBTService* self = (NxBTService*)pObj;
    char buffer[BUFFER_SIZE] = {0,};

    // example) "$OK#PBC#OPEN FAILED\n"

    sprintf(buffer, "$OK#PBC#OPEN FAILED\n");
    self->m_IPCServer.write_broadcast(buffer);
}

void NxBTService::sendPBCConnectionStatus_stub(void *pObj, bool is_connected)
{
    NxBTService* self = (NxBTService*)pObj;
    char buffer[BUFFER_SIZE] = {0,};

    // example1) CONNECTED    = "$OK#PBC#CONNECTION STATUS#CONNECTED\n"
    // example2) DISCONNECTED = "$OK#PBC#CONNECTION STATUS#DISCONNECTED\n"
    self->m_HS.pbc.connection.on = is_connected;

    sprintf(buffer, "$OK#%s#%s#%s\n", "PBC", "CONNECTION STATUS", is_connected ? "CONNECTED" : "DISCONNECTED");

    self->m_IPCServer.write_broadcast(buffer);
}

void NxBTService::sendNotifyGetPhonebook_stub(void *pObj, int32_t  type)
{
    NxBTService* self = (NxBTService*)pObj;
    char buffer[BUFFER_SIZE] = {0,};
    char command[100] = {0,};

    switch (self->m_HS.pbc.download) {
    case DownloadType_PhoneBook:
        strcpy(command, "DOWNLOAD PHONEBOOK");
        break;

	case DownloadType_CallHistory:
        strcpy(command, "DOWNLOAD CALL LOG");
        break;

    default:
        return;
    }

    switch (type) {
    case PARAM_PROGRESS:
        sprintf(buffer, "$OK#PBC#%s#PROGRESSING\n", command);
        break;

    case PARAM_PHONEBOOK:
        sprintf(buffer, "$OK#PBC#%s#DATA TRANSFER COMPLETED\n", command);
        break;

    case PARAM_FILE_TRANSFER_STATUS:
        sprintf(buffer, "$OK#PBC#%s#FILE CREATED#%s\n", command, PB_DATA_PATH);
        break;
    }

    self->m_IPCServer.write_broadcast(buffer);
}

// phone : MCE (message)
void NxBTService::sendMCEOpenFailed_stub(void *pObj)
{
    NxBTService* self = (NxBTService*)pObj;
    char buffer[BUFFER_SIZE] = {0,};

    // example) "$OK#MCE#OPEN FAILED\n"

    sprintf(buffer, "$OK#MCE#OPEN FAILED\n");
    self->m_IPCServer.write_broadcast(buffer);
}

void NxBTService::sendMCEConnectionStatus_stub(void *pObj, bool is_connected)
{
    NxBTService* self = (NxBTService*)pObj;
    char buffer[BUFFER_SIZE] = {0,};
    char bd_addr[DEVICE_ADDRESS_SIZE] = {0,};

    self->m_HS.mce.on = is_connected;
    if (0 > self->m_pModel->getConnectionDevAddrHS(bd_addr)) {
        std::string a = self->bdAddrToString(bd_addr, DEVICE_ADDRESS_SIZE, ':');
    }

    // example1) CONNECTED    = "$OK#MCE#CONNECTION STATUS#CONNECTED\n"
    // example2) DISCONNECTED = "$OK#MCE#CONNECTION STATUS#DISCONNECTED\n"
    sprintf(buffer, "$OK#%s#%s#%s\n", "MCE", "CONNECTION STATUS", is_connected ? "CONNECTED" : "DISCONNECTED");

    self->m_IPCServer.write_broadcast(buffer);
}

void NxBTService::sendNotifyGetMessageMCE_stub(void *pObj)
{
    NxBTService* self = (NxBTService*)pObj;
    char buffer[BUFFER_SIZE] = {0,};

    char name[20] = {0,};
    char phonenumber[20] = {0,};
    char message[256] = {0,};

    if (0 > m_pModel->getParserBmsg(name, phonenumber, message)) {
        sprintf(buffer, "$OK#%s#%s#%s#%s#%s\n", "MCE", "DOWNLOAD SMS MESSAGE", name, phonenumber, message);

        self->m_IPCServer.write_broadcast(buffer);
    }
}

NxBTService::NxBTService()
{
	//! [1] init DB for bluetooth status
	m_pDAudioStatus = new CNX_DAudioStatus(DAUDIO_STATUS_DATABASE_PATH);
	m_pDAudioStatus->SetBTConnection(0);
	//! [1]

	//! [2] variable reset to 0
	memset(&m_AVK, 0, sizeof(m_AVK));
	memset(&m_HS, 0, sizeof(m_HS));
	m_bInitialized = false;
	m_pModel = NULL;
	//! [2]

	//! [3] settings for NX IPC module
	g_ipc_manager_handle = GetIpcManagerHandle();

	// set NX_PROCESS_INFO structure
	NX_GetProcessInfo(&g_process_info);

	g_process_info.iFlags |= NX_PROCESS_FLAG_BACKGROUND;

	g_ipc_manager_handle->RegServerCallbackFunc(&callbackFromIPCServer, (void*)this);
	g_ipc_manager_handle->StartServer(g_process_info.szSockName);
	//! [3]
}

bool NxBTService::initialize()
{
	//! [1] settings for Nx BT module
	m_pModel = getInstance();
	if (!m_pModel) {
		return false;
	}
	//! [1]

	//! [2]
	registerCallbackFunctions();
	//! [2]

	//! [3]
	return (0 == m_pModel->initDevManager());
	//! [3]
}

void NxBTService::registerCallbackFunctions()
{
    // normal : MGT
	m_pModel->registerMGTOpenSucceedCbManager(this, sendMGTOpenSucceed_stub);
    m_pModel->registerMGTDisconnectedCbManager(this, sendMGTDisconnected_stub);
    m_pModel->registerPairingFailedCbManager(this, sendPairingFailed_stub);
    m_pModel->registerPairedDevicesCbManager(this, updatePairedDevices_stub);
    m_pModel->registerUnpairedDevicesCbManager(this, updateUnpairedDevices_stub);
    m_pModel->registerPairingRequestCbManager(this, sendPairingRequest_stub);
    m_pModel->registerLinkDownEventCbManager(this, callbackLinkDownEventManager);
    // audio : AVK
    m_pModel->registerOpenFailedCbAVK(this, sendAVKOpenFailed_stub);
    m_pModel->registerConnectionStatusCbAVK(this, sendAVKConnectionStatus_stub);
    m_pModel->registerConnectionStatusCbAVKRC(this, sendAVKRCConnectionStatus_stub);
    m_pModel->registerPlayStatusCbAVK(this, updatePlayStatusAVK_stub);
    m_pModel->registerMediaElementCbAVK(this, updateMediaElementsAVK_stub);
    m_pModel->registerPlayPositionCbAVK(this, updatePlayPositionAVK_stub);
    m_pModel->registerStreamingStartedCbAVK(this, sendAVKStreamingStarted_stub);
    m_pModel->registerStreamingStoppedCbAVK(this, sendAVKStreamingStopped_stub);
    // phone : HS
    m_pModel->registerOpenFailedCbHS(this, sendHSOpenFailed_stub);
    m_pModel->registerConnectionStatusCbHS(this, sendHSConnectionStatus_stub);
    m_pModel->registerCallStatusCbHS(this, sendHSCallStatus_stub);
    m_pModel->registerBatteryStatusCbHS(this, sendHSBatteryStatus_stub);
    m_pModel->registerCallOperNameCbHS(this, sendHSCallOperName_stub);
    m_pModel->registerAudioMuteStatusCbHS(this, sendHSAudioMuteStatus_stub);
    m_pModel->registerIncommingCallNumberCbHS(this,sendHSIncommingCallNumber_stub);
    // phone : PBC (phone book)
    m_pModel->registerOpenFailedCbPBC(this, sendPBCOpenFailed_stub);
    m_pModel->registerConnectionStatusCbPBC(this, sendPBCConnectionStatus_stub);
    m_pModel->registerNotifyGetPhonebookCbPBC(this, sendNotifyGetPhonebook_stub);
    // phone : MCE (message)
    m_pModel->registerOpenFailedCbMCE(this, sendMCEOpenFailed_stub);
    m_pModel->registerConnectionStatusCbMCE(this, sendMCEConnectionStatus_stub);
    m_pModel->registerNotifyGetMessageCbMCE(this, sendNotifyGetMessageMCE_stub);
}

void NxBTService::start()
{
    pthread_create(&h_pthread, NULL, NxBTService::foo, this);
}

void* NxBTService::foo(void* args)
{
    NxBTService* obj = (NxBTService*)args;
    char buffer_recv[BUFFER_SIZE];
//    CommandErrorType type;
    // register client path list
    obj->m_IPCServer.pushClient(DEFAULT_CLIENT_PATH_FOR_AUDIO);
    obj->m_IPCServer.pushClient(DEFAULT_CLIENT_PATH_FOR_PHONE);
    obj->m_IPCServer.pushClient(DEFAULT_CLIENT_PATH_FOR_SETTINGS);

    if (!obj->m_IPCServer.isRunning())
        obj->m_IPCServer.start((char*)DEFAULT_SERVER_PATH);

    while (obj->m_IPCServer.isRunning()) {
        // 1. wait for read from client.
        obj->m_IPCServer.read(buffer_recv, BUFFER_SIZE);

        // 2. run command
        //  2.1. parse command
        //   2.1.1. if current command is valid, run it.
        //   2.1.2. else
        //  2.2. make command result data(message) - buffer_send
		obj->runCommand(buffer_recv);
    }

    return NULL;
}

void* NxBTService::autoConnectThread(void* args)
{
    NxBTService* self = (NxBTService*)args;

    LOG(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>");
    LOG(__FUNCTION__);
    char buffer[1024];
    sprintf(buffer, "is autoconnection ? %s\nAVK is connected ? %s\nHS is connected ? %s\n", m_pModel->isAutoConnection() ? "ON" : "OFF", self->m_AVK.connection.on ? "ON" : "OFF", self->m_HS.hs.on ? "ON" : "OFF");
    LOG(buffer);
    while (m_pModel->isAutoConnection() && !(self->m_AVK.connection.on || self->m_HS.hs.on)) {
        LOG("try to auto connection!");
        m_pModel->autoConnection(true);
//        pthread_yield();
        usleep(1000000);
    }
    LOG("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<");

    return NULL;
}

//void* NxBTService::checkALSADeviceIsClosedThread(void* args)
//{
//    NxBTService* self = (NxBTService*)args;

//    LOG(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>");
//    LOG(__FUNCTION__);
//    clock_t start = clock();
//    clock_t end = start;
//    double elapsed = (double)(end- start) / CLOCKS_PER_SEC;
//    bool closed = false;
//    char buffer[1024];

//    while (elapsed < MAX_TIMEOUT) {
//        closed = !m_pModel->isAudioStatusAVK();

//        if (closed) {
//            break;
//        }

//        usleep(100000); // 100 ms sleep
//        elapsed += 100;
//    }

//    if (closed) {
//        sprintf(buffer, "$%s#%s#%s\n", "OK", "AVK", "AUDIO CLOSED");
//    } else {
//        sprintf(buffer, "$%s#%s#%s\n", "NG", "AVK", "AUDIO CLOSED");
//        char msg[1024];
//        sprintf(msg, "\n\n[TIMEOUT]: close failed! elapsed time = %.1f sec.\n\n", elapsed);
//        LOG(msg);
//    }

//    self->m_IPCServer.write_broadcast(buffer);
//    LOG("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<");
//    return NULL;
//}

void NxBTService::stop()
{
    int status;

    // stop server
    if (m_IPCServer.isRunning())
        m_IPCServer.stop();

    pthread_join(h_pthread, (void**)&status);
}

bool NxBTService::runCommand(char *command)
{
    std::vector<std::string> tokens = createTokensFromCommand(command);
    size_t count = tokens.size();

	if (count != 2 || !m_bInitialized) {
		goto loop_finish;
    }

    if (tokens[CommandType_Service] == "MGT") {
		if (tokens[CommandType_Command].find("PING") == 0) {

			return ping(tokens[CommandType_Service], tokens[CommandType_Command]);

		} else if (tokens[CommandType_Command].find("ENABLE AUTO CONNECTION") == 0) {

			return setEnableAutoConnection(tokens[CommandType_Service], tokens[CommandType_Command]);

        } else if (tokens[CommandType_Command].find("AUTO CONNECTION") == 0) {

			return isEnabledAutoConnection(tokens[CommandType_Service], tokens[CommandType_Command]);

        } else if (tokens[CommandType_Command].find("ACCEPT PAIRING") == 0) {

			return acceptPairing(tokens[CommandType_Service], tokens[CommandType_Command]);

        } else if (tokens[CommandType_Command].find("REJECT PAIRING") == 0) {

			return rejectPairing(tokens[CommandType_Service], tokens[CommandType_Command]);

        } else if (tokens[CommandType_Command].find("UNPAIR DEVICE ALL") == 0) {

			return unpairAll(tokens[CommandType_Service], tokens[CommandType_Command]);

        } else if (tokens[CommandType_Command].find("UNPAIR DEVICE") == 0) {

			return unpair(tokens[CommandType_Service], tokens[CommandType_Command]);

        } else if (tokens[CommandType_Command].find("ENABLE AUTO PAIRING") == 0) {

			return setEnableAutoPairing(tokens[CommandType_Service], tokens[CommandType_Command]);

        } else if (tokens[CommandType_Command].find("AUTO PAIRING") == 0) {

			return isEnabledAutoPairing(tokens[CommandType_Service], tokens[CommandType_Command]);

        } else if (tokens[CommandType_Command].find("LOCAL DEVICE NAME") == 0) {

			return localDeviceName(tokens[CommandType_Service], tokens[CommandType_Command]);

        } else if (tokens[CommandType_Command].find("LOCAL DEVICE ADDRESS") == 0) {

			return localDeviceAddress(tokens[CommandType_Service], tokens[CommandType_Command]);

        } else if (tokens[CommandType_Command].find("RENAME LOCAL DEVICE") == 0) {

			return renameLocalDevice(tokens[CommandType_Service], tokens[CommandType_Command]);

        } else if (tokens[CommandType_Command].find("PAIRED DEVICE COUNT") == 0) {

			return countOfPairedDevice(tokens[CommandType_Service], tokens[CommandType_Command]);

        } else if (tokens[CommandType_Command].find("PAIRED DEVICE INFO ALL LIST") == 0) {

			return infoListOfPairedDeviceAll(tokens[CommandType_Service], tokens[CommandType_Command]);

        } else if (tokens[CommandType_Command].find("PAIRED DEVICE INFO BY INDEX") == 0) {

			return infoOfPairedDeviceByIndex(tokens[CommandType_Service], tokens[CommandType_Command]);

        } else if (tokens[CommandType_Command].find("PAIRED DEVICE NAME BY INDEX") == 0) {

			return nameOfPairedDeviceByIndex(tokens[CommandType_Service], tokens[CommandType_Command]);

        } else if (tokens[CommandType_Command].find("PAIRED DEVICE ADDRESS BY INDEX") == 0) {

			return addressOfPairedDeviceByIndex(tokens[CommandType_Service], tokens[CommandType_Command]);

		}

    } else if (tokens[0] == "AVK") {

        if (tokens[CommandType_Command].find("IS CONNECTED") == 0) {

			return isConnectedToAVK(tokens[CommandType_Service], tokens[CommandType_Command]);

		} else if (tokens[CommandType_Command].find("CONNECTED DEVICE INDEX") == 0) {

			return indexOfConnectedDeviceToAVK(tokens[CommandType_Service], tokens[CommandType_Command]);

		} else if (tokens[CommandType_Command].find("CONNECT") == 0) {

			return connectToAVK(tokens[CommandType_Service], tokens[CommandType_Command]);

        } else if (tokens[CommandType_Command].find("DISCONNECT") == 0) {

			return disconnectToAVK(tokens[CommandType_Service], tokens[CommandType_Command]);

		} else if (tokens[CommandType_Command].find("ADDRESS CONNECTED DEVICE BY INDEX") == 0) {

			return addressOfConnectedDeviceToAVK(tokens[CommandType_Service], tokens[CommandType_Command]);

        } else if (tokens[CommandType_Command].find("PLAY START") == 0) {

			return playStartAVK(tokens[CommandType_Service], tokens[CommandType_Command]);

        } else if (tokens[CommandType_Command].find("PLAY STOP") == 0) {

			return playStopAVK(tokens[CommandType_Service], tokens[CommandType_Command]);

        } else if (tokens[CommandType_Command].find("PLAY PAUSE") == 0) {

			return playPauseAVK(tokens[CommandType_Service], tokens[CommandType_Command]);

        } else if (tokens[CommandType_Command].find("PLAY NEXT") == 0) {

			return playNextAVK(tokens[CommandType_Service], tokens[CommandType_Command]);

        } else if (tokens[CommandType_Command].find("PLAY PREV") == 0) {

			return playPrevAVK(tokens[CommandType_Service], tokens[CommandType_Command]);

        } else if (tokens[CommandType_Command].find("PLAY STATUS") == 0) {

			return playStatusAVK(tokens[CommandType_Service], tokens[CommandType_Command]);

        } else if (tokens[CommandType_Command].find("PLAY INFO") == 0) {

			return playInfoAVK(tokens[CommandType_Service], tokens[CommandType_Command]);

        } else if (tokens[CommandType_Command].find("CLOSE AUDIO") == 0) {

			return closeAudioAVK(tokens[CommandType_Service], tokens[CommandType_Command]);

        } else if (tokens[CommandType_Command].find("OPEN AUDIO") == 0) {

			return openAudioAVK(tokens[CommandType_Service], tokens[CommandType_Command]);

        }

    } else if (tokens[CommandType_Service] == "HS") {

		//----------------------------------------------------------------------
		// CONNECTION commands
		if (tokens[CommandType_Command].find("CONNECTED DEVICE INDEX") == 0)
		{
			return indexOfConnectedDeviceToHS(tokens[CommandType_Service], tokens[CommandType_Command]);
		}
		if (tokens[CommandType_Command].find("CONNECT") == 0)
		{
			return connectToHS(tokens[CommandType_Service], tokens[CommandType_Command]);
		}
		else if (tokens[CommandType_Command].find("DISCONNECT") == 0)
		{
			return disconnectFromHS(tokens[CommandType_Service], tokens[CommandType_Command]);
		}
		else if (tokens[CommandType_Command].find("IS CONNECTED") == 0)
		{
			return isConnectedToHS(tokens[CommandType_Service], tokens[CommandType_Command]);
		}
		//----------------------------------------------------------------------
		// CALL commands
		else if (tokens[CommandType_Command].find("DIAL") == 0)
		{
			return dialPhoneNumber(tokens[CommandType_Service], tokens[CommandType_Command]);
		}
		else if (tokens[CommandType_Command].find("REDIAL") == 0)
		{
			return reDialPhoneNumber(tokens[CommandType_Service], tokens[CommandType_Command]);
		}
		else if (tokens[CommandType_Command].find("PICK UP CALL") == 0)
		{
			return pickUpCall(tokens[CommandType_Service], tokens[CommandType_Command]);
		}
		else if (tokens[CommandType_Command].find("HANG UP CALL") == 0)
		{
			return hangUpCall(tokens[CommandType_Service], tokens[CommandType_Command]);
		}
		//----------------------------------------------------------------------
		// AUDIO/MICROPHONE commands
		else if (tokens[CommandType_Command].find("MICROPHONE MUTE") == 0)
		{
			return muteMicrophoneHS(tokens[CommandType_Service], tokens[CommandType_Command]);
		}
		else if (tokens[CommandType_Command].find("AUDIO OPEN") == 0)
		{
			return openAudioHS(tokens[CommandType_Service], tokens[CommandType_Command]);
		}
		else if (tokens[CommandType_Command].find("AUDIO CLOSE") == 0)
		{
			return closeAudioHS(tokens[CommandType_Service], tokens[CommandType_Command]);
		}

    } else if (tokens[CommandType_Service] == "MCE") {

        if (tokens[CommandType_Command].find("IS CONNECTED") == 0) {

			return isConnectedToMCE(tokens[CommandType_Service], tokens[CommandType_Command]);

        } else if (tokens[CommandType_Command].find("CONNECT") == 0) {

			return connectToMCE(tokens[CommandType_Service], tokens[CommandType_Command]);

        } else if (tokens[CommandType_Command].find("DISCONNECT") == 0) {

			return disconnectFromMCE(tokens[CommandType_Service], tokens[CommandType_Command]);

        } else if (tokens[CommandType_Command].find("DOWNLOAD SMS MESSAGE") == 0) {

			return downloadSMSMessage(tokens[CommandType_Service], tokens[CommandType_Command]);

        }

    } else if (tokens[CommandType_Service] == "PBC") {

        if (tokens[CommandType_Command].find("IS CONNECTED") == 0) {

			return isConnectedToPBC(tokens[CommandType_Service], tokens[CommandType_Command]);

        } else if (tokens[CommandType_Command].find("CONNECT") == 0) {

			return connectToPBC(tokens[CommandType_Service], tokens[CommandType_Command]);

        } else if (tokens[CommandType_Command].find("DISCONNECT") == 0) {

			return disconnectFromPBC(tokens[CommandType_Service], tokens[CommandType_Command]);

        } else if (tokens[CommandType_Command].find("DOWNLOAD PHONEBOOK") == 0) {

			return downloadPhoneBook(tokens[CommandType_Service], tokens[CommandType_Command]);

        } else if (tokens[CommandType_Command].find("DOWNLOAD CALL LOG") == 0) {

			return  downloadCallHistory(tokens[CommandType_Service], tokens[CommandType_Command]);

        }
	}

loop_finish:
	// if command is mismatch, run "NG" reply command.
	m_IPCServer.write_broadcast( (char*)( MakeReplyCommand(false, tokens).c_str() ) );
	return false;
}

std::vector<std::string> NxBTService::createTokensFromCommand(char* command)
{
    std::string target = (std::string)command;
    std::vector<std::string> tokens;
    std::string token;
    int stx, etx;

    stx = target.find("$");
    if (stx < 0) {
#if __dmesg__
        fprintf(stderr, "[ERROR]: STX no detect!\n");
#endif
        goto error_occur;
    }

    etx = target.find("\n", stx);
    if (etx < 0) {
#if __dmesg__
        fprintf(stderr, "[ERROR]: ETX no detect!\n");
#endif
        goto error_occur;
    }

    for (int i = stx+1; i < etx; i++) {
        if (target[i] == '#') {
            tokens.push_back(token);
            token.clear();
            continue;
        }

        token += target[i];
    }

    if (token.length()) {
        tokens.push_back(token);
    }

error_occur:
    return tokens;
}



std::string NxBTService::bdAddrToString(char* bd_addr, int len, char seperator)
{
    char buffer[BUFFER_SIZE] = {0,};

    if (len < DEVICE_ADDRESS_SIZE)
        return std::string();

    sprintf(buffer, "%02x%c%02x%c%02x%c%02x%c%02x%c%02x", bd_addr[0], seperator, bd_addr[1], seperator, bd_addr[2], seperator, bd_addr[3], seperator, bd_addr[4], seperator, bd_addr[5]);

    return (std::string)buffer;
}

//-----------------------------------------------------------------------
// MGT functions
bool NxBTService::ping(std::string service, std::string command)
{
	bool result = m_bInitialized;
	std::vector<std::string> reply;
	reply.push_back(service);
	reply.push_back(command);
	m_IPCServer.write_broadcast( (char*)( MakeReplyCommand(result, reply).c_str() ) );

	return result;
}

bool NxBTService::setEnableAutoConnection(std::string service, std::string command)
{
	// example 1) result = "$OK#ENABLE AUTO CONNECTION ON"
	// example 2) result = "$OK#ENABLE AUTO CONNECTION OFF"

	std::vector<std::string> reply;
	std::string argument = findArgument(&command);
	bool result = false;

	if (argument == "ON")
		result = (m_pModel->enableAutoConnection(true) == RET_OK);
	else if (argument == "OFF")
		result = (m_pModel->enableAutoConnection(false) == RET_OK);

	reply.push_back(service);
	reply.push_back(command);

	m_IPCServer.write_broadcast( (char*)( MakeReplyCommand(result, reply).c_str() ) );

	return result;
}

bool NxBTService::isEnabledAutoConnection(std::string service, std::string command)
{
	// example 1) result = "$OK#AUTO CONNECTION#ON\n"
	// example 2) result = "$OK#AUTO CONNECTION#OFF\n"

	std::vector<std::string> reply;
	bool result = true;

	reply.push_back(service);
	reply.push_back(command);
	reply.push_back(m_pModel->isAutoConnection() ? "ON" : "OFF");

	m_IPCServer.write_broadcast( (char*)( MakeReplyCommand(result, reply).c_str() ) );

	return result;
}

bool NxBTService::setEnableAutoPairing(std::string service, std::string command)
{
	// example 1) result = "$OK#ENABLE AUTO PAIRING ON"
	// example 2) result = "$OK#ENABLE AUTO PAIRING OFF"
fprintf(stderr, "\n\n\n %s \n\n\n", command.c_str());
	std::vector<std::string> reply;
	std::string argument = findArgument(&command);
	bool result = false;

	if (argument == "ON") {
		fprintf(stderr, "\n\n\n %s = %d \n\n\n", command.c_str(), m_pModel->enableAutoPairing(true));
		result = (m_pModel->enableAutoPairing(true) == RET_OK);

	} else if (argument == "OFF") {
				fprintf(stderr, "\n\n\n %s = %d \n\n\n", command.c_str(), m_pModel->enableAutoPairing(false));
		result = (m_pModel->enableAutoPairing(false) == RET_OK);
	}

	reply.push_back(service);
	reply.push_back(command);

	m_IPCServer.write_broadcast( (char*)( MakeReplyCommand(result, reply).c_str() ) );

	return result;
}

bool NxBTService::isEnabledAutoPairing(std::string service, std::string command)
{
	// example 1) result = "$OK#AUTO PAIRING#ON\n"
	// example 2) result = "$OK#AUTO PAIRING#OFF\n"

	std::vector<std::string> reply;
	bool result = true;

	reply.push_back(service);
	reply.push_back(command);
	reply.push_back(m_pModel->isAutoPairing() ? "ON" : "OFF");

	m_IPCServer.write_broadcast( (char*)( MakeReplyCommand(result, reply).c_str() ) );

	return result;
}

bool NxBTService::acceptPairing(std::string service, std::string command)
{
	// example) result = "$OK#ACCEPT PAIRING\n"

	std::vector<std::string> reply;
	bool result = (m_pModel->acceptPairing() == RET_OK);

	reply.push_back(service);
	reply.push_back(command);

	m_IPCServer.write_broadcast( (char*)( MakeReplyCommand(result, reply).c_str() ) );

	return result;
}

bool NxBTService::rejectPairing(std::string service, std::string command)
{
	// example) result = "$OK#REJECT PAIRING\n"

	std::vector<std::string> reply;
	bool result = (m_pModel->rejectPairing() == RET_OK);

	reply.push_back(service);
	reply.push_back(command);

	m_IPCServer.write_broadcast( (char*)( MakeReplyCommand(result, reply).c_str() ) );

	return result;
}

bool NxBTService::unpairAll(std::string service, std::string command)
{
	// example) result = "$OK#MGT#UNPAIR DEVICE ALL\n"

	std::vector<std::string> reply;
	bool result = true;

	for (int32_t i = m_pModel->getPairedDevCount()-1; i >= 0; --i) {
		if (0 == m_pModel->unpairDevice(i))
			result = false;
	}

	reply.push_back(service);
	reply.push_back(command);

	m_IPCServer.write_broadcast( (char*)( MakeReplyCommand(result, reply).c_str() ) );

	return result;
}

bool NxBTService::unpair(std::string service, std::string command)
{
	// example) result = "$OK#UNPAIR DEVICE 3\n"

	std::vector<std::string> reply;
	std::string argument = findArgument(&command);
	bool result = isDigit(argument) && ( m_pModel->unpairDevice( atoi(argument.c_str()) ) == RET_OK );

	reply.push_back(service);
	reply.push_back(command);

	m_IPCServer.write_broadcast( (char*)( MakeReplyCommand(result, reply).c_str() ) );

	return result;
}

bool NxBTService::localDeviceName(std::string service, std::string command)
{
	// example) result = "$OK#LOCAL DEVICE NAME#NEXELL-BT-G\n"

	std::vector<std::string> reply;
	bool result = true;

	reply.push_back(service);
	reply.push_back(command);
	reply.push_back(m_pModel->getLocalDevName());

	m_IPCServer.write_broadcast( (char*)( MakeReplyCommand(result, reply).c_str() ) );

	return result;
}

bool NxBTService::localDeviceAddress(std::string service, std::string command)
{
	std::vector<std::string> reply;
	bool result = true;

	reply.push_back(service);
	reply.push_back(command);
	reply.push_back(bdAddrToString(m_pModel->getLocalAddress(), DEVICE_ADDRESS_SIZE, ':'));

	m_IPCServer.write_broadcast( (char*)( MakeReplyCommand(result, reply).c_str() ) );

	return result;
}

bool NxBTService::renameLocalDevice(std::string service, std::string command)
{
	std::vector<std::string> reply;
	std::string argument = findArgument(&command);

	bool result = ( !argument.empty() && m_pModel->renameLocalDevice(argument.c_str()) == 0 );

	reply.push_back(service);
	reply.push_back(command);

	m_IPCServer.write_broadcast( (char*)( MakeReplyCommand(result, reply).c_str() ) );

	return result;
}

bool NxBTService::countOfPairedDevice(std::string service, std::string command)
{
	std::vector<std::string> reply;
	bool result = true;

	reply.push_back(service);
	reply.push_back(command);
	reply.push_back(std::to_string(m_pModel->getPairedDevCount()));

	m_IPCServer.write_broadcast( (char*)( MakeReplyCommand(result, reply).c_str() ) );

	return result;
}

bool NxBTService::infoListOfPairedDeviceAll(std::string service, std::string command)
{
	std::vector<std::string> reply;
	std::string temp;
	int32_t count = m_pModel->getPairedDevCount();
	char name[DEVICE_NAME_SIZE] = {0,};
	char bd_addr[DEVICE_ADDRESS_SIZE] = {0,};
	bool avk_connected = false;
	bool hs_connected = false;
	bool result = true;

	reply.push_back(service);
	reply.push_back(command);

	for (int32_t i = 0; i < count; i++) {
		if (0 > m_pModel->getPairedDevInfoByIndex(i, name, bd_addr))
			continue;

		temp = bdAddrToString(bd_addr, DEVICE_ADDRESS_SIZE, ':');

		avk_connected = ( temp == bdAddrToString(m_AVK.connection.bd_addr, DEVICE_ADDRESS_SIZE, ':') && m_AVK.connection.on );
		hs_connected = ( temp == bdAddrToString(m_HS.hs.bd_addr, DEVICE_ADDRESS_SIZE, ':') && m_HS.hs.on );

		temp = "<" + (std::string)name + "," + bdAddrToString(bd_addr, DEVICE_ADDRESS_SIZE, ':') + "," + (avk_connected ? "CONNECTED" : "DISCONNECTED") + "," + (hs_connected ? "CONNECTED" : "DISCONNECTED") +">";
		reply.push_back(temp);
	}

	m_IPCServer.write_broadcast( (char*)( MakeReplyCommand(result, reply).c_str() ) );

	return result;
}

bool NxBTService::infoOfPairedDeviceByIndex(std::string service, std::string command)
{
	std::vector<std::string> reply;
	std::string argument = findArgument(&command);
	std::string temp;
	char name[DEVICE_NAME_SIZE] = {0,};
	char bd_addr[DEVICE_ADDRESS_SIZE] = {0,};
	bool result = ( isDigit(argument) && m_pModel->getPairedDevInfoByIndex( atoi(argument.c_str()), name, bd_addr ) == RET_OK);

	if (result) {
		temp = "<" + (std::string)name + "," + bdAddrToString(bd_addr, DEVICE_ADDRESS_SIZE, ':') + ">";
	}

	reply.push_back(service);
	reply.push_back(command);
	reply.push_back(temp);

	m_IPCServer.write_broadcast( (char*)( MakeReplyCommand(result, reply).c_str() ) );

	return result;
}

bool NxBTService::nameOfPairedDeviceByIndex(std::string service, std::string command)
{
	std::vector<std::string> reply;
	std::string argument = findArgument(&command);
	char name[DEVICE_NAME_SIZE] = {0,};
	bool result = ( isDigit(argument) && m_pModel->getPairedDevNameByIndex( atoi(argument.c_str()), name ) == RET_OK );

	reply.push_back(service);
	reply.push_back(command);

	if (result) {
		reply.push_back((std::string)name);
	}

	m_IPCServer.write_broadcast( (char*)( MakeReplyCommand(result, reply).c_str() ) );

	return result;
}

bool NxBTService::addressOfPairedDeviceByIndex(std::string service, std::string command)
{
	std::vector<std::string> reply;
	std::string argument = findArgument(&command);
	char bd_addr[DEVICE_ADDRESS_SIZE] = {0,};
	bool result = ( isDigit(argument) && m_pModel->getPairedDevAddrByIndex( atoi(argument.c_str()), bd_addr ) == RET_OK );

	reply.push_back(service);
	reply.push_back(command);

	if (result) {
		reply.push_back(bdAddrToString(bd_addr, DEVICE_ADDRESS_SIZE, ':'));
	}

	m_IPCServer.write_broadcast( (char*)( MakeReplyCommand(result, reply).c_str() ) );

	return result;
}

bool NxBTService::connectToAVK(std::string service, std::string command)
{
	// example) result = "$OK#AVK#CONNECT 3\n"
	std::vector<std::string> reply;
	std::string argument = findArgument(&command);
	int32_t index = atoi(argument.c_str());
	bool result = ( m_pModel->isConnectedAVK() || ( isDigit(argument) && m_pModel->connectToAVK(index) == RET_OK ) );

	if (result) {
		m_AVK.connection.on = true;
		m_AVK.connection.index = index;
		m_pModel->getPairedDevInfoByIndex(index, m_AVK.connection.name, m_AVK.connection.bd_addr);
	}

	reply.push_back(service);
	reply.push_back(command);

	m_IPCServer.write_broadcast( (char*)( MakeReplyCommand(result, reply).c_str() ) );

	return result;
}

bool NxBTService::disconnectToAVK(std::string service, std::string command)
{
	std::vector<std::string> reply;
	char bd_addr[6] = {0,};
	bool already_disconnected = !m_pModel->isConnectedAVK();
	bool result = true;

	if ( ! already_disconnected )
	{
		m_pModel->getConnectionDevAddrAVK(AVK_CONNECTED_INDEX, bd_addr);
		result = (m_pModel->disconnectFromAVK(bd_addr) == RET_OK);
	}

	reply.push_back(service);
	reply.push_back(command);

	if (result)
	{
		struct connect_state_t t;
		t.on = false;
		t.index = -1;
		memset(t.name, 0, DEVICE_NAME_SIZE);
		memset(t.bd_addr, 0, DEVICE_ADDRESS_SIZE);

		updateAVKConnectionState(t);

		if ( ! already_disconnected )
		{
			reply.push_back(bdAddrToString(bd_addr, DEVICE_ADDRESS_SIZE, ':'));
		}
	}

	m_IPCServer.write_broadcast( (char*)( MakeReplyCommand(result, reply).c_str() ) );

	return result;
}

void NxBTService::updateAVKConnectionState(struct connect_state_t target)
{
	if (m_AVK.connection.on != target.on) {
		m_AVK.connection.on = target.on;
	}

	if (m_AVK.connection.index != target.index) {
		m_AVK.connection.index = target.index;
	}

	if (strcmp(m_AVK.connection.name, target.name) != 0) {
		memcpy(m_AVK.connection.name, target.name, DEVICE_ADDRESS_SIZE);
	}

	if (strcmp(m_AVK.connection.bd_addr, target.bd_addr) == 0) {
		memcpy(m_AVK.connection.bd_addr, target.bd_addr, DEVICE_ADDRESS_SIZE);
	}
}

bool NxBTService::isConnectedToAVK(string service, string command)
{
	// example 1) $OK#AVK#IS CONNECTED#CONNECTED
	// example 2) $OK#AVK#IS CONNECTED#DISCONNECTED

	std::vector<std::string> reply;
	bool result = true;

	reply.push_back(service);
	reply.push_back(command);
	reply.push_back(m_pModel->isConnectedAVK() ? "CONNECTED" : "DISCONNECTED");

	m_IPCServer.write_broadcast( (char*)( MakeReplyCommand(result, reply).c_str() ) );

	return result;
}

bool NxBTService::indexOfConnectedDeviceToAVK(std::string service, std::string command)
{
	std::vector<std::string> reply;
	bool result = (m_AVK.connection.index >= 0);

	reply.push_back(service);
	reply.push_back(command);

	if (result) {
		reply.push_back(std::to_string(m_AVK.connection.index));
	}

	m_IPCServer.write_broadcast( (char*)( MakeReplyCommand(result, reply).c_str() ) );

	return result;
}

bool NxBTService::addressOfConnectedDeviceToAVK(std::string service, std::string command)
{
	std::vector<std::string> reply;
	char bd_addr[DEVICE_ADDRESS_SIZE] = {0,};
	bool result = (m_pModel->getConnectionDevAddrAVK(AVK_CONNECTED_INDEX, bd_addr) == RET_OK);

	reply.push_back(service);
	reply.push_back(command);

	if (result) {
		reply.push_back(bdAddrToString(bd_addr, DEVICE_ADDRESS_SIZE, ':'));
	}

	m_IPCServer.write_broadcast( (char*)( MakeReplyCommand(result, reply).c_str() ) );

	return result;
}

bool NxBTService::playStartAVK(std::string service/*= "AVK"*/, std::string command/*= "PLAY START"*/)
{
	std::vector<std::string> reply;
	char bd_addr[6];
	bool result = true;

	reply.push_back(service);
	reply.push_back(command);

	result = !(m_pModel->getConnectionDevAddrAVK(AVK_CONNECTED_INDEX, bd_addr) < 0 || m_pModel->playStartAVK(bd_addr) < 0);

	if (result)
		reply.push_back(bdAddrToString(bd_addr, DEVICE_ADDRESS_SIZE, ':'));

	m_IPCServer.write_broadcast( (char*)( MakeReplyCommand(result, reply).c_str() ) );
	return result;
}

bool NxBTService::playStopAVK(std::string service/*= "AVK"*/, std::string command/*= "PLAY STOP"*/)
{
	std::vector<std::string> reply;
	char bd_addr[6];
	bool result = true;

	reply.push_back(service);
	reply.push_back(command);

	result = !(m_pModel->getConnectionDevAddrAVK(AVK_CONNECTED_INDEX, bd_addr) < 0 || m_pModel->playStopAVK(bd_addr) < 0);

	if (result)
		reply.push_back(bdAddrToString(bd_addr, DEVICE_ADDRESS_SIZE, ':'));

	m_IPCServer.write_broadcast( (char*)( MakeReplyCommand(result, reply).c_str() ) );
	return result;
}

bool NxBTService::playPauseAVK(std::string service/*= "AVK"*/, std::string command/*= "PLAY PAUSE"*/)
{
	std::vector<std::string> reply;
	char bd_addr[6];
	bool result = true;

	reply.push_back(service);
	reply.push_back(command);

	result = !(m_pModel->getConnectionDevAddrAVK(AVK_CONNECTED_INDEX, bd_addr) < 0 || m_pModel->playPauseAVK(bd_addr) < 0);

	if (result)
		reply.push_back(bdAddrToString(bd_addr, DEVICE_ADDRESS_SIZE, ':'));

	m_IPCServer.write_broadcast( (char*)( MakeReplyCommand(result, reply).c_str() ) );
	return result;
}

bool NxBTService::playPrevAVK(std::string service/*= "AVK"*/, std::string command/*= "PLAY PREV"*/)
{
	std::vector<std::string> reply;
	char bd_addr[6];
	bool result = true;

	reply.push_back(service);
	reply.push_back(command);

	result = !(m_pModel->getConnectionDevAddrAVK(AVK_CONNECTED_INDEX, bd_addr) < 0 || m_pModel->playPrevAVK(bd_addr) < 0);

	if (result)
		reply.push_back(bdAddrToString(bd_addr, DEVICE_ADDRESS_SIZE, ':'));

	m_IPCServer.write_broadcast( (char*)( MakeReplyCommand(result, reply).c_str() ) );
	return result;
}

bool NxBTService::playNextAVK(std::string service/*= "AVK"*/, std::string command/*= "PLAY NEXT"*/)
{
	std::vector<std::string> reply;
	char bd_addr[6];
	bool result = true;

	reply.push_back(service);
	reply.push_back(command);

	result = !(m_pModel->getConnectionDevAddrAVK(AVK_CONNECTED_INDEX, bd_addr) < 0 || m_pModel->playNextAVK(bd_addr) < 0);

	if (result)
		reply.push_back(bdAddrToString(bd_addr, DEVICE_ADDRESS_SIZE, ':'));

	m_IPCServer.write_broadcast( (char*)( MakeReplyCommand(result, reply).c_str() ) );
	return result;
}

bool NxBTService::playStatusAVK(std::string service/*= "AVK"*/, std::string command/*= "PLAY STATUS"*/)
{
	std::vector<std::string> reply;
	bool result = true;

	reply.push_back(service);
	reply.push_back(command);

	switch (m_AVK.status) {
	case PlayStatus_Stopped: // 0x00
		reply.push_back("STOPPED");
		break;

	case PlayStatus_Playing: // 0x01
		reply.push_back("PLAYING");
		break;

	case PlayStatus_Paused: // 0x02
		reply.push_back("PAUSED");
		break;
	}

	m_IPCServer.write_broadcast( (char*)( MakeReplyCommand(result, reply).c_str() ) );
	return result;
}

bool NxBTService::playInfoAVK(std::string service/*= "AVK"*/, std::string command/*= "PLAT INFO"*/)
{
	// example) "$OK#AVK#PLAY INFO#[title]#[artist]#[album]#[genre]#[duration]#[play position]\n"
	std::vector<std::string> reply;
	bool result = true;

	reply.push_back(service);
	reply.push_back(command);
	reply.push_back(m_AVK.info.title);
	reply.push_back(m_AVK.info.artist);
	reply.push_back(m_AVK.info.album);
	reply.push_back(m_AVK.info.genre);
	reply.push_back(std::to_string(m_AVK.info.duration));
	reply.push_back(std::to_string(m_AVK.info.position));

	m_IPCServer.write_broadcast( (char*)( MakeReplyCommand(result, reply).c_str() ) );
	return result;
}

bool NxBTService::openAudioAVK(std::string service/*= "AVK"*/, std::string command/*= "OPEN AUDIO"*/)
{
	std::vector<std::string> reply;
	bool result = true;

	reply.push_back(service);
	reply.push_back(command);

	result = (m_pModel->isAudioStatusAVK() || m_pModel->openAudioAVK() == RET_OK);

	m_IPCServer.write_broadcast( (char*)( MakeReplyCommand(result, reply).c_str() ) );
	return result;
}

bool NxBTService::closeAudioAVK(std::string service/*= "AVK"*/, std::string command/*= "CLOSE AUDIO"*/)
{
	std::vector<std::string> reply;
	bool result = true;

	reply.push_back(service);
	reply.push_back(command);

	if (m_pModel->isAudioStatusAVK())
		m_pModel->closeAudioAVK();

	m_IPCServer.write_broadcast( (char*)( MakeReplyCommand(result, reply).c_str() ) );
	return result;
}

//-----------------------------------------------------------------------
// HS functions
bool NxBTService::connectToHS(std::string service, std::string command)
{
	// example) result = "$OK#HS#CONNECT 3\n"
	std::vector<std::string> reply;
	std::string argument = findArgument(&command);
	int32_t index = atoi(argument.c_str());
	bool result = (isDigit(argument) && m_pModel->connectToHS(index) == RET_OK);

	if (result) {
		m_HS.hs.on = true;
		m_HS.hs.index = index;

		m_pModel->getPairedDevInfoByIndex(index, m_HS.hs.name, m_HS.hs.bd_addr);
	}

	reply.push_back(service);
	reply.push_back(command);

	m_IPCServer.write_broadcast( (char*)( MakeReplyCommand(result, reply).c_str() ) );

	return result;
}

bool NxBTService::disconnectFromHS(std::string service, std::string command)
{
	std::vector<std::string> reply;
	bool result = (m_pModel->disconnectFromHS() == RET_OK);

	if (result) {
		m_HS.hs.on = false;
		m_HS.hs.index = -1;
	}

	reply.push_back(service);
	reply.push_back(command);

	m_IPCServer.write_broadcast( (char*)( MakeReplyCommand(result, reply).c_str() ) );

	return result;
}

bool NxBTService::isConnectedToHS(std::string service, std::string command)
{
	std::vector<std::string> reply;
	bool result = true;

	reply.push_back(service);
	reply.push_back(command);
	reply.push_back(m_pModel->isConnectedHS() ? "CONNECTED" : "DISCONNECTED");

	m_IPCServer.write_broadcast( (char*)( MakeReplyCommand(result, reply).c_str() ) );

	return result;
}

bool NxBTService::indexOfConnectedDeviceToHS(std::string service, std::string command)
{
	std::vector<std::string> reply;
	bool result = (m_HS.hs.index >= 0);

	reply.push_back(service);
	reply.push_back(command);

	if (result) {
		reply.push_back(std::to_string(m_HS.hs.index));
	}

	m_IPCServer.write_broadcast( (char*)( MakeReplyCommand(result, reply).c_str() ) );

	return result;
}

bool NxBTService::dialPhoneNumber(std::string service, std::string command)
{
	std::vector<std::string> reply;
	std::string argument = findArgument(&command);
	bool result = isDigit(argument) && ( m_pModel->dialPhoneNumber(argument.c_str()) == RET_OK );

	reply.push_back(service);
	reply.push_back(command);

	m_IPCServer.write_broadcast( (char*)( MakeReplyCommand(result, reply).c_str() ) );

	return result;
}

bool NxBTService::reDialPhoneNumber(std::string service, std::string command)
{
	std::vector<std::string> reply;
	bool result = m_pModel->reDialPhoneNumber();

	reply.push_back(service);
	reply.push_back(command);

	m_IPCServer.write_broadcast( (char*)( MakeReplyCommand(result, reply).c_str() ) );

	return result;
}

bool NxBTService::pickUpCall(std::string service, std::string command)
{
	std::vector<std::string> reply;
	bool result = (m_pModel->pickUpCall() == RET_OK);

	reply.push_back(service);
	reply.push_back(command);

	m_IPCServer.write_broadcast( (char*)( MakeReplyCommand(result, reply).c_str() ) );

	return result;
}

bool NxBTService::hangUpCall(std::string service, std::string command)
{
	std::vector<std::string> reply;
	bool result = (m_pModel->hangUpCall() == RET_OK);

	reply.push_back(service);
	reply.push_back(command);

	m_IPCServer.write_broadcast( (char*)( MakeReplyCommand(result, reply).c_str() ) );

	return result;
}

bool NxBTService::muteMicrophoneHS(std::string service, std::string command)
{
	std::vector<std::string> reply;
	std::string argument = findArgument(&command);
	bool result = true;

	if (argument == "ON")
		m_pModel->muteMicrophoneHS(true);
	else if (argument == "OFF")
		m_pModel->muteMicrophoneHS(false);
	else
		result = false;

	reply.push_back(service);
	reply.push_back(command);

	m_IPCServer.write_broadcast( (char*)( MakeReplyCommand(result, reply).c_str() ) );

	return result;
}

bool NxBTService::openAudioHS(std::string service, std::string command)
{
	std::vector<std::string> reply;
	bool result = ( m_pModel->isOpenedAudioHS() || (m_pModel->openAudioHS() == RET_OK) );

	reply.push_back(service);
	reply.push_back(command);

	m_IPCServer.write_broadcast( (char*)( MakeReplyCommand(result, reply).c_str() ) );

	return result;
}

bool NxBTService::closeAudioHS(std::string service, std::string command)
{
	std::vector<std::string> reply;
	bool result = ( !m_pModel->isOpenedAudioHS() || m_pModel->closeAudioHS() == RET_OK);

	reply.push_back(service);
	reply.push_back(command);

	m_IPCServer.write_broadcast( (char*)( MakeReplyCommand(result, reply).c_str() ) );

	return result;
}

//-----------------------------------------------------------------------
// MCE functions
bool NxBTService::connectToMCE(std::string service, std::string command)
{
	// example) result = "$OK#MCE#CONNECT 3\n"
	std::vector<std::string> reply;
	std::string argument = findArgument(&command);
	int32_t index = atoi(argument.c_str());
	bool result = (isDigit(argument) && m_pModel->connectToMCE(index) == RET_OK);

	if (result) {
		m_HS.mce.on = true;
		m_HS.mce.index = index;

		m_pModel->getPairedDevInfoByIndex(index, m_HS.mce.name, m_HS.mce.bd_addr);
	}

	reply.push_back(service);
	reply.push_back(command);

	m_IPCServer.write_broadcast( (char*)( MakeReplyCommand(result, reply).c_str() ) );

	return result;
}

bool NxBTService::disconnectFromMCE(std::string service, std::string command)
{
	std::vector<std::string> reply;
	bool result = (m_pModel->disconnectFromMCE() == RET_OK);

	if (result) {
		m_HS.mce.on = false;
		m_HS.mce.index = -1;
	}

	reply.push_back(service);
	reply.push_back(command);

	m_IPCServer.write_broadcast( (char*)( MakeReplyCommand(result, reply).c_str() ) );

	return result;
}

bool NxBTService::isConnectedToMCE(std::string service, std::string command)
{
	std::vector<std::string> reply;
	bool result = true;

	reply.push_back(service);
	reply.push_back(command);
	reply.push_back(m_pModel->isConnectedMCE() ? "CONNECTED" : "DISCONNECTED");

	m_IPCServer.write_broadcast( (char*)( MakeReplyCommand(result, reply).c_str() ) );

	return result;
}

bool NxBTService::downloadSMSMessage(std::string service, std::string command)
{
	std::vector<std::string> reply;
	char name[20] = {0,};
	char phonenumber[20] = {0,};
	char message[256] = {0,};
	bool result = (m_pModel->getParserBmsg(name, phonenumber, message) == RET_OK);

	reply.push_back(service);
	reply.push_back(command);

	if (result) {
		reply.push_back(name);
		reply.push_back(phonenumber);
		reply.push_back(message);
	}

	m_IPCServer.write_broadcast( (char*)( MakeReplyCommand(result, reply).c_str() ) );

	return result;
}

//-----------------------------------------------------------------------
// PBC functions
bool NxBTService::connectToPBC(std::string service, std::string command)
{
	// example) result = "$OK#PBC#CONNECT 3\n"
	std::vector<std::string> reply;
	std::string argument = findArgument(&command);
	int32_t index = atoi(argument.c_str());
	bool result = (isDigit(argument) && m_pModel->connectToPBC(index) == RET_OK);

	if (result) {
		m_HS.pbc.connection.on = true;
		m_HS.pbc.connection.index = index;

		m_pModel->getPairedDevInfoByIndex(index, m_HS.pbc.connection.name, m_HS.pbc.connection.bd_addr);
	}

	reply.push_back(service);
	reply.push_back(command);

	m_IPCServer.write_broadcast( (char*)( MakeReplyCommand(result, reply).c_str() ) );

	return result;
}

bool NxBTService::disconnectFromPBC(std::string service, std::string command)
{
	std::vector<std::string> reply;
	bool result = (m_pModel->disconnectFromPBC() == RET_OK);

	if (result) {
		m_HS.pbc.connection.on = false;
		m_HS.pbc.connection.index = -1;
	}

	reply.push_back(service);
	reply.push_back(command);

	m_IPCServer.write_broadcast( (char*)( MakeReplyCommand(result, reply).c_str() ) );

	return result;
}

bool NxBTService::isConnectedToPBC(std::string service, std::string command)
{
	std::vector<std::string> reply;
	bool result = true;

	reply.push_back(service);
	reply.push_back(command);
	reply.push_back(m_pModel->isConnectedPBC() ? "CONNECTED" : "DISCONNECTED");

	m_IPCServer.write_broadcast( (char*)( MakeReplyCommand(result, reply).c_str() ) );

	return result;
}

bool NxBTService::downloadPhoneBook(string service, string command)
{
	std::vector<std::string> reply;
	bool result = true;

	m_HS.pbc.download = DownloadType_PhoneBook;
	result = (m_pModel->getContactFromPBC() == RET_OK);

	reply.push_back(service);
	reply.push_back(command);

	m_IPCServer.write_broadcast( (char*)( MakeReplyCommand(result, reply).c_str() ) );

	return result;
}

bool NxBTService::downloadCallHistory(string service, string command)
{
	std::vector<std::string> reply;
	bool result = true;

	m_HS.pbc.download = DownloadType_CallHistory;
	result = (m_pModel->getCallHistoryFromPBC() == RET_OK);

	reply.push_back(service);
	reply.push_back(command);

	m_IPCServer.write_broadcast( (char*)( MakeReplyCommand(result, reply).c_str() ) );

	return result;
}

//-----------------------------------------------------------------------
// Utility functions
bool NxBTService::isDigit(std::string text)
{
	if (text.empty())
		return false;

	for (size_t i = 0; i < text.length(); i++) {
		if (text[i] < '0' || '9' < text[i])
			return false;
	}

	return true;
}

std::string NxBTService::MakeReplyCommand(bool ok, std::vector<std::string> reply)
{
	std::string command;

	command += COMMAND_FORMAT_STX;
	command += ok ? COMMAND_FORMAT_REPLY_DONE : COMMAND_FORMAT_REPLY_FAIL;

	for (size_t i = 0; i < reply.size(); i++) {
		command += COMMAND_FORMAT_SEPERATOR + reply[i];
	}

	command += COMMAND_FORMAT_ETX;

	return command;
}

std::string NxBTService::findArgument(std::string* command)
{
	size_t pos = command->rfind(" ");
	if (pos == std::string::npos)
		return std::string();

	++pos;

	return command->substr(pos, command->length()-pos);
}

bool NxBTService::findArgument(std::string* command, std::string target, std::string* argument)
{
	argument->clear();

	for (size_t i = strlen(target.c_str()); i < command->length(); i++) {
		*argument += command->at(i);
	}

	return !argument->empty();
}

std::vector<std::string> NxBTService::createTokens(std::string text, char seperator, char stx/*= 0*/, char etx/*= 0*/)
{
	std::vector<std::string> tokens;
	std::string token;
	int start = 0, end = text.length();

	if (stx) {
		start = text.find(stx);

		if (start < 0) {
			goto loop_finish;
		}
	}

	if (etx) {
		end = text.find(etx, stx);

		if (end < 0) {
			goto loop_finish;
		}
	}

	for (int i = start; i < end; i++) {
		if (text[i] == seperator) {
			tokens.push_back(token);
			token.clear();
			continue;
		}

		token += text[i];
	}

	if (token.length()) {
		tokens.push_back(token);
	}

loop_finish:
	return tokens;
}

bool NxBTService::findPID(std::string name, std::vector<int32_t>& pids)
{
	std::string command;
	FILE* fp = NULL;
	char buffer[BUFFER_SIZE] = {0,};
	std::vector<std::string> tokens;

	pids.clear();

	// if name is invalid(empty) data, finish loop.
	if (name.empty())
		return false;

	// set command
	command = "pidof " + name;

	// run command
	fp = popen(command.c_str(), "r");
	if (fp) {
		while ( fgets(buffer, BUFFER_SIZE, fp) );
		pclose(fp);
	}

	// split form command result <seperator : ' ' (0x20, white space)>.
	tokens = createTokens((std::string)buffer, ' ');

	// if last character is LF, remove LF
	if (tokens.size()) {
		int32_t last_1 = tokens.size()-1;
		int32_t last_2 = tokens[last_1].length()-1;
		if (tokens[last_1][last_2] == '\n')
			tokens[last_1] = tokens[last_1].substr(0, last_2);
	}

	// fill pids.
	for (size_t i = 0; i < tokens.size(); ++i) {
		if (!isDigit(tokens[i]))
			continue;

		pids.push_back( atoi(tokens[i].c_str()) );
	}

	// if pids size is not zero, return true
	// otherwise return false.
	return (pids.size() > 0);
}

void* NxBTService::testAudioFocusThread(void* args)
{
	(void)args;

	while (1) {
		printf("----------------------------------------------------------------\n");
		printf("g_calling_mode_on = %s\n", g_calling_mode_on ? "TRUE" : "FALSE");
		printf("g_has_audio_focus = %s\n", g_has_audio_focus ? "TRUE" : "FALSE");
		printf("g_has_audio_focus_transient = %s\n", g_has_audio_focus_transient ? "TRUE" : "FALSE");
		printf("----------------------------------------------------------------\n");

		usleep(100000);
	}

	return NULL;
}

bool NxBTService::isInitialized()
{
	return m_bInitialized;
}

void NxBTService::setInitialized(bool state)
{
	if (m_bInitialized) {
		return;
	}

	m_bInitialized = state;
	if (m_bInitialized) {
		// Set ALSA device names
		m_pModel->setALSADevName(NX_ALSA_DEV_NAME_P, NX_ALSA_DEV_NAME_C, NX_ALSA_BT_DEV_NAME_P, NX_ALSA_BT_DEV_NAME_C, true);

		// Auto connection
		m_pModel->autoConnection(m_pModel->isAutoConnection());

		ping("MGT", "PING");
	}
}
