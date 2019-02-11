/*****************************************************************************
 **
 **  Name:        main.cpp
 **
 **  Description: Nexell Linux Bluetooth Test Console Application
 **
 **  Copyright (c) 2017, Nexell Corp., All Rights Reserved.
 **  Broadcom BT stack supports. Proprietary and confidential.
 **
 **  Author: Chris Lee.
 **
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>
#include <execinfo.h>
#include <signal.h>

#include <INX_BT.h>
#include <NxBTServiceConsole.h>

#include "NX_IConfig.h"
#define NXBTSERVICE_CONFIG "/nexell/daudio/NxBTService/nxbtservice_config.xml"

#define USE_BACKTRACE
#define SYNC_MODE	true

int app_get_string(const char *querystring, char *str, int len) {
	int c, index;

	if (querystring) {
		printf("%s => ", querystring);
	}

	index = 0;

	do {
		c = getchar();
		if (c == EOF) {
			return -1;
		}
		if ((c != '\n') && (index < (len - 1))) {
			str[index] = (char)c;
			index++;
		}
	} while (c != '\n');

	str[index] = '\0';
	return index;
}

int app_get_choice(const char *querystring) {
	int neg, value, c, base;
	int count = 0;

	base = 10;
	neg = 1;
	printf("%s => ", querystring);
	value = 0;

	do {
		c = getchar();

		if ((count == 0) && (c == '\n')) {
			return -1;
		}
		count ++;

		if ((c >= '0') && (c <= '9')) {
			value = (value * base) + (c - '0');
		} else if ((c >= 'a') && (c <= 'f')) {
			value = (value * base) + (c - 'a' + 10);
		} else if ((c >= 'A') && (c <= 'F')) {
			value = (value * base) + (c - 'A' + 10);
		} else if (c == '-') {
			neg *= -1;
		} else if (c == 'x') {
			base = 16;
		}
	} while ((c != EOF) && (c != '\n'));

	return value * neg;
}

void app_display_main_menu(void) {
	printf("\n");
	printf("======================================================\n");
	printf("NXBT version : v%s\n", NXBT_VERSION);
	printf("======================================================\n");
	printf("NXBT profile service main menu :\n");
	printf("[MGT]=================================================\n");
	printf(" %d		=> Get paired device list\n", APP_MGT_MENU_GET_PAIRED_DEV_LIST);
	printf(" %d		=> Enable auto-connection mode\n", APP_MGT_MENU_ENABLE_AUTOCONN);
	printf(" %d		=> Disable auto-connection mode\n", APP_MGT_MENU_DISABLE_AUTOCONN);
	printf(" %d		=> Enable auto-pairing mode\n", APP_MGT_MENU_ENABLE_AUTOPAIR);
	printf(" %d		=> Disable auto-pairing mode\n", APP_MGT_MENU_DISABLE_AUTOPAIR);
	printf(" %d		=> Accept pairing\n", APP_MGT_MENU_PAIR_ACCEPT);
	printf(" %d		=> Reject pairing\n", APP_MGT_MENU_PAIR_REJECT);
	printf(" %d		=> Unpair BT device\n", APP_MGT_MENU_UNPAIR);
	printf(" %d		=> Set discoverable\n", APP_MGT_MENU_ENABLE_DISCOVERABLE);
	printf(" %d		=> Clesr discoverable\n", APP_MGT_MENU_DISABLE_DISCOVERABLE);
#ifdef DISC
	printf(" %d		=> Start discovery\n", APP_MGT_MENU_START_DISCOVERY);
	printf(" %d		=> Stop discovery\n", APP_MGT_MENU_STOP_DISCOVERY);
#endif
	printf("[AVK]=================================================\n");
	printf(" %d		=> AVK connection\n", APP_AVK_MENU_OPEN);
	printf(" %d		=> AVK disconnection\n", APP_AVK_MENU_CLOSE);
	printf(" %d		=> Get connection number\n", APP_AVK_MENU_GET_CONN_NUMBER);
	printf(" %d		=> Get AVK connected remote BT address\n", APP_AVK_MENU_GET_CONN_BD_ADDR);
	printf(" %d		=> Get latest AVK connected device\n", APP_AVK_MENU_GET_LAST_CONN_INFO);
	printf(" %d		=> Start play\n", APP_AVK_MENU_PLAY_START);
	printf(" %d		=> Stop play\n", APP_AVK_MENU_PLAY_STOP);
	printf(" %d		=> Pause play\n", APP_AVK_MENU_PLAY_PAUSE);
	printf(" %d		=> Next play\n", APP_AVK_MENU_PLAY_NEXT);
	printf(" %d		=> Prev play\n", APP_AVK_MENU_PLAY_PREV);
	printf(" %d		=> Open ALSA\n", APP_AVK_MENU_OPEN_AUDIO);
	printf(" %d		=> Close ALSA\n", APP_AVK_MENU_CLOSE_AUDIO);
	printf(" %d		=> Get media elements\n", APP_AVK_MENU_GET_MEDIA_ELEMENT);
	printf("[HS]==================================================\n");
	printf(" %d		=> HS connection\n", APP_HS_MENU_OPEN);
	printf(" %d		=> HS disconnection\n", APP_HS_MENU_CLOSE);
	printf(" %d		=> Get HS connected remote BT address\n", APP_HS_MENU_GET_CONN_BD_ADDR);
	printf(" %d		=> Get latest HS connected device\n", APP_HS_MENU_GET_LAST_CONN_INFO);
	printf(" %d		=> Pickup the call\n", APP_HS_MENU_PICKUP);
	printf(" %d		=> Hangup the call\n", APP_HS_MENU_HANGUP);
	printf(" %d		=> Open audio\n", APP_HS_MENU_OPEN_AUDIO);
	printf(" %d		=> Close audio\n", APP_HS_MENU_CLOSE_AUDIO);
	printf(" %d		=> Mute microphone\n", APP_HS_MENU_MUTE_MIC);
	printf(" %d		=> Unmute microphone\n", APP_HS_MENU_UNMUTE_MIC);
	printf(" %d		=> Dial a phone number\n", APP_HS_MENU_DIAL);
	printf(" %d		=> Redial a phone number\n", APP_HS_MENU_REDIAL);
	printf(" %d		=> Send DTMF AT command\n", APP_HS_MENU_SEND_DTMF);
	printf(" %d		=> Request call indicator\n", APP_HS_MENU_SEND_CIND);
	printf(" %d		=> Request call operater name\n", APP_HS_MENU_SEND_COPS);
	printf(" %d		=> Request current calls\n", APP_HS_MENU_SEND_CLCC);
	printf(" %d		=> Get battery charging status value\n", APP_HS_MENU_GET_BATT);
	printf("[PBC]================================================\n");
	printf(" %d		=> PBC connection\n", APP_PBC_MENU_OPEN);
	printf(" %d		=> PBC disconnection\n", APP_PBC_MENU_CLOSE);
	printf(" %d		=> PBC abort\n", APP_PBC_MENU_ABORT);
	printf(" %d		=> Get contact\n", APP_PBC_MENU_GET_CONTACT);
	printf(" %d		=> Get call history\n", APP_PBC_MENU_GET_CCH);
	printf("[MCE]================================================\n");
	printf(" %d		=> MCE connection\n", APP_MCE_NENU_OPEN);
	printf(" %d		=> MCE disconnection\n", APP_MCE_NENU_CLOSE);
	printf(" %d		=> Start MCE notification server\n", APP_MCE_MENU_START_MNS);
	printf(" %d		=> Stop MCE notification server\n", APP_MCE_MENU_STOP_MNS);
	printf(" %d		=> Get message\n", APP_MCE_MENU_GET_MESSAGE);
	printf("=====================================================\n");
	printf(" %d		=> Quit\n", APP_MENU_QUIT);
	printf("=====================================================\n");
}

#ifdef USE_BACKTRACE

#define CALLSTACK_SIZE 10

static void backtrace_dump(void) {
	int i, nptrs;
	void *buf[CALLSTACK_SIZE + 1];
	char **strings;

	nptrs = backtrace(buf, CALLSTACK_SIZE);
	printf("%s: backtrace() returned %d addresses\n", __FUNCTION__, nptrs);

	strings = backtrace_symbols(buf, nptrs);

	if (strings == NULL) {
		printf("%s: No backtrace captured\n", __FUNCTION__);
		return;
	}

	for (i = 0; i < nptrs; i++) {
		printf("%s\n", strings[i]);
	}

	free(strings);
}

static void sigHandler(int signum) {
	printf("\n%s: Signal %d\n", __FUNCTION__, signum);

	switch(signum ) {
		case SIGILL:
		case SIGABRT:
		case SIGSEGV:
			backtrace_dump();
			break;
		default:
			break;
	}
}
#endif

/* UI callback stub */
static void sendMGTOpenSucceed_stub(void *pObj, int32_t result) {
	// to do : callback
}

static void sendMGTDisconnected_stub(void *pObj) {
	// to do : callback
}

static void sendPairingFailed_stub(void *pObj, int32_t fail_reason) {
	// To do : callback
}

static void updatePairedDevices_stub(void *pObj) {
	// To do : callback
}

static void updateUnpairedDevices_stub(void *pObj) {
	// To do : callback
}

static void sendPairingRequest_stub(void *pObj, bool auto_mode, char *name, unsigned char *bd_addr, int32_t pairing_code) {
	// To do : callback
}

static void sendLinkDownEvent_stub(void *pObj, unsigned char *bd_addr, int32_t reason_code) {
	// To do : callback
}

static void sendAVKOpenFailed_stub(void *pObj) {
	// To do : callback
}

static void sendAVKStreamingStarted_stub(void *pObj, bool alsa_avk_popened) {
	// To do : callback
}

static void sendAVKStreamingStopped_stub(void *pObj) {
	// To do : callback
}

static void updatePlayStatusAVK_stub(void *pObj, int32_t play_status) {
	// To do : callback
}

static void sendAVKConnectionStatus_stub(void *pObj, bool is_connected, char *name, unsigned char *bd_addr) {
	// To do : callback
}

static void sendAVKRCConnectionStatus_stub(void *pObj, bool is_connected) {
	// to do : callback
}

static void updatePlayPositionAVK_stub(void *pObj, int32_t play_pos_msec) {
	// To do : callback
}

static void updateMediaElementsAVK_stub(void *pObj, char *mediaTitle, char *mediaArtist, char *mediaAlbum, char *mediaGenre, int32_t playTime_msec) {
	// To do : callback
}

static void sendHSOpenFailed_stub(void *pObj) {
	// To do : callback
}

static void sendHSConnectionStatus_stub(void *pObj, bool is_connected, char *name, unsigned char *bd_addr) {
	// To do : callback
}

static void sendHSInbandRingSupported_stub(void *pObj, bool is_supported) {
	// to do : callback
}

static void sendHSCallStatus_stub(void *pObj, int32_t call_status) {
	// To do : callback
}

static void sendHSBatteryStatus_stub(void *pObj, int32_t batt_status) {
	// To do : callback
}

static void sendHSCallOperName_stub(void *pObj, char *name) {
	// To do : callback
}

static void sendHSCurrentCalls_stub(void *pObj, char *currentCalls) {
	// To do : callback
}

static void sendHSIncommingCallNumber_stub(void *pObj, char *number) {
	// To do : callback
}

static void sendHSCallIndicator_stub(void *pObj, char *indicator) {
	// To do : callback
}

static void sendPBCOpenFailed_stub(void *pObj) {
	// To do : callback
}

static void sendPBCConnectionStatus_stub(void *pObj, bool is_connected) {
	// To do : callback
}

static void sendNotifyGetPhonebook_stub(void *pObj, int32_t type) {
	// To do : callback
}

static void sendMCEOpenFailed_stub(void *pObj) {
	// To do : callback
}

static void sendMCEConnectionStatus_stub(void *pObj, bool is_connected) {
	// To do : callback
}

static void sendNotifyGetMessageMCE_stub(void *pObj) {
	// To do : callback
}

int main (int argc, char *argv[])
{
	INX_BT *pInstance = getInstance();
	int choice, sel;
	int i, j;
	unsigned char *localAddress = NULL;
	nxbt_paired_dev_t pairedDev;
	nxbt_avk_connected_dev_t connectedDevAVK;
	char number[20];
	char key;
	unsigned char hs_connected_bd_addr[6];
	int dummy;
	void *m_pObjHandler = &dummy;		// UI handler
	Bmessage_info_t bmsg;				// BMessage
	NX_IConfig *pConfig = NULL;			// for config(xml)

#ifdef USE_BACKTRACE
    // Register signal handler for debugging
    signal(SIGILL, sigHandler);
    signal(SIGABRT, sigHandler);
    signal(SIGSEGV, sigHandler);
#endif

	memset(&pairedDev, 0, sizeof(nxbt_paired_dev_t));

	bmsg.fullName = NULL;
	bmsg.phoneNumber = NULL;
	bmsg.msgBody = NULL;

	// Register UI callbacks
	pInstance->registerMGTOpenSucceedCbManager(m_pObjHandler, sendMGTOpenSucceed_stub);
	pInstance->registerMGTDisconnectedCbManager(m_pObjHandler, sendMGTDisconnected_stub);
	pInstance->registerPairingFailedCbManager(m_pObjHandler, sendPairingFailed_stub);
	pInstance->registerPairedDevicesCbManager(m_pObjHandler, updatePairedDevices_stub);
	pInstance->registerUnpairedDevicesCbManager(m_pObjHandler, updateUnpairedDevices_stub);
    pInstance->registerPairingRequestCbManager(m_pObjHandler, sendPairingRequest_stub);
	pInstance->registerLinkDownEventCbManager(m_pObjHandler, sendLinkDownEvent_stub);

	pInstance->registerOpenFailedCbAVK(m_pObjHandler, sendAVKOpenFailed_stub);
	pInstance->registerStreamingStartedCbAVK(m_pObjHandler, sendAVKStreamingStarted_stub);
	pInstance->registerStreamingStoppedCbAVK(m_pObjHandler, sendAVKStreamingStopped_stub);
    pInstance->registerConnectionStatusCbAVK(m_pObjHandler, sendAVKConnectionStatus_stub);
	pInstance->registerConnectionStatusCbAVKRC(m_pObjHandler, sendAVKRCConnectionStatus_stub);
	pInstance->registerPlayStatusCbAVK(m_pObjHandler, updatePlayStatusAVK_stub);
    pInstance->registerMediaElementCbAVK(m_pObjHandler, updateMediaElementsAVK_stub);
    pInstance->registerPlayPositionCbAVK(m_pObjHandler, updatePlayPositionAVK_stub);

	pInstance->registerOpenFailedCbHS(m_pObjHandler, sendHSOpenFailed_stub);
	pInstance->registerConnectionStatusCbHS(m_pObjHandler, sendHSConnectionStatus_stub);
	pInstance->registerInbandRingSupportedCbHS(m_pObjHandler, sendHSInbandRingSupported_stub);
	pInstance->registerCallStatusCbHS(m_pObjHandler, sendHSCallStatus_stub);
	pInstance->registerBatteryStatusCbHS(m_pObjHandler, sendHSBatteryStatus_stub);
	pInstance->registerCallOperNameCbHS(m_pObjHandler, sendHSCallOperName_stub);
	pInstance->registerCurrentCalllsCbHS(m_pObjHandler, sendHSCurrentCalls_stub);
	pInstance->registerIncommingCallNumberCbHS(m_pObjHandler, sendHSIncommingCallNumber_stub);
	pInstance->registerCallIndicatorCbHS(m_pObjHandler, sendHSCallIndicator_stub);

	pInstance->registerOpenFailedCbPBC(m_pObjHandler, sendPBCOpenFailed_stub);
	pInstance->registerConnectionStatusCbPBC(m_pObjHandler, sendPBCConnectionStatus_stub);
	pInstance->registerNotifyGetPhonebookCbPBC(m_pObjHandler, sendNotifyGetPhonebook_stub);

	pInstance->registerOpenFailedCbMCE(m_pObjHandler, sendMCEOpenFailed_stub);
	pInstance->registerConnectionStatusCbMCE(m_pObjHandler, sendMCEConnectionStatus_stub);
	pInstance->registerNotifyGetMessageCbMCE(m_pObjHandler, sendNotifyGetMessageMCE_stub);

	if (pInstance->initDevManager() < 0) {
		goto EXIT;
	}

	localAddress = pInstance->getLocalAddress();

	// Rename local device
	pInstance->renameLocalDevice("NX-Link");
	printf("Local BT name : %s\n", pInstance->getLocalDevName());
	printf("Local BT address : ");
	for (i = 0; i < 6; i++) {
		printf("%02x", localAddress[i]);
	}
	printf("\n");

	for (i = 0; i < pInstance->getPairedDevCount(); i++) {
		pInstance->getPairedDevInfoByIndex(i, pairedDev.pairedDevInfo[i].name, pairedDev.pairedDevInfo[i].bd_addr);
		printf("Paired device name : %s\n", pairedDev.pairedDevInfo[i].name);
	}

	// Read xml and settings to engine
	pConfig = GetConfigHandle();
	if (0 == pConfig->Open(NXBTSERVICE_CONFIG)) {
		char *pBuf = NULL;
		char alsa_playback[100] = {0,};
		char alsa_capture[100] = {0,};
		char alsa_sco_playback[100] = {0,};
		char alsa_sco_capture[100] = {0,};

		// Read bsa_recovery
		if (0 == pConfig->Read("bsa_recovery", &pBuf)) {
			// Set recovery command
			pInstance->setRecoveryCommand(pBuf);
		} else {
			printf("[%s] Read failed : bsa_recovery\n", __FUNCTION__);
		}

		// Read alsa_playback
		if (0 == pConfig->Read("alsa_playback", &pBuf)) {
			strcpy(alsa_playback, pBuf);
		} else {
			printf("[%s] Read failed : alsa_playback\n", __FUNCTION__);
		}

		// Read alsa_capture
		if (0 == pConfig->Read("alsa_capture", &pBuf)) {
			strcpy(alsa_capture, pBuf);
		} else {
			printf("[%s] Read failed : alsa_capture\n", __FUNCTION__);
		}

		// Read alsa_sco_playback
		if (0 == pConfig->Read("alsa_sco_playback", &pBuf)) {
			strcpy(alsa_sco_playback, pBuf);
		} else {
			printf("[%s] Read failed : alsa_sco_playback\n", __FUNCTION__);
		}

		// Read alsa_sco_capture
		if (0 == pConfig->Read("alsa_sco_capture", &pBuf)) {
			strcpy(alsa_sco_capture, pBuf);
		} else {
			printf("[%s] Read failed : alsa_sco_capture\n", __FUNCTION__);
		}

		// Set ALSA device names
		pInstance->setALSADevName(alsa_playback, alsa_capture, alsa_sco_playback, alsa_sco_capture, SYNC_MODE);
	} else {
		printf("[%s] Open failed : %s\n", __FUNCTION__, NXBTSERVICE_CONFIG);
	}
	delete pConfig;

	// Auto connection
	pInstance->autoConnection(pInstance->isAutoConnection());

	// Test command loop
	do {
		app_display_main_menu();
		choice = app_get_choice("Select menu");

		switch (choice) {
			case APP_MGT_MENU_GET_PAIRED_DEV_LIST:
				for (i = 0; i < pInstance->getPairedDevCount(); i++) {
					pInstance->getPairedDevInfoByIndex(i, pairedDev.pairedDevInfo[i].name, pairedDev.pairedDevInfo[i].bd_addr);
					printf("Paired device name : %s, bd_addr : %02x:%02x:%02x:%02x:%02x:%02x\n", pairedDev.pairedDevInfo[i].name, pairedDev.pairedDevInfo[i].bd_addr[0], pairedDev.pairedDevInfo[i].bd_addr[1], pairedDev.pairedDevInfo[i].bd_addr[2], pairedDev.pairedDevInfo[i].bd_addr[3], pairedDev.pairedDevInfo[i].bd_addr[4], pairedDev.pairedDevInfo[i].bd_addr[5]);
				}
				break;
			case APP_MGT_MENU_ENABLE_AUTOCONN:
				pInstance->enableAutoConnection(true);
				sync();
				break;
			case APP_MGT_MENU_DISABLE_AUTOCONN:
				pInstance->enableAutoConnection(false);
				sync();
				break;
			case APP_MGT_MENU_ENABLE_AUTOPAIR:
				pInstance->enableAutoPairing(true);
				sync();
				break;
			case APP_MGT_MENU_DISABLE_AUTOPAIR:
				pInstance->enableAutoPairing(false);
				sync();
				break;
			case APP_MGT_MENU_PAIR_ACCEPT:
				pInstance->acceptPairing();
				break;
			case APP_MGT_MENU_PAIR_REJECT:
				pInstance->rejectPairing();
				break;
			case APP_MGT_MENU_UNPAIR:
				sel = app_get_choice("Select device index");
				pInstance->unpairDevice(sel);
				break;
			case APP_MGT_MENU_ENABLE_DISCOVERABLE:
				pInstance->enableDiscoverable(true);
				break;
			case APP_MGT_MENU_DISABLE_DISCOVERABLE:
				pInstance->enableDiscoverable(false);
				break;
#ifdef DISC
			// Not yet supported
			case APP_MGT_MENU_START_DISCOVERY:
				break;
			case APP_MGT_MENU_STOP_DISCOVERY:
				break;
#endif
			case APP_AVK_MENU_OPEN:
				sel = app_get_choice("Select device index");
				pInstance->connectToAVK(sel);
				break;
			case APP_AVK_MENU_CLOSE:
				sel = app_get_choice("Select device index");
				pInstance->disconnectFromAVK(pairedDev.pairedDevInfo[sel].bd_addr);
				break;
			case APP_AVK_MENU_GET_CONN_NUMBER:
				printf("AVK connection number : %d\n", pInstance->getConnectionNumberAVK());
				break;
			case APP_AVK_MENU_GET_CONN_BD_ADDR:
				for (i = 0; i < pInstance->getConnectionNumberAVK(); i++) {
					pInstance->getConnectionDevAddrAVK(i, connectedDevAVK.connectedDevInfoAVK[i].bd_addr);
					printf("%d : ", i);
				    for (j = 0; j < 6; j++) {
				        printf("%02x", connectedDevAVK.connectedDevInfoAVK[i].bd_addr[j]);
					}
				    printf("\n");
				}
				break;
			case APP_AVK_MENU_GET_LAST_CONN_INFO:
				printf("Lastest AVK connected device index : %d\n", pInstance->requestLastAVKConnectedDevIndex());
				break;
			case APP_AVK_MENU_PLAY_START:
				sel = app_get_choice("Select device index");
				pInstance->playStartAVK(pairedDev.pairedDevInfo[sel].bd_addr);
				break;
			case APP_AVK_MENU_PLAY_STOP:
				sel = app_get_choice("Select device index");
				pInstance->playStopAVK(pairedDev.pairedDevInfo[sel].bd_addr);
				break;
			case APP_AVK_MENU_PLAY_PAUSE:
				sel = app_get_choice("Select device index");
				pInstance->playPauseAVK(pairedDev.pairedDevInfo[sel].bd_addr);
				break;
			case APP_AVK_MENU_PLAY_NEXT:
				sel = app_get_choice("Select device index");
				pInstance->playNextAVK(pairedDev.pairedDevInfo[sel].bd_addr);
				break;
			case APP_AVK_MENU_PLAY_PREV:
				sel = app_get_choice("Select device index");
				pInstance->playPrevAVK(pairedDev.pairedDevInfo[sel].bd_addr);
				break;
			case APP_AVK_MENU_OPEN_AUDIO:
				pInstance->openAudioAVK();
				break;
			case APP_AVK_MENU_CLOSE_AUDIO:
				pInstance->closeAudioAVK();
				break;
			case APP_AVK_MENU_GET_MEDIA_ELEMENT:
				sel = app_get_choice("Select device index");
				pInstance->requestGetElementAttr(pairedDev.pairedDevInfo[sel].bd_addr);
				break;
			case APP_HS_MENU_OPEN:
				sel = app_get_choice("Select device index");
				pInstance->connectToHS(sel);
				break;
			case APP_HS_MENU_CLOSE:
				pInstance->disconnectFromHS();
				break;
			case APP_HS_MENU_GET_CONN_BD_ADDR:
				memset(hs_connected_bd_addr, 0, sizeof(hs_connected_bd_addr));
				if (pInstance->getConnectionDevAddrHS(hs_connected_bd_addr) >= 0) {
					for (i = 0; i < 6; i++) {
						printf("%02x", hs_connected_bd_addr[i]);
					}
					printf("\n");
				}
				break;
			case APP_HS_MENU_GET_LAST_CONN_INFO:
				printf("Lastest HS connected device index : %d\n", pInstance->requestLastHSConnectedDevIndex());
				break;
			case APP_HS_MENU_PICKUP:
				pInstance->pickUpCall();
				break;
			case APP_HS_MENU_HANGUP:
				pInstance->hangUpCall();
				break;
			case APP_HS_MENU_OPEN_AUDIO:
				pInstance->openAudioHS();
				break;
			case APP_HS_MENU_CLOSE_AUDIO:
				pInstance->closeAudioHS();
				break;
			case APP_HS_MENU_MUTE_MIC:
				pInstance->muteMicrophoneHS(true);
				break;
			case APP_HS_MENU_UNMUTE_MIC:
				pInstance->muteMicrophoneHS(false);
				break;
			case APP_HS_MENU_DIAL:
				memset(number, 0, sizeof(number));
				app_get_string("Input dial number : ", number, sizeof(number));
				pInstance->dialPhoneNumber(number);
				break;
			case APP_HS_MENU_REDIAL:
				pInstance->reDialPhoneNumber();
				break;
			case APP_HS_MENU_SEND_DTMF:
				printf("Input DTMF AT command (Dialkeypad key) : ");
				key = getchar();
				pInstance->setATCommandDTMF(key);
				break;
			case APP_HS_MENU_SEND_CIND:
				pInstance->requestIndicator();
				break;
			case APP_HS_MENU_SEND_COPS:
				pInstance->requestCallOperName();
				break;
			case APP_HS_MENU_SEND_CLCC:
				pInstance->requestCurrentCalls();
				break;
			case APP_HS_MENU_GET_BATT:
				printf("Battery charging level[0-5] : %d\n", pInstance->getCurrentBattChargingStatus());
				break;
			case APP_PBC_MENU_OPEN:
				sel = app_get_choice("Select device index");
				pInstance->connectToPBC(sel);
				break;
			case APP_PBC_MENU_CLOSE:
				pInstance->disconnectFromPBC();
				break;
			case APP_PBC_MENU_ABORT:
				pInstance->abortPBC();
				break;
			case APP_PBC_MENU_GET_CONTACT:
				pInstance->getContactFromPBC();
				break;
			case APP_PBC_MENU_GET_CCH:
				pInstance->getCallHistoryFromPBC();
				break;
			case APP_MCE_NENU_OPEN:
				sel = app_get_choice("Select device index");
				pInstance->connectToMCE(sel);
				break;
			case APP_MCE_NENU_CLOSE:
				pInstance->disconnectFromMCE();
				break;
			case APP_MCE_MENU_START_MNS:
				pInstance->startNotifyServerFromMCE();
				break;
			case APP_MCE_MENU_STOP_MNS:
				pInstance->stopNotifyServerFromMCE();
				break;
			case APP_MCE_MENU_GET_MESSAGE:
				pInstance->getParserBmsg(&bmsg);

				if (bmsg.fullName) {
					printf("FN  : %s\n", bmsg.fullName);
				}

				if (bmsg.phoneNumber) {
					printf("TEL : %s\n", bmsg.phoneNumber);
				}

				if (bmsg.msgBody) {
					printf("MSG : %s\n", bmsg.msgBody);
				}

				// Must be released
				if (bmsg.fullName) {
					free(bmsg.fullName);
				}

				if (bmsg.phoneNumber) {
					free(bmsg.phoneNumber);
				}

				if (bmsg.msgBody) {
					free(bmsg.msgBody);
				}
				break;
			case APP_MENU_QUIT:
				printf("Exit program!\n");
				break;
			default:
				printf("Unknown command!\n");
				break;
		}
	} while (choice != APP_MENU_QUIT);

EXIT:
	if (pInstance) {
		delete pInstance;
	}

	return 0;
}
