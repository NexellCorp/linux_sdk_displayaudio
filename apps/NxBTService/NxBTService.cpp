#include "NxBTService.h"

#define LOG_TAG "[NxBTService]"
#include <NX_Log.h>

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

#define RET_OK 0
#define RET_FAIL -1

NxBTService *NxBTService::m_spInstance = NULL;

// Audio Focus
void (*NxBTService::m_pRequestAudioFocus)(FocusPriority ePriority, bool *bOk) = NULL;
void (*NxBTService::m_pRequestAudioFocusTransient)(FocusPriority ePriority, bool *bOk) = NULL;
void (*NxBTService::m_pRequestAudoFocusLoss)() = NULL;
// Plug-in Run/Terminate
void (*NxBTService::m_pRequestPlugInRun)(const char *pPlugin, const char *pArgs) = NULL;
void (*NxBTService::m_pRequestPlugInTerminate)(const char *pPlugin) = NULL;
void (*NxBTService::m_pRequestPlugInIsRunning)(const char *pPlugin, bool *bOk) = NULL;
// Send Message
void (*NxBTService::m_pRequestSendMessage)(const char *pDst, const char *pMsg, int32_t iMsgSize) = NULL;
// Popup Message
void (*NxBTService::m_pRequestPopupMessage)(PopupMessage *, bool *) = NULL;
void (*NxBTService::m_pRequestExpirePopupMessage)() = NULL;
//
void (*NxBTService::m_pRequestNotification)(PopupMessage *) = NULL;
void (*NxBTService::m_pRequestExpireNotification)() = NULL;

bool NxBTService::g_calling_mode_on = false;
bool NxBTService::g_has_audio_focus = false;
bool NxBTService::g_has_audio_focus_transient = false;

NxBTService* NxBTService::GetInstance(void *pObj)
{
	(void)pObj;

	if (!m_spInstance) {
		m_spInstance = new NxBTService();
	}

	return m_spInstance;
}

NxBTService* NxBTService::GetInstance()
{
	return m_spInstance;
}

void NxBTService::DestroyInstance()
{
	if (m_spInstance) {
		delete m_spInstance;
		m_spInstance = NULL;
	}
}

/*
 * MGT service callback functions
 */
void NxBTService::sendMGTOpenSucceed_stub(void* pObj, int32_t result)
{
	NxBTService* self = (NxBTService *)pObj;

	if (result < 0) {
		DestroyInstance();
	} else {
		if (self) {
			self->setInitialized(result == 0);
		}
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

	if (self->m_pRequestExpirePopupMessage) {
		self->m_pRequestExpirePopupMessage();
	}
}

void NxBTService::updatePairedDevices_stub(void *pObj)
{
	NxBTService* self = (NxBTService*)pObj;
	char buffer[BUFFER_SIZE];
	int count = m_pModel->getPairedDevCount();
	char name[249] = {0,};
	unsigned char bd_addr[6] = {0,};
	char avk_connection[100] = {0,};
	char hs_connection[100] = {0,};
	std::string s_bd_addr;

	sprintf(buffer, "$OK#%s#%s", "MGT", "PAIRED DEVICE INFO ALL LIST");

	for (int i = 0; i < count; i++) {
		if (0 > m_pModel->getPairedDevInfoByIndex(i, name, bd_addr))
			continue;

		if (memcmp(self->m_AVK.connection.bd_addr, bd_addr, sizeof(bd_addr)) == 0 && self->m_AVK.connection.on) {
			strcpy(avk_connection, "CONNECTED");
		} else {
			strcpy(avk_connection, "DISCONNECTED");
		}

		if (memcmp(self->m_HS.hs.bd_addr, bd_addr, sizeof(bd_addr)) == 0 && self->m_HS.hs.on) {
			strcpy(hs_connection, "CONNECTED");
		} else {
			strcpy(hs_connection, "DISCONNECTED");
		}

		s_bd_addr = self->bdAddrToString(bd_addr, DEVICE_ADDRESS_SIZE, ':');

		sprintf(buffer+strlen(buffer), "#<%s,%s,%s,%s>", name, s_bd_addr.c_str(), avk_connection, hs_connection);
	}

    sprintf(buffer+strlen(buffer), "\n");
    NXLOGD(buffer);
    self->Broadcast(buffer);

	if (self->m_pRequestExpirePopupMessage) {
		self->m_pRequestExpirePopupMessage();
	}
}

void NxBTService::updateUnpairedDevices_stub(void *pObj)
{
	NxBTService* self = (NxBTService*)pObj;
	char buffer[BUFFER_SIZE];
	int count = m_pModel->getPairedDevCount();
	char name[249] = {0,};
	unsigned char bd_addr[6] = {0,};
	char avk_connection[100] = {0,};
	char hs_connection[100] = {0,};
	std::string s_bd_addr;

	sprintf(buffer, "$OK#%s#%s", "MGT", "PAIRED DEVICE INFO ALL LIST");

	for (int i = 0; i < count; i++) {
		if (0 > m_pModel->getPairedDevInfoByIndex(i, name, bd_addr)) {
			continue;
		}

		if (memcmp(self->m_AVK.connection.bd_addr, bd_addr, sizeof(bd_addr)) == 0 && self->m_AVK.connection.on) {
			strcpy(avk_connection, "CONNECTED");
		} else {
			strcpy(avk_connection, "DISCONNECTED");
		}

		if (memcmp(self->m_HS.hs.bd_addr, bd_addr, sizeof(bd_addr)) == 0 && self->m_HS.hs.on) {
			strcpy(hs_connection, "CONNECTED");
		} else {
			strcpy(hs_connection, "DISCONNECTED");
		}

		s_bd_addr = self->bdAddrToString(bd_addr, DEVICE_ADDRESS_SIZE, ':');

		sprintf(buffer+strlen(buffer), "#<%s,%s,%s,%s>", name, s_bd_addr.c_str(), avk_connection, hs_connection);
	}

	sprintf(buffer+strlen(buffer), "\n");
	self->Broadcast(buffer);
}

void NxBTService::sendPairingRequest_stub(void *pObj_, bool auto_mode_, char *name_, unsigned char *bd_addr_, int32_t pairing_code_)
{
	// example 1) AUTO ON   > $OK#MGT#PAIRING REQUEST#AUTO ON#iPhone6-xxx#18:6D:99:20:09:CB#915112\n
	// example 2) AUTO OFF  > $OK#MGT#PAIRING REQUEST#AUTO OFF#iPhone6-xxx#18:6D:99:20:09:CB#915112\n
	NxBTService* self = (NxBTService*)pObj_;
	char name[DEVICE_NAME_SIZE] = {0,};
	unsigned char bd_addr[DEVICE_ADDRESS_SIZE] = {0,};

	strncpy(name, name_, strlen(name_));
	memcpy(bd_addr, bd_addr_, sizeof(bd_addr));

	std::string s_bd_addr = self->bdAddrToString(bd_addr, DEVICE_ADDRESS_SIZE, ':');

	if (self->m_pRequestPopupMessage) {
		bool bOk = false;
		PopupMessage sData;
		sData.pMsgTitle = new char[1024];
		sData.pMsgBody = new char[1024];

		sprintf(sData.pMsgTitle, "<b>Bluetooth pairing request<b/>");
		sprintf(sData.pMsgBody, "<p align=\"left\">Please check your authorization number to connect with the '%s' device.<br>Device address : %s<br><p align=\"center\"><font size=\"12\" color=\"blue\">%d</font></p>", name, s_bd_addr.c_str(), pairing_code_);

		sData.eVisibility = auto_mode_ ? ButtonVisibility_Ok : ButtonVisibility_Default;
		sData.uiTimeout = 0;

		self->m_pRequestPopupMessage(&sData, &bOk);

		delete[] sData.pMsgTitle;
		delete[] sData.pMsgBody;
	}
}

void NxBTService::callbackLinkDownEventManager(void* pObj, unsigned char* bd_addr, int32_t reason_code)
{
	NXLOGD(__FUNCTION__);
	(void)bd_addr;

	NxBTService* self = (NxBTService*)pObj;
	switch (reason_code) {
		case 0x08:
			if (m_pModel->isAutoConnection()) {
				if (pthread_attr_init(&self->h_AutoConnectAttribution) == 0) {
					if (pthread_attr_setdetachstate(&self->h_AutoConnectAttribution, PTHREAD_CREATE_DETACHED) != 0) {
						NXLOGE("pthread_attr_setdetachstate() [FAILED]");
					}

					pthread_create(&self->h_AutoConnectThread, &self->h_AutoConnectAttribution, NxBTService::autoConnectThread, self);
					pthread_attr_destroy(&self->h_AutoConnectAttribution);
				} else {
					NXLOGE("pthread_attr_init() [FAILED]");
				}
			}
			break;
		case 0x13:
		case 0x16:
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

	self->Broadcast(buffer);
}

void NxBTService::sendAVKConnectionStatus_stub(void *pObj, bool is_connected, char *name, unsigned char *bd_addr)
{
	NxBTService* self = (NxBTService*)pObj;
	char buffer[BUFFER_SIZE] = {0,};
	string s_bd_addr;

	memcpy(self->m_AVK.connection.bd_addr, bd_addr, sizeof(self->m_AVK.connection.bd_addr));
	strncpy(self->m_AVK.connection.name, name, strlen(name));

	// Update connection status and connected index
	self->m_AVK.connection.on = is_connected;
	self->m_AVK.connection.index = (is_connected ? m_pModel->getPairedDevIndexByAddr(bd_addr) : -1);

	// Update connected device name and address
	if (is_connected) {
		m_pModel->getPairedDevInfoByIndex(self->m_AVK.connection.index, self->m_AVK.connection.name, self->m_AVK.connection.bd_addr);
	} else {
		memset(self->m_AVK.connection.name, 0, DEVICE_NAME_SIZE);
		memset(self->m_AVK.connection.bd_addr, 0, DEVICE_ADDRESS_SIZE);
	}

	// Reset play information
	memset(&self->m_AVK.info, 0, sizeof(self->m_AVK.info));

	s_bd_addr = self->bdAddrToString(self->m_AVK.connection.bd_addr, DEVICE_ADDRESS_SIZE, ':');

	// example1) CONNECTED    = "$OK#AVK#CONNECTION STATUS#CONNECTED#iphone#18:6D99:20:09:CB\n"
	// example2) DISCONNECTED = "$OK#AVK#CONNECTION STATUS#DISCONNECTED#iphone#18:6D99:20:09:CB\n"
	sprintf(buffer, "$OK#%s#%s#%s#%s#%s\n", "AVK", "CONNECTION STATUS", is_connected ? "CONNECTED" : "DISCONNECTED", self->m_AVK.connection.name, s_bd_addr.c_str());

	self->Broadcast(buffer);

	// update bt connection
	self->m_pDAudioStatus->SetBTConnection((int32_t)(self->m_AVK.connection.on || self->m_HS.hs.on));
}

void NxBTService::sendAVKRCConnectionStatus_stub(void *pObj, bool is_connected)
{
	NxBTService* self = (NxBTService*)pObj;
	char buffer[BUFFER_SIZE] = {0,};
	unsigned char bd_addr[DEVICE_ADDRESS_SIZE] = {0,};

	sprintf(buffer, "$OK#%s#%s#%s\n", "AVK", "RC CONNECTION STATUS", is_connected ? "CONNECTED" : "DISCONNECTED");

	// Update connection status and connected index
	self->m_AVK.connection.on = is_connected;

	if (is_connected && RET_OK == m_pModel->getConnectionDevAddrAVK(AVK_CONNECTED_INDEX, bd_addr)) {
		self->m_AVK.connection.index = (is_connected ? m_pModel->getPairedDevIndexByAddr(bd_addr) : -1);
	} else {
		self->m_AVK.connection.index = -1;
	}

	// Update connected device name and address
	if (is_connected) {
		m_pModel->getPairedDevInfoByIndex(self->m_AVK.connection.index, self->m_AVK.connection.name, self->m_AVK.connection.bd_addr);
	} else {
		memset(self->m_AVK.connection.name, 0, DEVICE_NAME_SIZE);
		memset(self->m_AVK.connection.bd_addr, 0, DEVICE_ADDRESS_SIZE);
	}

	self->Broadcast(buffer);

	// Update bt connection
	self->m_pDAudioStatus->SetBTConnection((int32_t)(self->m_AVK.connection.on || self->m_HS.hs.on));
}

void NxBTService::updatePlayStatusAVK_stub(void *pObj, int32_t play_status)
{
	NxBTService* self = (NxBTService*)pObj;
	char buffer[BUFFER_SIZE] = {0,};

	PlayStatus temp = self->m_AVK.status;
	self->m_AVK.status = (PlayStatus)play_status;

	// Send play status to clients.
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

	self->Broadcast(buffer);

	if (temp != self->m_AVK.status && self->m_AVK.status == PlayStatus_Playing) {
		// Try to switch audio focus
		if (!g_has_audio_focus) {
			if (self->m_pRequestAudioFocus) {
				bool bOk = false;
				self->m_pRequestAudioFocus(FocusPriority_Normal, &bOk);
				g_has_audio_focus = bOk;
			}
		}

		self->openAudioAVK();
	}

	NXLOGI("[%s] <2>", __FUNCTION__);
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
	self->m_AVK.info.position = 0;

	// example) "$OK#AVK#UPDATE MEDIA ELEMENT#[TITLE]#[ARTIST]#[ALBUM]#[GENRE]#[DURATION]\n"
	sprintf(buffer, "$OK#%s#%s#%s#%s#%s#%s#%d\n", "AVK", "UPDATE MEDIA ELEMENT", self->m_AVK.info.title, self->m_AVK.info.artist, self->m_AVK.info.album, self->m_AVK.info.genre, self->m_AVK.info.duration);

	self->Broadcast(buffer);
}

void NxBTService::updatePlayPositionAVK_stub(void *pObj, int32_t play_pos_msec)
{
    NXLOGD("NxBTService::updatePlayPositionAVK_stub");
    NxBTService* self = (NxBTService*)pObj;
    char buffer[BUFFER_SIZE] = {0,};

    // example) $OK#AVK#UPDATE PLAY POSITION#12345\n"
    self->m_AVK.info.position = play_pos_msec;

    sprintf(buffer, "$OK#%s#%s#%d\n", "AVK", "UPDATE PLAY POSITION", self->m_AVK.info.position);

    self->Broadcast(buffer);
}

void NxBTService::sendAVKStreamingStarted_stub(void* pObj, bool is_opened)
{
	NxBTService* self = (NxBTService*)pObj;
	char buffer[BUFFER_SIZE] = {0,};

	// Send streaming start status to clients.
	if (is_opened) {
		sprintf(buffer, "$OK#%s#%s#%s\n", "AVK", "STREAMING STARTED", "ALSA DEVICE OPENED");
	} else {
		sprintf(buffer, "$OK#%s#%s#%s\n", "AVK", "STREAMING STARTED", "ALSA DEVICE NOT OPENED");
	}

	self->Broadcast(buffer);

	// Switching audio focus
	if (!g_has_audio_focus) {
		if (self->m_pRequestAudioFocus) {
			bool bOk = false;
			self->m_pRequestAudioFocus(FocusPriority_Normal, &bOk);
			g_has_audio_focus = bOk;
		}
	}

	// Try to open audio device
	self->openAudioAVK();
}

void NxBTService::sendAVKStreamingStopped_stub(void *pObj)
{
	NXLOGD(__FUNCTION__);
	NxBTService* self = (NxBTService*)pObj;
	char buffer[BUFFER_SIZE] = {0,};

	// Send streaming stop status to clients.
	sprintf(buffer, "$OK#%s#%s\n", "AVK", "STREAMING STOPPED");

	self->Broadcast(buffer);

	// Try to close audio device
//	self->closeAudioAVK();
}

// Phone : HS
void NxBTService::sendHSOpenFailed_stub(void *pObj)
{
    NxBTService* self = (NxBTService*)pObj;
    char buffer[BUFFER_SIZE] = {0,};

    // example) "$OK#HS#OPEN FAILED\n"
    sprintf(buffer, "$OK#HS#OPEN FAILED\n");

    self->Broadcast(buffer);
}

void NxBTService::sendHSConnectionStatus_stub(void *pObj, bool is_connected, char *name, unsigned char *bd_addr)
{
	NxBTService* self = (NxBTService*)pObj;
	char buffer[BUFFER_SIZE] = {0,};
	string s_bd_addr;

	// Update device address
	if (bd_addr) {
		memcpy(self->m_HS.hs.bd_addr, bd_addr, sizeof(self->m_HS.hs.bd_addr));
	} else {
		memset(self->m_HS.hs.bd_addr, 0, DEVICE_ADDRESS_SIZE);
	}

	// Update device name
	if (name) {
		strncpy(self->m_HS.hs.name, name, strlen(name));
	} else {
		self->m_HS.hs.name[0] = '\0';
	}

	// Update connection status and connected index
	self->m_HS.hs.on = is_connected;
	self->m_HS.hs.index = (is_connected ? m_pModel->getPairedDevIndexByAddr(bd_addr) : -1);

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

	self->Broadcast(buffer);

	// Update bt connection
	self->m_pDAudioStatus->SetBTConnection((int32_t)(self->m_AVK.connection.on || self->m_HS.hs.on));
}

void NxBTService::sendHSCallStatus_stub(void *pObj, int32_t call_status)
{
	NxBTService* self = (NxBTService*)pObj;
	char buffer[BUFFER_SIZE] = {0,};
	char status[100] = {0,};

	// Send 'CALL STATUS' command to client
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

	self->Broadcast(buffer);

	// Switching audio focus
	switch (call_status) {
	case HANG_UP_CALL:
	case DISCONNECTED_CALL:
	{
		// Switching calling mode
		g_calling_mode_on = false;

		if (!m_pModel->isOpenedAudioHS() && g_has_audio_focus_transient) {
			// release audio focus
			if (self->m_pRequestAudoFocusLoss) {
				self->m_pRequestAudoFocusLoss();
				g_has_audio_focus_transient = false;
			}
		}

		if (m_pRequestExpireNotification)
		{
			if (self->m_NotificationType != NotificationMessageType_Unknown)
			{
				self->m_NotificationType = NotificationMessageType_Unknown;
				m_pRequestExpireNotification();
			}
		}
		break;
	}

	case INCOMMING_CALL:
	case READY_OUTGOING_CALL:
	{
		// Switching calling mode
		g_calling_mode_on = true;

		// Switching audio focus (get)
		if (!g_has_audio_focus) {
			bool bOk = false;
			if (self->m_pRequestAudioFocusTransient) {
				self->m_pRequestAudioFocusTransient(FocusPriority_High, &bOk);
			}
			g_has_audio_focus_transient = bOk;
		}
		break;
	}

	default:
		break;
	}
}

void NxBTService::sendHSBatteryStatus_stub(void *pObj, int32_t batt_status)
{
	NxBTService* self = (NxBTService*)pObj;
	char buffer[BUFFER_SIZE] = {0,};

	sprintf(buffer, "$OK#%s#%s#%d\n", "HS", "BATTERY STATUS", batt_status);

	self->Broadcast(buffer);
}

void NxBTService::sendHSCallOperName_stub(void *pObj, char *name)
{
	NxBTService* self = (NxBTService*)pObj;
	char buffer[BUFFER_SIZE] = {0,};

	sprintf(buffer, "$OK#%s#%s#%s\n", "HS", "CALL OPER NAME", name);

	self->Broadcast(buffer);
}

void NxBTService::sendHSAudioMuteStatus_stub(void *pObj, bool is_muted, bool is_opened)
{
	NxBTService* self = (NxBTService*)pObj;
	char buffer[BUFFER_SIZE] = {0,};
	char audio[BUFFER_SIZE] = {0,};
	char microphone[BUFFER_SIZE] = {0,};

	// Send Audio open/close and Microphone mute status to clients
	strcpy(audio, is_opened ? "AUDIO OPENED" : "AUDIO CLOSED");
	strcpy(microphone, is_muted ? "MICROPHONE MUTE ON" : "MICROPHONE MUTE OFF");

	sprintf(buffer, "$OK#HS#AUDIO MUTE STATUS#%s#%s\n", audio, microphone);
	self->Broadcast(buffer);

	// If all of the following conditions are met, the audio focus is returned.
	// condition 1 : audio focus transient flag 'ON'
	// condition 2 : calling mode flag 'OFF'
	// condition 3 : audio open / close status 'CLOSE'
	if (g_has_audio_focus_transient && !g_calling_mode_on && !is_opened) {
		// Release audio focus
		if (self->m_pRequestAudoFocusLoss) {
			self->m_pRequestAudoFocusLoss();
			g_has_audio_focus_transient = false;
		}
	}
}

void NxBTService::sendHSIncommingCallNumber_stub(void *pObj, char *number)
{
	NxBTService* self = (NxBTService*)pObj;
	char buffer[BUFFER_SIZE] = {0,};

#ifdef CONFIG_HSP_PROCESS_MANAGEMENT
	if (self->m_pRequestPlugInIsRunning)
	{
		bool bOk = false;
		self->m_pRequestPlugInIsRunning("NxBTPhone", &bOk);
		if (!bOk)
		{
			// notification
			if (m_pRequestNotification)
			{
				PopupMessage sData;

				sData.pMsgBody = new char[1024];
				sprintf(sData.pMsgBody, "[BT Incoming] %s", number);
				sData.eVisibility = ButtonVisibility_Default;
				sData.uiTimeout = 0;

				self->m_NotificationType = NotificationMessageType_IncomingCall;

				m_pRequestNotification(&sData);

				delete[] sData.pMsgBody;
			}
		}
	}
#endif
	/*
	 * NXBT HS Event : Send incomming call number to UI =>  "0316987429",129,,,"      "
	 * [sendHSIncommingCallNumber_stub] number =  "0316987429",129,,,"      "
	 */
	sprintf(buffer, "$OK#%s#%s#%s\n", "HS", "INCOMMING CALL NUMBER", number);

	self->Broadcast(buffer);
}

// Phone : PBC (phone book)
void NxBTService::sendPBCOpenFailed_stub(void *pObj)
{
	NxBTService* self = (NxBTService*)pObj;
	char buffer[BUFFER_SIZE] = {0,};

	// example) "$OK#PBC#OPEN FAILED\n"
	sprintf(buffer, "$OK#PBC#OPEN FAILED\n");

	self->Broadcast(buffer);
}

void NxBTService::sendPBCConnectionStatus_stub(void *pObj, bool is_connected)
{
	NxBTService* self = (NxBTService*)pObj;
	char buffer[BUFFER_SIZE] = {0,};

	// example1) CONNECTED    = "$OK#PBC#CONNECTION STATUS#CONNECTED\n"
	// example2) DISCONNECTED = "$OK#PBC#CONNECTION STATUS#DISCONNECTED\n"
	self->m_HS.pbc.connection.on = is_connected;

	sprintf(buffer, "$OK#%s#%s#%s\n", "PBC", "CONNECTION STATUS", is_connected ? "CONNECTED" : "DISCONNECTED");

	self->Broadcast(buffer);
}

void NxBTService::sendNotifyGetPhonebook_stub(void *pObj, int32_t type)
{
	NxBTService* self = (NxBTService*)pObj;
	char buffer[BUFFER_SIZE] = {0,};
	char command[100] = {0,};
	bool download_ok = true;

	switch (self->m_HS.pbc.download) {
		case DownloadType_PhoneBook:
			strcpy(command, "DOWNLOAD PHONEBOOK");
			break;
		case DownloadType_CallHistory:
			strcpy(command, "DOWNLOAD CALL LOG");
			break;
		default:
			NXLOGE("Not supported download type!\n");
			break;
    }

	if ((self->m_HS.pbc.download == DownloadType_PhoneBook) || (self->m_HS.pbc.download == DownloadType_CallHistory)) {
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
			default:
				NXLOGE("Invalid download process!\n");
				download_ok = false;
				break;
		}

		if (download_ok) {
			self->Broadcast(buffer);
		}
	}
}

// Phone : MCE (message)
void NxBTService::sendMCEOpenFailed_stub(void *pObj)
{
	NxBTService* self = (NxBTService*)pObj;
	char buffer[BUFFER_SIZE] = {0,};

	// example) "$OK#MCE#OPEN FAILED\n"
	sprintf(buffer, "$OK#MCE#OPEN FAILED\n");

	self->Broadcast(buffer);
}

void NxBTService::sendMCEConnectionStatus_stub(void *pObj, bool is_connected)
{
	NxBTService* self = (NxBTService*)pObj;
	char buffer[BUFFER_SIZE] = {0,};
	unsigned char bd_addr[DEVICE_ADDRESS_SIZE] = {0,};

	self->m_HS.mce.on = is_connected;
	if (0 > self->m_pModel->getConnectionDevAddrHS(bd_addr)) {
		std::string a = self->bdAddrToString(bd_addr, DEVICE_ADDRESS_SIZE, ':');
	}

	// example1) CONNECTED    = "$OK#MCE#CONNECTION STATUS#CONNECTED\n"
	// example2) DISCONNECTED = "$OK#MCE#CONNECTION STATUS#DISCONNECTED\n"
	sprintf(buffer, "$OK#%s#%s#%s\n", "MCE", "CONNECTION STATUS", is_connected ? "CONNECTED" : "DISCONNECTED");

	self->Broadcast(buffer);
}

void NxBTService::sendNotifyGetMessageMCE_stub(void *pObj)
{
	NxBTService* self = (NxBTService*)pObj;
	self->downloadSMSMessage("MCE", "DOWNLOAD SMS MESSAGE");
}

#if 0
#include <signal.h>

static void signal_handler(int32_t signal)
{
	void *array[10];
	size_t size, count = 0;
	char **strings;
	char modules[256] = {0,};

	size = backtrace(array, 10);
	strings = backtrace_symbols(array, size);

	for (size_t i = 0; i < size; ++i) {
		NXLOGE("[%s] %s", __FUNCTION__, strings[i]);
	}

	free(strings);
}
#endif

NxBTService::NxBTService()
{
#if 0
	signal(SIGINT, signal_handler);
	signal(SIGABRT, signal_handler);
#endif

	// Init DB for bluetooth status
	m_pDAudioStatus = new CNX_DAudioStatus(DAUDIO_STATUS_DATABASE_PATH);
	m_pDAudioStatus->SetBTConnection(0);

	m_NotificationType = NotificationMessageType_Unknown;

	// Variable reset to 0
	memset(&m_AVK, 0, sizeof(m_AVK));
	memset(&m_HS, 0, sizeof(m_HS));
	m_bInitialized = false;
	m_pModel = NULL;

	pthread_create(&m_hCommandThread, NULL, NxBTService::CommandThreadStub, (void *)this);
}

NxBTService::~NxBTService()
{
	NXLOGI("[%s]", __FUNCTION__);
}

void NxBTService::Initialize()
{
	// Settings for Nx BT module
	NXLOGI("[%s]", __FUNCTION__);
	m_pModel = getInstance();

	if (m_pModel) {
		registerCallbackFunctions();
		pthread_create(&m_hStartThread, NULL, NxBTService::StartThreadStub, (void *)this);
	}
}

void NxBTService::registerCallbackFunctions()
{
	NXLOGI("[%s] %p %p", __FUNCTION__, m_spInstance, this);

	// Manager : MGT
	m_pModel->registerMGTOpenSucceedCbManager(this, sendMGTOpenSucceed_stub);
	m_pModel->registerMGTDisconnectedCbManager(this, sendMGTDisconnected_stub);
	m_pModel->registerPairingFailedCbManager(this, sendPairingFailed_stub);
	m_pModel->registerPairedDevicesCbManager(this, updatePairedDevices_stub);
	m_pModel->registerUnpairedDevicesCbManager(this, updateUnpairedDevices_stub);
	m_pModel->registerPairingRequestCbManager(this, sendPairingRequest_stub);
	m_pModel->registerLinkDownEventCbManager(this, callbackLinkDownEventManager);
	// Audio : AVK
	m_pModel->registerOpenFailedCbAVK(this, sendAVKOpenFailed_stub);
	m_pModel->registerConnectionStatusCbAVK(this, sendAVKConnectionStatus_stub);
	m_pModel->registerConnectionStatusCbAVKRC(this, sendAVKRCConnectionStatus_stub);
	m_pModel->registerPlayStatusCbAVK(this, updatePlayStatusAVK_stub);
	m_pModel->registerMediaElementCbAVK(this, updateMediaElementsAVK_stub);
	m_pModel->registerPlayPositionCbAVK(this, updatePlayPositionAVK_stub);
	m_pModel->registerStreamingStartedCbAVK(this, sendAVKStreamingStarted_stub);
	m_pModel->registerStreamingStoppedCbAVK(this, sendAVKStreamingStopped_stub);
	// Phone : HS
	m_pModel->registerOpenFailedCbHS(this, sendHSOpenFailed_stub);
	m_pModel->registerConnectionStatusCbHS(this, sendHSConnectionStatus_stub);
	m_pModel->registerCallStatusCbHS(this, sendHSCallStatus_stub);
	m_pModel->registerBatteryStatusCbHS(this, sendHSBatteryStatus_stub);
	m_pModel->registerCallOperNameCbHS(this, sendHSCallOperName_stub);
	m_pModel->registerAudioMuteStatusCbHS(this, sendHSAudioMuteStatus_stub);
	m_pModel->registerIncommingCallNumberCbHS(this,sendHSIncommingCallNumber_stub);
	// Phone : PBC (phone book)
	m_pModel->registerOpenFailedCbPBC(this, sendPBCOpenFailed_stub);
	m_pModel->registerConnectionStatusCbPBC(this, sendPBCConnectionStatus_stub);
	m_pModel->registerNotifyGetPhonebookCbPBC(this, sendNotifyGetPhonebook_stub);
	// Phone : MCE (message)
	m_pModel->registerOpenFailedCbMCE(this, sendMCEOpenFailed_stub);
	m_pModel->registerConnectionStatusCbMCE(this, sendMCEConnectionStatus_stub);
	m_pModel->registerNotifyGetMessageCbMCE(this, sendNotifyGetMessageMCE_stub);
}

void* NxBTService::autoConnectThread(void* args)
{
	NxBTService* self = (NxBTService*)args;

	NXLOGD(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>");
	NXLOGD(__FUNCTION__);
	char buffer[1024];
	sprintf(buffer, "is autoconnection ? %s\nAVK is connected ? %s\nHS is connected ? %s\n", m_pModel->isAutoConnection() ? "ON" : "OFF", self->m_AVK.connection.on ? "ON" : "OFF", self->m_HS.hs.on ? "ON" : "OFF");
	NXLOGD(buffer);
	while (m_pModel->isAutoConnection() && !(self->m_AVK.connection.on || self->m_HS.hs.on)) {
		NXLOGD("Try to auto connection!");
		m_pModel->autoConnection(true);
//		pthread_yield();
		usleep(1000000);
	}
	NXLOGD("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<");

	return (void *)0;
}

bool NxBTService::runCommand(const char *command)
{
	std::vector<std::string> tokens = createTokensFromCommand(command);
	NXLOGI("[%s] CMD = %s", __FUNCTION__, command);
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
		} else if (tokens[CommandType_Command].find("GET MEDIA ELEMENTS") == 0) {
			return requestGetElementAttr(tokens[CommandType_Service], tokens[CommandType_Command]);
		}
	} else if (tokens[CommandType_Service] == "HS") {
		if (tokens[CommandType_Command].find("CONNECTED DEVICE INDEX") == 0) {
			return indexOfConnectedDeviceToHS(tokens[CommandType_Service], tokens[CommandType_Command]);
		} else if (tokens[CommandType_Command].find("CONNECT") == 0) {
			return connectToHS(tokens[CommandType_Service], tokens[CommandType_Command]);
		} else if (tokens[CommandType_Command].find("DISCONNECT") == 0) {
			return disconnectFromHS(tokens[CommandType_Service], tokens[CommandType_Command]);
		} else if (tokens[CommandType_Command].find("IS CONNECTED") == 0) {
			return isConnectedToHS(tokens[CommandType_Service], tokens[CommandType_Command]);
		} else if (tokens[CommandType_Command].find("DIAL") == 0) {
			return dialPhoneNumber(tokens[CommandType_Service], tokens[CommandType_Command]);
		} else if (tokens[CommandType_Command].find("REDIAL") == 0) {
			return reDialPhoneNumber(tokens[CommandType_Service], tokens[CommandType_Command]);
		} else if (tokens[CommandType_Command].find("PICK UP CALL") == 0) {
			return pickUpCall(tokens[CommandType_Service], tokens[CommandType_Command]);
		} else if (tokens[CommandType_Command].find("HANG UP CALL") == 0) {
			return hangUpCall(tokens[CommandType_Service], tokens[CommandType_Command]);
		} else if (tokens[CommandType_Command].find("MICROPHONE MUTE") == 0) {
			return muteMicrophoneHS(tokens[CommandType_Service], tokens[CommandType_Command]);
		} else if (tokens[CommandType_Command].find("AUDIO OPEN") == 0) {
			return openAudioHS(tokens[CommandType_Service], tokens[CommandType_Command]);
		} else if (tokens[CommandType_Command].find("AUDIO CLOSE") == 0) {
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
	Broadcast(MakeReplyCommand(false, tokens).c_str());

	return false;
}

std::vector<std::string> NxBTService::createTokensFromCommand(const char* command)
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

std::string NxBTService::bdAddrToString(unsigned char* bd_addr, int len, char seperator)
{
	char buffer[BUFFER_SIZE] = {0,};

	if (len < DEVICE_ADDRESS_SIZE) {
		return std::string();
	}

	sprintf(buffer, "%02x%c%02x%c%02x%c%02x%c%02x%c%02x", bd_addr[0] & 0xFF, seperator, bd_addr[1] & 0xFF, seperator, bd_addr[2] & 0xFF, seperator, bd_addr[3] & 0xFF, seperator, bd_addr[4] & 0xFF, seperator, bd_addr[5] & 0xFF);

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
	Broadcast(MakeReplyCommand(result, reply).c_str());

	return result;
}

bool NxBTService::setEnableAutoConnection(std::string service, std::string command)
{
	// example 1) result = "$OK#ENABLE AUTO CONNECTION ON"
	// example 2) result = "$OK#ENABLE AUTO CONNECTION OFF"

	std::vector<std::string> reply;
	std::string argument = findArgument(&command);
	bool result = false;

	if (argument == "ON") {
		result = (m_pModel->enableAutoConnection(true) == RET_OK);
	} else if (argument == "OFF") {
		result = (m_pModel->enableAutoConnection(false) == RET_OK);
	}

	reply.push_back(service);
	reply.push_back(command);

	Broadcast(MakeReplyCommand(result, reply).c_str());

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

	Broadcast(MakeReplyCommand(result, reply).c_str());

	return result;
}

bool NxBTService::setEnableAutoPairing(std::string service, std::string command)
{
	// example 1) result = "$OK#ENABLE AUTO PAIRING ON"
	// example 2) result = "$OK#ENABLE AUTO PAIRING OFF"
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

	Broadcast(MakeReplyCommand(result, reply).c_str());

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

	Broadcast(MakeReplyCommand(result, reply).c_str());

	return result;
}

bool NxBTService::acceptPairing(std::string service, std::string command)
{
	// example) result = "$OK#ACCEPT PAIRING\n"

	std::vector<std::string> reply;
	bool result = (m_pModel->acceptPairing() == RET_OK);

	reply.push_back(service);
	reply.push_back(command);

	Broadcast(MakeReplyCommand(result, reply).c_str());

	return result;
}

bool NxBTService::rejectPairing(std::string service, std::string command)
{
	// example) result = "$OK#REJECT PAIRING\n"

	std::vector<std::string> reply;
	bool result = (m_pModel->rejectPairing() == RET_OK);

	reply.push_back(service);
	reply.push_back(command);

	Broadcast(MakeReplyCommand(result, reply).c_str());

	return result;
}

bool NxBTService::unpairAll(std::string service, std::string command)
{
	// example) result = "$OK#MGT#UNPAIR DEVICE ALL\n"

	std::vector<std::string> reply;
	bool result = true;

	for (int32_t i = m_pModel->getPairedDevCount()-1; i >= 0; --i) {
		if (0 == m_pModel->unpairDevice(i)) {
			result = false;
		}
	}

	reply.push_back(service);
	reply.push_back(command);

	Broadcast(MakeReplyCommand(result, reply).c_str());

	return result;
}

bool NxBTService::unpair(std::string service, std::string command)
{
	// example) result = "$OK#UNPAIR DEVICE 3\n"

	std::vector<std::string> reply;
	std::string argument = findArgument(&command);
	bool result = isDigit(argument) && (m_pModel->unpairDevice(atoi(argument.c_str())) == RET_OK);

	reply.push_back(service);
	reply.push_back(command);

	Broadcast(MakeReplyCommand(result, reply).c_str());

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

	Broadcast(MakeReplyCommand(result, reply).c_str());

	return result;
}

bool NxBTService::localDeviceAddress(std::string service, std::string command)
{
	std::vector<std::string> reply;
	bool result = true;

	reply.push_back(service);
	reply.push_back(command);
	reply.push_back(bdAddrToString(m_pModel->getLocalAddress(), DEVICE_ADDRESS_SIZE, ':'));

	Broadcast(MakeReplyCommand(result, reply).c_str());

	return result;
}

bool NxBTService::renameLocalDevice(std::string service, std::string command)
{
	std::vector<std::string> reply;
	std::string argument = findArgument(&command);

	bool result = (!argument.empty() && m_pModel->renameLocalDevice(argument.c_str()) == 0);

	reply.push_back(service);
	reply.push_back(command);

	Broadcast(MakeReplyCommand(result, reply).c_str());

	return result;
}

bool NxBTService::countOfPairedDevice(std::string service, std::string command)
{
	std::vector<std::string> reply;
	bool result = true;

	reply.push_back(service);
	reply.push_back(command);
	reply.push_back(std::to_string(m_pModel->getPairedDevCount()));

	Broadcast(MakeReplyCommand(result, reply).c_str());

	return result;
}

bool NxBTService::infoListOfPairedDeviceAll(std::string service, std::string command)
{
	std::vector<std::string> reply;
	std::string temp;
	int32_t count = m_pModel->getPairedDevCount();
	char name[DEVICE_NAME_SIZE] = {0,};
	unsigned char bd_addr[DEVICE_ADDRESS_SIZE] = {0,};
	bool avk_connected = false;
	bool hs_connected = false;
	bool result = true;

	reply.push_back(service);
	reply.push_back(command);

	for (int32_t i = 0; i < count; i++) {
		if (0 > m_pModel->getPairedDevInfoByIndex(i, name, bd_addr)) {
			continue;
		}

		temp = bdAddrToString(bd_addr, DEVICE_ADDRESS_SIZE, ':');

		NXLOGI("[%s] %s", __FUNCTION__, temp.c_str());

		avk_connected = (temp == bdAddrToString(m_AVK.connection.bd_addr, DEVICE_ADDRESS_SIZE, ':') && m_AVK.connection.on);
		hs_connected = (temp == bdAddrToString(m_HS.hs.bd_addr, DEVICE_ADDRESS_SIZE, ':') && m_HS.hs.on);

		temp = "<" + (std::string)name + "," + bdAddrToString(bd_addr, DEVICE_ADDRESS_SIZE, ':') + "," + (avk_connected ? "CONNECTED" : "DISCONNECTED") + "," + (hs_connected ? "CONNECTED" : "DISCONNECTED") + ">";
		reply.push_back(temp);
	}

	Broadcast(MakeReplyCommand(result, reply).c_str());

	return result;
}

bool NxBTService::infoOfPairedDeviceByIndex(std::string service, std::string command)
{
	std::vector<std::string> reply;
	std::string argument = findArgument(&command);
	std::string temp;
	char name[DEVICE_NAME_SIZE] = {0,};
	unsigned char bd_addr[DEVICE_ADDRESS_SIZE] = {0,};
	bool result = (isDigit(argument) && m_pModel->getPairedDevInfoByIndex(atoi(argument.c_str()), name, bd_addr) == RET_OK);

	if (result) {
		temp = "<" + (std::string)name + "," + bdAddrToString(bd_addr, DEVICE_ADDRESS_SIZE, ':') + ">";
	}

	reply.push_back(service);
	reply.push_back(command);
	reply.push_back(temp);

	Broadcast(MakeReplyCommand(result, reply).c_str());

	return result;
}

bool NxBTService::nameOfPairedDeviceByIndex(std::string service, std::string command)
{
	std::vector<std::string> reply;
	std::string argument = findArgument(&command);
	char name[DEVICE_NAME_SIZE] = {0,};
	bool result = (isDigit(argument) && m_pModel->getPairedDevNameByIndex( atoi(argument.c_str()), name) == RET_OK);

	reply.push_back(service);
	reply.push_back(command);

	if (result) {
		reply.push_back((std::string)name);
	}

	Broadcast(MakeReplyCommand(result, reply).c_str());

	return result;
}

bool NxBTService::addressOfPairedDeviceByIndex(std::string service, std::string command)
{
	std::vector<std::string> reply;
	std::string argument = findArgument(&command);
	unsigned char bd_addr[DEVICE_ADDRESS_SIZE] = {0,};
	bool result = (isDigit(argument) && m_pModel->getPairedDevAddrByIndex(atoi(argument.c_str()), bd_addr) == RET_OK);

	reply.push_back(service);
	reply.push_back(command);

	if (result) {
		reply.push_back(bdAddrToString(bd_addr, DEVICE_ADDRESS_SIZE, ':'));
	}

	Broadcast(MakeReplyCommand(result, reply).c_str());

	return result;
}

bool NxBTService::connectToAVK(std::string service, std::string command)
{
	// example) result = "$OK#AVK#CONNECT 3\n"
	std::vector<std::string> reply;
	std::string argument = findArgument(&command);
	int32_t index = atoi(argument.c_str());
	bool result = (m_pModel->isConnectedAVK() || (isDigit(argument) && m_pModel->connectToAVK(index) == RET_OK));

	if (result) {
		m_AVK.connection.on = true;
		m_AVK.connection.index = index;
		m_pModel->getPairedDevInfoByIndex(index, m_AVK.connection.name, m_AVK.connection.bd_addr);
	}

	reply.push_back(service);
	reply.push_back(command);

	Broadcast(MakeReplyCommand(result, reply).c_str());

	return result;
}

bool NxBTService::disconnectToAVK(std::string service, std::string command)
{
	std::vector<std::string> reply;
	unsigned char bd_addr[6] = {0,};
	bool already_disconnected = !m_pModel->isConnectedAVK();
	bool result = true;

	if (!already_disconnected) {
		m_pModel->getConnectionDevAddrAVK(AVK_CONNECTED_INDEX, bd_addr);
		result = (m_pModel->disconnectFromAVK(bd_addr) == RET_OK);
	}

	reply.push_back(service);
	reply.push_back(command);

	if (result) {
		struct connect_state_t t;
		t.on = false;
		t.index = -1;
		memset(t.name, 0, DEVICE_NAME_SIZE);
		memset(t.bd_addr, 0, DEVICE_ADDRESS_SIZE);

		updateAVKConnectionState(t);

		if (!already_disconnected) {
			reply.push_back(bdAddrToString(bd_addr, DEVICE_ADDRESS_SIZE, ':'));
		}
	}

	Broadcast(MakeReplyCommand(result, reply).c_str());

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

	if (strncmp(m_AVK.connection.name, target.name, strlen(m_AVK.connection.name)) != 0) {
		strncpy(m_AVK.connection.name, target.name, strlen(target.name));
	}

	if (memcmp(m_AVK.connection.bd_addr, target.bd_addr, sizeof(target.bd_addr)) == 0) {
		memcpy(m_AVK.connection.bd_addr, target.bd_addr, sizeof(m_AVK.connection.bd_addr));
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

	Broadcast(MakeReplyCommand(result, reply).c_str());

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

	Broadcast(MakeReplyCommand(result, reply).c_str());

	return result;
}

bool NxBTService::addressOfConnectedDeviceToAVK(std::string service, std::string command)
{
	std::vector<std::string> reply;
	unsigned char bd_addr[DEVICE_ADDRESS_SIZE] = {0,};
	bool result = (m_pModel->getConnectionDevAddrAVK(AVK_CONNECTED_INDEX, bd_addr) == RET_OK);

	reply.push_back(service);
	reply.push_back(command);

	if (result) {
		reply.push_back(bdAddrToString(bd_addr, DEVICE_ADDRESS_SIZE, ':'));
	}

	Broadcast(MakeReplyCommand(result, reply).c_str());

	return result;
}

bool NxBTService::playStartAVK(std::string service/*= "AVK"*/, std::string command/*= "PLAY START"*/)
{
	std::vector<std::string> reply;
	unsigned char bd_addr[6];
	bool result = true;

	reply.push_back(service);
	reply.push_back(command);

	result = !(m_pModel->getConnectionDevAddrAVK(AVK_CONNECTED_INDEX, bd_addr) < 0 || m_pModel->playStartAVK(bd_addr) < 0);

	if (result) {
		reply.push_back(bdAddrToString(bd_addr, DEVICE_ADDRESS_SIZE, ':'));
	}

	Broadcast(MakeReplyCommand(result, reply).c_str());

	return result;
}

bool NxBTService::playStopAVK(std::string service/*= "AVK"*/, std::string command/*= "PLAY STOP"*/)
{
	std::vector<std::string> reply;
	unsigned char bd_addr[6];
	bool result = true;

	reply.push_back(service);
	reply.push_back(command);

	result = !(m_pModel->getConnectionDevAddrAVK(AVK_CONNECTED_INDEX, bd_addr) < 0 || m_pModel->playStopAVK(bd_addr) < 0);

	if (result) {
		reply.push_back(bdAddrToString(bd_addr, DEVICE_ADDRESS_SIZE, ':'));
	}

	Broadcast(MakeReplyCommand(result, reply).c_str());

	return result;
}

bool NxBTService::playPauseAVK(std::string service/*= "AVK"*/, std::string command/*= "PLAY PAUSE"*/)
{
	std::vector<std::string> reply;
	unsigned char bd_addr[6];
	bool result = true;

	reply.push_back(service);
	reply.push_back(command);

	result = !(m_pModel->getConnectionDevAddrAVK(AVK_CONNECTED_INDEX, bd_addr) < 0 || m_pModel->playPauseAVK(bd_addr) < 0);

	if (result) {
		reply.push_back(bdAddrToString(bd_addr, DEVICE_ADDRESS_SIZE, ':'));
	}

	Broadcast(MakeReplyCommand(result, reply).c_str());

	return result;
}

bool NxBTService::playPrevAVK(std::string service/*= "AVK"*/, std::string command/*= "PLAY PREV"*/)
{
	std::vector<std::string> reply;
	unsigned char bd_addr[6];
	bool result = true;

	reply.push_back(service);
	reply.push_back(command);

	result = !(m_pModel->getConnectionDevAddrAVK(AVK_CONNECTED_INDEX, bd_addr) < 0 || m_pModel->playPrevAVK(bd_addr) < 0);

	if (result) {
		reply.push_back(bdAddrToString(bd_addr, DEVICE_ADDRESS_SIZE, ':'));
	}

	Broadcast(MakeReplyCommand(result, reply).c_str());

	return result;
}

bool NxBTService::playNextAVK(std::string service/*= "AVK"*/, std::string command/*= "PLAY NEXT"*/)
{
	std::vector<std::string> reply;
	unsigned char bd_addr[6];
	bool result = true;

	reply.push_back(service);
	reply.push_back(command);

	result = !(m_pModel->getConnectionDevAddrAVK(AVK_CONNECTED_INDEX, bd_addr) < 0 || m_pModel->playNextAVK(bd_addr) < 0);

	if (result) {
		reply.push_back(bdAddrToString(bd_addr, DEVICE_ADDRESS_SIZE, ':'));
	}

	Broadcast(MakeReplyCommand(result, reply).c_str());

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

	Broadcast(MakeReplyCommand(result, reply).c_str());

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

	Broadcast(MakeReplyCommand(result, reply).c_str());

	return result;
}

bool NxBTService::openAudioAVK(std::string service/*= "AVK"*/, std::string command/*= "OPEN AUDIO"*/)
{
	std::vector<std::string> reply;
	bool result = true;

	reply.push_back(service);
	reply.push_back(command);

	result = (m_pModel->isAudioStatusAVK() || m_pModel->openAudioAVK() == RET_OK);

	Broadcast(MakeReplyCommand(result, reply).c_str());

	return result;
}

bool NxBTService::closeAudioAVK(std::string service/*= "AVK"*/, std::string command/*= "CLOSE AUDIO"*/)
{
	std::vector<std::string> reply;
	bool result = true;

	reply.push_back(service);
	reply.push_back(command);

	if (m_pModel->isAudioStatusAVK()) {
		m_pModel->closeAudioAVK();
	}

	Broadcast(MakeReplyCommand(result, reply).c_str());

	return result;
}

bool NxBTService::requestGetElementAttr(std::string service/*= "AVK"*/, std::string command/*= "GET MEDIA ELEMENTS"*/)
{
    std::vector<std::string> reply;
    unsigned char bd_addr[6];
    bool result = true;

    reply.push_back(service);
    reply.push_back(command);

    result = !(m_pModel->getConnectionDevAddrAVK(AVK_CONNECTED_INDEX, bd_addr) < 0 || m_pModel->requestGetElementAttr(bd_addr) < 0);

    if (result) {
        reply.push_back(bdAddrToString(bd_addr, DEVICE_ADDRESS_SIZE, ':'));
    }

    Broadcast(MakeReplyCommand(result, reply).c_str());

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

	Broadcast(MakeReplyCommand(result, reply).c_str());

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

	Broadcast(MakeReplyCommand(result, reply).c_str());

	return result;
}

bool NxBTService::isConnectedToHS(std::string service, std::string command)
{
	std::vector<std::string> reply;
	bool result = true;

	reply.push_back(service);
	reply.push_back(command);
	reply.push_back(m_pModel->isConnectedHS() ? "CONNECTED" : "DISCONNECTED");

	Broadcast(MakeReplyCommand(result, reply).c_str());

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

	Broadcast(MakeReplyCommand(result, reply).c_str());

	return result;
}

bool NxBTService::dialPhoneNumber(std::string service, std::string command)
{
	std::vector<std::string> reply;
	std::string argument = findArgument(&command);
	bool result = isDigit(argument) && ( m_pModel->dialPhoneNumber(argument.c_str()) == RET_OK );

	reply.push_back(service);
	reply.push_back(command);

	Broadcast(MakeReplyCommand(result, reply).c_str());

	return result;
}

bool NxBTService::reDialPhoneNumber(std::string service, std::string command)
{
	std::vector<std::string> reply;
	bool result = m_pModel->reDialPhoneNumber();

	reply.push_back(service);
	reply.push_back(command);

	Broadcast(MakeReplyCommand(result, reply).c_str());

	return result;
}

bool NxBTService::pickUpCall(std::string service, std::string command)
{
	std::vector<std::string> reply;
	bool result = (m_pModel->pickUpCall() == RET_OK);

	reply.push_back(service);
	reply.push_back(command);

	Broadcast(MakeReplyCommand(result, reply).c_str());

	return result;
}

bool NxBTService::hangUpCall(std::string service, std::string command)
{
	std::vector<std::string> reply;
	bool result = (m_pModel->hangUpCall() == RET_OK);

	reply.push_back(service);
	reply.push_back(command);

	Broadcast(MakeReplyCommand(result, reply).c_str());

	return result;
}

bool NxBTService::muteMicrophoneHS(std::string service, std::string command)
{
	std::vector<std::string> reply;
	std::string argument = findArgument(&command);
	bool result = true;

	if (argument == "ON") {
		m_pModel->muteMicrophoneHS(true);
	} else if (argument == "OFF") {
		m_pModel->muteMicrophoneHS(false);
	} else {
		result = false;
	}

	reply.push_back(service);
	reply.push_back(command);

	Broadcast(MakeReplyCommand(result, reply).c_str());

	return result;
}

bool NxBTService::openAudioHS(std::string service, std::string command)
{
	std::vector<std::string> reply;
	bool result = (m_pModel->isOpenedAudioHS() || (m_pModel->openAudioHS() == RET_OK));

	reply.push_back(service);
	reply.push_back(command);

	Broadcast(MakeReplyCommand(result, reply).c_str());

	return result;
}

bool NxBTService::closeAudioHS(std::string service, std::string command)
{
	std::vector<std::string> reply;
	bool result = (!m_pModel->isOpenedAudioHS() || m_pModel->closeAudioHS() == RET_OK);

	reply.push_back(service);
	reply.push_back(command);

	Broadcast(MakeReplyCommand(result, reply).c_str());

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

	Broadcast(MakeReplyCommand(result, reply).c_str());

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

	Broadcast(MakeReplyCommand(result, reply).c_str());

	return result;
}

bool NxBTService::isConnectedToMCE(std::string service, std::string command)
{
	std::vector<std::string> reply;
	bool result = true;

	reply.push_back(service);
	reply.push_back(command);
	reply.push_back(m_pModel->isConnectedMCE() ? "CONNECTED" : "DISCONNECTED");

	Broadcast(MakeReplyCommand(result, reply).c_str());

	return result;
}

bool NxBTService::downloadSMSMessage(std::string service, std::string command)
{
	std::vector<std::string> reply;
	Bmessage_info_t bmsg;
	bmsg.fullName = NULL;
	bmsg.phoneNumber = NULL;
	bmsg.msgBody = NULL;

	bool result = (m_pModel->getParserBmsg(&bmsg) == RET_OK);

	reply.push_back(service);
	reply.push_back(command);

	if (result) {
		if (bmsg.fullName) {
			reply.push_back(bmsg.fullName);
		} else {
			reply.push_back("");
		}

		if (bmsg.phoneNumber) {
			reply.push_back(bmsg.phoneNumber);
		} else {
			reply.push_back("");
		}

		if (bmsg.msgBody) {
			reply.push_back(bmsg.msgBody);
		} else {
			reply.push_back("");
		}
	}

	Broadcast(MakeReplyCommand(result, reply).c_str());

	if (bmsg.fullName) {
		free(bmsg.fullName);
	}
	if (bmsg.phoneNumber) {
		free(bmsg.phoneNumber);
	}
	if (bmsg.msgBody) {
		free(bmsg.msgBody);
	}

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

	Broadcast(MakeReplyCommand(result, reply).c_str());

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

	Broadcast(MakeReplyCommand(result, reply).c_str());

	return result;
}

bool NxBTService::isConnectedToPBC(std::string service, std::string command)
{
	std::vector<std::string> reply;
	bool result = true;

	reply.push_back(service);
	reply.push_back(command);
	reply.push_back(m_pModel->isConnectedPBC() ? "CONNECTED" : "DISCONNECTED");

	Broadcast(MakeReplyCommand(result, reply).c_str());

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

	Broadcast(MakeReplyCommand(result, reply).c_str());

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

	Broadcast(MakeReplyCommand(result, reply).c_str());

	return result;
}

//-----------------------------------------------------------------------
// Utility functions
bool NxBTService::isDigit(std::string text)
{
	if (text.empty()) {
		return false;
	}

	for (size_t i = 0; i < text.length(); i++) {
		if (text[i] < '0' || '9' < text[i]) {
			return false;
		}
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
	if (pos == std::string::npos) {
		return std::string();
	}
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

bool NxBTService::isInitialized()
{
	return m_bInitialized;
}

void NxBTService::setInitialized(bool state)
{
	if (!m_bInitialized) {
		m_bInitialized = state;
		if (m_bInitialized) {
			// Set ALSA device names
			m_pModel->setALSADevName(NX_ALSA_DEV_NAME_P, NX_ALSA_DEV_NAME_C, NX_ALSA_BT_DEV_NAME_P, NX_ALSA_BT_DEV_NAME_C, true);

			// Auto connection
			m_pModel->autoConnection(m_pModel->isAutoConnection());

			ping("MGT", "PING");
		}
	}
}

void NxBTService::RequestAudioFocus(FocusType eType, FocusPriority ePriority, bool *bOk)
{
	NxBTService *p = NxBTService::GetInstance();

	if (eType == FocusType_Get) {
		FocusPriority eCurrPriority = g_calling_mode_on ? FocusPriority_High : FocusPriority_Normal;

		if (eCurrPriority > ePriority) {
			*bOk = false;
		} else {
			*bOk = true;
		}

		if (*bOk) {
			g_has_audio_focus = false;

			p->closeAudioAVK();

			if (p->m_AVK.status == PlayStatus_Playing) {
				p->playPauseAVK();
			}

#ifdef CONFIG_A2DP_PROCESS_MANAGEMENT
			if (p->m_pRequestPlugInTerminate) {
				p->m_pRequestPlugInTerminate("NxBTAudio");
			}
#endif
		}
	} else {
		*bOk = true;
		p->playStartAVK();
	}
}

void NxBTService::RequestAudioFocusTransient(FocusPriority ePriority, bool *bOk)
{
	NxBTService *p = NxBTService::GetInstance();
	FocusPriority eCurrPriority = g_calling_mode_on ? FocusPriority_High : FocusPriority_Normal;

	if (eCurrPriority > ePriority) {
		*bOk = false;
	} else {
		*bOk = true;
	}

	if (*bOk) {
		g_has_audio_focus = false;

		p->closeAudioAVK();

		if (p->m_AVK.status == PlayStatus_Playing) {
			p->playPauseAVK();
		}

#ifdef CONFIG_A2DP_PROCESS_MANAGEMENT
		if (p->m_pRequestPlugInTerminate) {
			p->m_pRequestPlugInTerminate("NxBTAudio");
		}
#endif
	}
}

void NxBTService::RegisterRequestSendMessage(void (*cbFunc)(const char *pDst, const char *pMsg, int32_t iMsgSize))
{
	if (cbFunc) {
		m_pRequestSendMessage = cbFunc;
	}
}

void NxBTService::RegisterRequestPopupMessage(void (*cbFunc)(PopupMessage *, bool *))
{
	if (cbFunc) {
		m_pRequestPopupMessage = cbFunc;
	}
}

void NxBTService::RegisterRequestExpirePopupMessage(void (*cbFunc)())
{
	if (cbFunc) {
		m_pRequestExpirePopupMessage = cbFunc;
	}
}

void NxBTService::PopupMessageResponse(bool bOk)
{
	switch (m_PopupMessageType) {
		case PopupMessageType_PairingRequest_AutoOff:
		{
			if (bOk) {
				acceptPairing();
			} else {
				rejectPairing();
			}
			break;
		}
		case PopupMessageType_PairingRequest_AutoOn:
			break;
		default:
			break;
	}
}

void NxBTService::RegisterRequestNotification(void (*cbFunc)(PopupMessage *))
{
	if (cbFunc) {
		m_pRequestNotification = cbFunc;
	}
}

void NxBTService::RegisterRequestExpireNotification(void (*cbFunc)())
{
	if (cbFunc) {
		m_pRequestExpireNotification = cbFunc;
	}
}

void NxBTService::NotificationResponse(bool bOk)
{
	switch (m_NotificationType) {
	case NotificationMessageType_IncomingCall:
	{
		if (g_calling_mode_on)
		{
			m_NotificationType = NotificationMessageType_Unknown;

			if (bOk)
				m_spInstance->pickUpCall();
			else
				m_spInstance->hangUpCall();
		}
		break;
	}

	default:
		break;
	}
}

void NxBTService::RegisterRequestAudioFocus(void (*cbFunc)(FocusPriority ePriority, bool *bOk))
{
	if (cbFunc) {
		m_pRequestAudioFocus = cbFunc;
	}
}

void NxBTService::RegisterRequestAudioFocusTransient(void (*cbFunc)(FocusPriority ePriority, bool *bOk))
{
	if (cbFunc) {
		m_pRequestAudioFocusTransient = cbFunc;
	}
}

void NxBTService::RegisterRequestAudioFocusLoss(void (*cbFunc)(void))
{
	if (cbFunc) {
		m_pRequestAudoFocusLoss = cbFunc;
	}
}

void NxBTService::RegisterRequestPlugInRun(void (*cbFunc)(const char *pPlugin, const char *pArgs))
{
	if (cbFunc) {
		m_pRequestPlugInRun = cbFunc;
	}
}

void NxBTService::RegisterRequestPlugInTerminate(void (*cbFunc)(const char *pPlugin))
{
	if (cbFunc) {
		m_pRequestPlugInTerminate = cbFunc;
	}
}

void NxBTService::RegisterRequestPlugInIsRunning(void (*cbFunc)(const char *pPlugin, bool *bOk))
{
	if (cbFunc) {
		m_pRequestPlugInIsRunning = cbFunc;
	}
}

void NxBTService::Broadcast(const char *pMsg)
{
	if (m_pRequestSendMessage) {
		m_pRequestSendMessage("NxBTAudio", pMsg, strlen(pMsg));
		m_pRequestSendMessage("NxBTPhone", pMsg, strlen(pMsg));
		m_pRequestSendMessage("NxBTSettings", pMsg, strlen(pMsg));
	}
}

void* NxBTService::StartThreadStub(void *pObj)
{
    if (pObj) {
        ((NxBTService*)pObj)->StartThreadProc();
		pthread_join(((NxBTService*)pObj)->m_hStartThread, NULL);
    }

    pthread_exit((void *)0);
}

void NxBTService::StartThreadProc()
{
	m_pModel->initDevManager();
	pthread_exit((void *)0);
}

void* NxBTService::CommandThreadStub(void *pObj)
{
	if (pObj) {
		((NxBTService*)pObj)->CommandThreadProc();
		pthread_join(((NxBTService*)pObj)->m_hCommandThread, NULL);
	}

	pthread_exit((void *)0);
}

void NxBTService::CommandThreadProc()
{
	NxBTService *p = m_spInstance;

	while (1) {
		if (-1 != access("/home/root/cmd.audiofocus", F_OK)) {
			NXLOGI("[%s] DETECT COMMAND : AudioFocus", __FUNCTION__);

			if (p->m_pRequestAudioFocus) {
				bool bOk = false;
				p->m_pRequestAudioFocus(FocusPriority_Normal, &bOk);
				g_has_audio_focus = bOk;
			}

			remove("/home/root/cmd.audiofocus");
		}

		usleep(100000);
	}
}
