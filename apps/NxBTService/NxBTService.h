#ifndef NXBTSERVICE_H
#define NXBTSERVICE_H

#include <pthread.h>
#include <unistd.h>

// for std
#include <vector>
#include <string>
using namespace std;

#include <time.h>

// NXBT header
#include <INX_BT.h>

// NX IPC header
#include <NX_Type.h>

// NX DAUDIO UTILS header
#include <CNX_DAudioStatus.h>

#ifndef BUFFER_SIZE
#   define BUFFER_SIZE 1024
#endif /* BUFFER_SIZE */

#define PB_DATA_PATH	"/etc/bluetooth/pb_data.vcf"

#define DEVICE_NAME_SIZE    249
#define DEVICE_ADDRESS_SIZE 6

#define AVK "AVK"
#define MGT "MGT"
#define HS	"HS"
#define PBC "PBC"
#define MCE "MCE"

class NxBTService
{
public:
	enum CommandErrorType {
		CommandParseErrorType_None = 0,
		CommandParseErrorType_UnknownService = -1,
		CommandParseErrorType_UnknownCommand = -2,
		CommandParseErrorType_UnknownCommandArgument = -3,
		CommandParseErrorType_InvalidCommandFormat = -4,
		CommandParseErrorType_FailedFromInterface = -5
	};

	enum CallStatus {
		UNKNOWN_CALL = 0,
		HANG_UP_CALL,
		INCOMMING_CALL,
		READY_OUTGOING_CALL,
		OUTGOING_CALL,
		PICK_UP_CALL,
		DISCONNECTED_CALL
	};

	enum GetPBStatus {
		PARAM_PROGRESS = 2,
		PARAM_PHONEBOOK,
		PARAM_FILE_TRANSFER_STATUS
	};

	enum PlayStatus {
		PlayStatus_Stopped = 0,
		PlayStatus_Playing,
		PlayStatus_Paused
	};

	enum DownloadType {
		DownloadType_None,
		DownloadType_PhoneBook,
		DownloadType_CallHistory
	};

	enum PopupMessageType {
		PopupMessageType_PairingRequest_AutoOff,
		PopupMessageType_PairingRequest_AutoOn,
		PopupMessageType_PairingRequest_Unknown
	};

	enum NotificationMessageType {
		NotificationMessageType_IncomingCall,
		NotificationMessageType_Unknown
	};

	struct PlayInfo {
		char title[102];
		char artist[102];
		char album[102];
		char genre[102];
		int duration; // unit : milisecond
		int position; // unit : milisecond
	};

	struct connect_state_t {
		bool on;
		int index; // connected index
		unsigned char bd_addr[DEVICE_ADDRESS_SIZE];
		char name[DEVICE_NAME_SIZE];
	};

	struct AVKService {
		connect_state_t connection;
		PlayStatus status;
		PlayInfo info;
	};

	struct PBCService {
		connect_state_t connection;
		DownloadType download;
	};

	struct HSService {
		connect_state_t hs;
		connect_state_t mce;
		PBCService pbc;
	};

private:
	NxBTService();
	~NxBTService();

public:
	static NxBTService* GetInstance(void *pObj);

	static NxBTService* GetInstance();

	static void DestroyInstance();

	void Initialize();

	bool isInitialized();

	void setInitialized(bool state);

	static void RegisterRequestSendMessage(void (*cbFunc)(const char *pDst, const char *pMsg, int32_t iMsgSize));

	static void RegisterRequestPopupMessage(void (*cbFunc)(PopupMessage *, bool *));

	static void RegisterRequestExpirePopupMessage(void (*cbFunc)());

	void PopupMessageResponse(bool bOk);

	// Notification
	static void RegisterRequestNotification(void (*cbFunc)(PopupMessage *));

	static void RegisterRequestExpireNotification(void (*cbFunc)());

	void NotificationResponse(bool bOk);

	bool runCommand(const char* command);

	void RequestAudioFocus(FocusType eType, FocusPriority ePriority, bool *bOk);

	static void RegisterRequestAudioFocus(void (*cbFunc)(FocusPriority ePriority, bool *bOk));

	void RequestAudioFocusTransient(FocusPriority ePriority, bool *bOk);

	static void RegisterRequestAudioFocusTransient(void (*cbFunc)(FocusPriority ePriority, bool *bOk));

	static void RegisterRequestAudioFocusLoss(void (*cbFunc)(void));

	static void RegisterRequestPlugInRun(void (*cbFunc)(const char *pPlugin, const char *pArgs));

	static void RegisterRequestPlugInTerminate(void (*cbFunc)(const char *pPlugin));

	static void RegisterRequestPlugInIsRunning(void (*cbFunc)(const char *pPlugin, bool *bOk));

	void registerCallbackFunctions();

	static void* autoConnectThread(void* args);

	void findClientType();

	std::vector<std::string> createTokensFromCommand(const char* command);

	std::string bdAddrToString(unsigned char* bd_addr, int len, char seperator);

	// Callback functions
	static void sendMGTOpenSucceed_stub(void* pObj, int32_t result);

	static void sendMGTDisconnected_stub(void *pObj);

	static void sendPairingFailed_stub(void *pObj, int32_t fail_reason);

	static void updatePairedDevices_stub(void *pObj);

	static void updateUnpairedDevices_stub(void *pObj);

	static void sendPairingRequest_stub(void *pObj, bool auto_mode, char *name, unsigned char *bd_addr, int32_t pairing_code);

	static void callbackLinkDownEventManager(void* pObj, unsigned char* bd_addr, int32_t reason_code);

	static void sendAVKOpenFailed_stub(void *pObj);

	static void sendAVKConnectionStatus_stub(void *pObj, bool is_connected, char *name, unsigned char *bd_addr);

	static void sendAVKRCConnectionStatus_stub(void *pObj, bool is_connected);

	static void updatePlayStatusAVK_stub(void *pObj, int32_t play_status);

	static void updateMediaElementsAVK_stub(void *pObj, char *mediaTitle, char *mediaArtist, char *mediaAlbum, char *mediaGenre, int32_t playTime_msec);

	static void updatePlayPositionAVK_stub(void *pObj, int32_t play_pos_msec);

	static void sendAVKStreamingStarted_stub(void* pObj, bool is_opened);

	static void sendAVKStreamingStopped_stub(void *pObj);

	// HS
	static void sendHSOpenFailed_stub(void *pObj);

	static void sendHSConnectionStatus_stub(void *pObj_, bool is_connected_, char *name_, unsigned char *bd_addr_);

	static void sendHSCallStatus_stub(void *pObj, int32_t call_status);

	static void sendHSBatteryStatus_stub(void *pObj, int32_t batt_status);

	static void sendHSCallOperName_stub(void *pObj, char *name);

	static void sendHSAudioMuteStatus_stub(void *pObj, bool is_muted, bool is_opened);

	static void sendHSIncommingCallNumber_stub(void *pObj, char *number);

	// HS - PBC
	static void sendPBCOpenFailed_stub(void *pObj);

	static void sendPBCConnectionStatus_stub(void *pObj, bool is_connected);

	static void sendNotifyGetPhonebook_stub(void *pObj, int32_t type);

	// HS - MCE
	static void sendMCEOpenFailed_stub(void *pObj);

	static void sendMCEConnectionStatus_stub(void *pObj, bool is_connected);

	static void sendNotifyGetMessageMCE_stub(void *pObj);

	//-----------------------------------------------------------------------
	// MGT functions
	bool ping(std::string service, std::string command);

	bool setEnableAutoConnection(std::string service, std::string command);

	bool isEnabledAutoConnection(std::string service, std::string command);

	bool setEnableAutoPairing(std::string service, std::string command);

	bool isEnabledAutoPairing(std::string service, std::string command);

	bool acceptPairing(std::string service = "MGT", std::string command = "ACCEPT PAIRING");

	bool rejectPairing(std::string service = "MGT", std::string command = "REJECT PAIRING");

	bool unpairAll(std::string service, std::string command);

	bool unpair(std::string service, std::string command);

	bool localDeviceName(std::string service, std::string command);

	bool localDeviceAddress(std::string service, std::string command);

	bool renameLocalDevice(std::string service, std::string command);

	bool countOfPairedDevice(std::string service, std::string command);

	bool infoListOfPairedDeviceAll(std::string service, std::string command);

	bool infoOfPairedDeviceByIndex(std::string service, std::string command);

	bool nameOfPairedDeviceByIndex(std::string service, std::string command);

	bool addressOfPairedDeviceByIndex(std::string service, std::string command);

	//-----------------------------------------------------------------------
	// AVK functions
	bool connectToAVK(std::string service, std::string command);

	bool disconnectToAVK(std::string service, std::string command);

	bool indexOfConnectedDeviceToAVK(std::string service, std::string command);

	bool addressOfConnectedDeviceToAVK(std::string service, std::string command);

	bool isConnectedToAVK(std::string service, std::string command);

	bool playStartAVK(std::string service = "AVK", std::string command = "PLAY START");

	bool playStopAVK(std::string service = "AVK", std::string command = "PLAY STOP");

	bool playPauseAVK(std::string service = "AVK", std::string command = "PLAY PAUSE");

	bool playPrevAVK(std::string service = "AVK", std::string command = "PLAY PREV");

	bool playNextAVK(std::string service = "AVK", std::string command = "PLAY NEXT");

	bool playStatusAVK(std::string service = "AVK", std::string command = "PLAY STATUS");

	bool playInfoAVK(std::string service = "AVK", std::string command = "PLAT INFO");

	bool openAudioAVK(std::string service = "AVK", std::string command = "OPEN AUDIO");

	bool closeAudioAVK(std::string service = "AVK", std::string command = "CLOSE AUDIO");

	bool requestGetElementAttr(std::string service = "AVK", std::string command = "GET MEDIA ELEMENTS");

	//-----------------------------------------------------------------------
	// HS functions
	bool connectToHS(std::string service, std::string command);

	bool disconnectFromHS(std::string service, std::string command);

	bool isConnectedToHS(std::string service, std::string command);

	bool indexOfConnectedDeviceToHS(std::string service, std::string command);

	bool dialPhoneNumber(std::string service, std::string command);

	bool reDialPhoneNumber(std::string service, std::string command);

	bool pickUpCall(std::string service = "HS", std::string command = "PICK UP CALL");

	bool hangUpCall(std::string service = "HS", std::string command = "HANG UP CALL");

	bool muteMicrophoneHS(std::string service, std::string command);

	bool openAudioHS(std::string service, std::string command);

	bool closeAudioHS(std::string service, std::string command);

	//-----------------------------------------------------------------------
	// MCE functions
	bool connectToMCE(std::string service, std::string command);

	bool disconnectFromMCE(std::string service, std::string command);

	bool isConnectedToMCE(std::string service, std::string command);

	bool downloadSMSMessage(std::string service, std::string command);

	//-----------------------------------------------------------------------
	// PBC functions
	bool connectToPBC(std::string service, std::string command);

	bool disconnectFromPBC(std::string service, std::string command);

	bool isConnectedToPBC(std::string service, std::string command);

	bool downloadPhoneBook(std::string service, std::string command);

	bool downloadCallHistory(std::string service, std::string command);

private:
	static INX_BT *m_pModel;

	pthread_t h_pthread;

	pthread_t h_AutoConnectThread;
	pthread_cond_t h_AutoConnectCondition;
	pthread_mutex_t h_AutoConnectMutex;
	pthread_attr_t h_AutoConnectAttribution;
	bool m_bAutoConnect;

	pthread_t h_CheckALSADeviceIsClosedThread;
	pthread_attr_t h_CheckALSADeviceIsClosedThreadAttr;

	void updateAVKConnectionState(struct connect_state_t target);

	pthread_t m_hCommandThread;
	pthread_t m_hStartThread;

	static void* CommandThreadStub(void *pObj);
	static void* StartThreadStub(void *pObj);

	void CommandThreadProc();
	void StartThreadProc();

	AVKService m_AVK;

	HSService m_HS;

	CNX_DAudioStatus* m_pDAudioStatus;

	bool m_bInitialized;

	static NxBTService *m_spInstance;

	// Audio Focus
	static void (*m_pRequestAudioFocus)(FocusPriority ePriority, bool *bOk);

	static void (*m_pRequestAudioFocusTransient)(FocusPriority ePriority, bool *bOk);

	static void (*m_pRequestAudoFocusLoss)();

	static void (*m_pRequestPlugInRun)(const char *pPlugin, const char *pArgs);

	static void (*m_pRequestPlugInTerminate)(const char *pPlugin);

	static void (*m_pRequestPlugInIsRunning)(const char *pPlugin, bool *bOk);

	// Send Message
	static void (*m_pRequestSendMessage)(const char *pDst, const char *pMsg, int32_t iMsgSize);

	// Popup Message
	static void (*m_pRequestPopupMessage)(PopupMessage *, bool *);

	static void (*m_pRequestExpirePopupMessage)();

	// Notification
	static void (*m_pRequestNotification)(PopupMessage *);

	static void (*m_pRequestExpireNotification)();

	void Broadcast(const char *pMsg);

	static bool g_calling_mode_on;
	static bool g_has_audio_focus;
	static bool g_has_audio_focus_transient;

	PopupMessageType m_PopupMessageType;
	NotificationMessageType m_NotificationType;
};

#endif // NXBTSERVICE_H
