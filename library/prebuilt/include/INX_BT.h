/*****************************************************************************
 **
 **  Name:        INX_BT.h
 **
 **  Description: Nexell linux bluetooth interface class header.
 **
 **  Copyright (c) 2017, Nexell Corp., All Rights Reserved.
 **  Broadcom BT stack supports. Proprietary and confidential.
 **
 **  Author: Chris Leean.
 **
 *****************************************************************************/

#ifndef __INX_BT_H__
#define __INX_BT_H__

#define NXBT_VERSION	"1.2.0"

typedef struct Bmessage_info {
	char *fullName;
	char *phoneNumber;
	char *msgBody;
} Bmessage_info_t;

class INX_BT {

public:
	INX_BT(void) {}
	virtual	~INX_BT(void) {}

	/* NXBT manager APIs */
	virtual int32_t initDevManager(void) = 0;
	virtual void setRecoveryCommand(const char *command) = 0;
	virtual int32_t enableAutoConnection(bool enable) = 0;
	virtual bool isAutoConnection(void) = 0;
	virtual void autoConnection(bool enable) = 0;
	virtual int32_t requestLastAVKConnectedDevIndex(void) = 0;
	virtual int32_t requestLastHSConnectedDevIndex(void) = 0;
	virtual int32_t acceptPairing(void) = 0;
	virtual int32_t rejectPairing(void) = 0;
	virtual int32_t unpairDevice(int32_t device_index) = 0;
	virtual int32_t enableAutoPairing(bool enable) = 0;
	virtual bool isAutoPairing(void) = 0;
	virtual int32_t enableDiscoverable(bool enable) = 0;
	virtual bool isDiscoverable(void) = 0;
	virtual int32_t renameLocalDevice(const char *name) = 0;
	virtual char* getLocalDevName(void) = 0;
	virtual unsigned char* getLocalAddress(void) = 0;
	virtual int32_t getPairedDevCount(void) = 0;
	virtual int32_t getPairedDevInfoByIndex(int32_t device_index, char *name, unsigned char *bd_addr) = 0;
	virtual int32_t getPairedDevAddrByIndex(int32_t device_index, unsigned char *bd_addr) = 0;
	virtual int32_t getPairedDevNameByIndex(int32_t device_index, char *name) = 0;
	virtual int32_t getPairedDevIndexByAddr(unsigned char *bd_addr) = 0;
	virtual char* getPairedDevNameByAddr(unsigned char *bd_addr) = 0;
	virtual void setALSADevName(const char *playback, const char *capture, const char *playback_bt, const char *capture_bt, bool use_pcm_sync) = 0;

	/* NXBT AVK service APIs */
	virtual int32_t openAudioAVK(void) = 0;
	virtual void closeAudioAVK(void) = 0;
	virtual bool isAudioStatusAVK(void) = 0;
	virtual bool isConnectedAVK(void) = 0;
	virtual int32_t connectToAVK(int32_t device_index) = 0;
	virtual int32_t disconnectFromAVK(unsigned char *bd_addr) = 0;
	virtual int32_t getConnectionNumberAVK(void) = 0;
	virtual int32_t getConnectionDevAddrAVK(int32_t connected_index, unsigned char *bd_addr) = 0;
	virtual int32_t requestGetElementAttr(unsigned char *bd_addr) = 0;
	virtual int32_t playStartAVK(unsigned char *bd_addr) = 0;
	virtual int32_t playStopAVK(unsigned char *bd_addr) = 0;
	virtual int32_t playPauseAVK(unsigned char *bd_addr) = 0;
	virtual int32_t playNextAVK(unsigned char *bd_addr) = 0;
	virtual int32_t playPrevAVK(unsigned char *bd_addr) = 0;

	/* NXBT HS service APIs */
	virtual bool isConnectedHS(void) = 0;
	virtual int32_t requestIndicator(void) = 0;
	virtual int32_t requestCurrentCalls(void) = 0;
	virtual int32_t getConnectionDevAddrHS(unsigned char *bd_addr) = 0;
	virtual int32_t connectToHS(int32_t device_index) = 0;
	virtual int32_t disconnectFromHS(void) = 0;
	virtual int32_t pickUpCall(void) = 0;
	virtual int32_t hangUpCall(void) = 0;
	virtual int32_t openAudioHS(void) = 0;
	virtual int32_t closeAudioHS(void) = 0;
	virtual bool isOpenedAudioHS(void) = 0;
	virtual void muteMicrophoneHS(bool mute) = 0;
	virtual bool isMutedMicrophoneHS(void) = 0;
	virtual int32_t dialPhoneNumber(const char *number) = 0;
	virtual int32_t reDialPhoneNumber(void) = 0;
	virtual int32_t setATCommandDTMF(char key) = 0;
	virtual int32_t requestCallOperName(void) = 0;
	virtual int32_t getCurrentBattChargingStatus(void) = 0;
	virtual int32_t startVoiceRecognition(void) = 0;
	virtual int32_t stopVoiceRecognition(void) = 0;

	/* NXBT PBC service APIs */
	virtual bool isConnectedPBC(void) = 0;
	virtual int32_t connectToPBC(int32_t device_index) = 0;
	virtual int32_t disconnectFromPBC(void) = 0;
	virtual int32_t abortPBC(void) = 0;
	virtual int32_t getContactFromPBC(void) = 0;
	virtual int32_t getCallHistoryFromPBC(void) = 0;

	/* NXBT MCE service APIs */
	virtual bool isConnectedMCE(void) = 0;
	virtual int32_t connectToMCE(int32_t device_index) = 0;
	virtual int32_t disconnectFromMCE(void) = 0;
	virtual int32_t abortMCE(void) = 0;
	virtual int32_t startNotifyServerFromMCE(void) = 0;
	virtual int32_t stopNotifyServerFromMCE(void) = 0;
	virtual int32_t getParserBmsg(Bmessage_info_t *bmsg) = 0;

	/* NXBT UI callback functions */
	virtual void registerMGTOpenSucceedCbManager(void *pObj, void (*cbFunc)(void *, int32_t)) = 0;
	virtual void registerMGTDisconnectedCbManager(void *pObj, void (*cbFunc)(void *)) = 0;
	virtual void registerPairingFailedCbManager(void *pObj, void (*cbFunc)(void *, int32_t)) = 0;
	virtual void registerPairedDevicesCbManager(void *pObj, void (*cbFunc)(void *)) = 0;
	virtual void registerUnpairedDevicesCbManager(void *pObj, void (*cbFunc)(void *)) = 0;
	virtual void registerPairingRequestCbManager(void *pObj, void (*cbFunc)(void *, bool, char *, unsigned char *, int32_t)) = 0;
	virtual void registerLinkDownEventCbManager(void *pObj, void (*cbFunc)(void *, unsigned char *, int32_t)) = 0;
	virtual void registerOpenFailedCbAVK(void *pObj, void (*cbFunc)(void *)) = 0;
	virtual void registerStreamingStartedCbAVK(void *pObj, void (*cbFunc)(void *, bool)) = 0;
	virtual void registerStreamingStoppedCbAVK(void *pObj, void (*cbFunc)(void *)) = 0;
	virtual void registerConnectionStatusCbAVK(void *pObj, void (*cbFunc)(void *, bool, char *, unsigned char *)) = 0;
	virtual void registerConnectionStatusCbAVKRC(void *pObj, void (*cbFunc)(void *, bool)) = 0;
	virtual void registerPlayStatusCbAVK(void *pObj, void (*cbFunc)(void *, int32_t)) = 0;
	virtual void registerMediaElementCbAVK(void *pObj, void (*cbFunc)(void *, char *, char *, char *, char *, int32_t)) = 0;
	virtual void registerPlayPositionCbAVK(void *pObj, void (*cbFunc)(void *, int32_t)) = 0;
	virtual void registerOpenFailedCbHS(void *pObj, void (*cbFunc)(void *)) = 0;
	virtual void registerConnectionStatusCbHS(void *pObj, void (*cbFunc)(void *, bool, char *, unsigned char *)) = 0;
	virtual void registerInbandRingSupportedCbHS(void *pObj, void (*cbFunc)(void *, bool)) = 0;
	virtual void registerCallStatusCbHS(void *pObj, void (*cbFunc)(void *, int32_t)) = 0;
	virtual void registerBatteryStatusCbHS(void *pObj, void (*cbFunc)(void *, int32_t)) = 0;
	virtual void registerCallOperNameCbHS(void *pObj, void (*cbFunc)(void *, char *)) = 0;
	virtual void registerCurrentCalllsCbHS(void *pObj, void (*cbFunc)(void *, char *)) = 0;
	virtual void registerAudioMuteStatusCbHS(void *pObj, void (*cbFunc)(void *, bool, bool)) = 0;
	virtual void registerIncommingCallNumberCbHS(void *pObj, void (*cbFunc)(void *, char *)) = 0;
	virtual void registerCallIndicatorCbHS(void *pObj, void (*cbFunc)(void *, char *)) = 0;
	virtual void registerOpenFailedCbPBC(void *pObj, void (*cbFunc)(void *)) = 0;
	virtual void registerConnectionStatusCbPBC(void *pObj, void (*cbFunc)(void *, bool)) = 0;
	virtual void registerNotifyGetPhonebookCbPBC(void *pObj, void (*cbFunc)(void *, int32_t)) = 0;
	virtual void registerOpenFailedCbMCE(void *pObj, void (*cbFunc)(void *)) = 0;
	virtual void registerConnectionStatusCbMCE(void *pObj, void (*cbFunc)(void *, bool)) = 0;
	virtual void registerNotifyGetMessageCbMCE(void *pObj, void (*cbFunc)(void *)) = 0;
};

extern INX_BT* getInstance(void);

#endif /* __INX_BT_H__ */
