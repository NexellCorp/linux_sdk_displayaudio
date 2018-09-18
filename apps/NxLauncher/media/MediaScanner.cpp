#include "MediaScanner.h"
#include "MediaConf.h"

const char *pstDeviceReserved[] = {
	"mmcblk0"
};

const char *pstMountPosition[] = {
	"/tmp/media",
};

#define LOG_TAG "[MediaScanner]"
#include <NX_Log.h>

MediaScanner::MediaScanner()
{
	m_pUeventManager = new CNX_UeventManager();
	connect(m_pUeventManager, SIGNAL(signalDetectUevent(uint8_t*,uint32_t)), this, SLOT(slotDetectUevent(uint8_t*,uint32_t)));
	m_pUeventManager->Start();

	m_pVolumeManager = new CNX_VolumeManager();
	m_pVolumeManager->SetDeviceReserved(pstDeviceReserved, sizeof(pstDeviceReserved) / sizeof(pstDeviceReserved[0]));
	m_pVolumeManager->SetMountPosition(pstMountPosition, sizeof(pstMountPosition) / sizeof(pstMountPosition[0]));

	m_pMediaScanner = new CNX_MediaScanner();
	connect(m_pMediaScanner, SIGNAL(signalScanDone()), this, SLOT(slotScanDone()));

	connect(&m_StartTimer, SIGNAL(timeout()), this, SLOT(slotStartTimer()));

	Start(3000);
}

MediaScanner::~MediaScanner()
{
	if (m_pUeventManager)
		delete m_pUeventManager;
	if (m_pMediaScanner)
		delete m_pMediaScanner;
}

void MediaScanner::slotDetectUevent(uint8_t *pDesc, uint32_t uiDescSize)
{
	(void)uiDescSize;
	NxEventTypes eType = (NxEventTypes)0;
	NXLOGI("[%s] %s", __FUNCTION__, pDesc);

	for (uint32_t i = 0; i < sizeof(pstDeviceReserved) / sizeof(pstDeviceReserved[0]); i++)
	{
		if (strstr((char*)pDesc, pstDeviceReserved[i]))
			return;
	}

	if (!strncmp((char*)pDesc, NX_UEVENT_STORAGE_ADD, strlen(NX_UEVENT_STORAGE_ADD)) && strstr((char*)pDesc, "mmcblk"))
	{
		eType = E_NX_EVENT_SDCARD_INSERT;
		NXLOGI("[%s] SDCARD_INSERT", __FUNCTION__);
	}
	else if (!strncmp((char*)pDesc, NX_UEVENT_STORAGE_REMOVE, strlen(NX_UEVENT_STORAGE_REMOVE)) && strstr((char*)pDesc, "mmcblk"))
	{
		eType = E_NX_EVENT_SDCARD_REMOVE;
		NXLOGI("[%s] SDCARD_REMOVE", __FUNCTION__);
	}
	else if (!strncmp((char*)pDesc, NX_UEVENT_STORAGE_ADD, strlen(NX_UEVENT_STORAGE_ADD)) && strstr((char*)pDesc, "scsi_disk"))
	{
		eType = E_NX_EVENT_USB_INSERT;
		NXLOGI("[%s] USB_INSERT", __FUNCTION__);
	}
	else if (!strncmp((char*)pDesc, NX_UEVENT_STORAGE_REMOVE, strlen(NX_UEVENT_STORAGE_REMOVE)) && strstr((char*)pDesc, "scsi_disk"))
	{
		eType = E_NX_EVENT_USB_REMOVE;
		NXLOGI("[%s] USB_REMOVE", __FUNCTION__);
	}
	else
		return;

	// notify uevent
	emit signalMediaEvent(eType);

	//
	Start(3000);
}

void MediaScanner::slotScanDone()
{
	emit signalMediaEvent(E_NX_EVENT_MEDIA_SCAN_DONE);
}

void MediaScanner::Start(int msec)
{
	m_StartTimer.stop();
	m_StartTimer.start(msec);
}

void MediaScanner::slotStartTimer()
{
	m_StartTimer.stop();

	start();
}

void MediaScanner::run()
{
	NXLOGI("[%s] <1>", __PRETTY_FUNCTION__);
	NX_VOLUME_INFO *pVolumeInfo = NULL;
	int32_t iVolumeNum = 0;

	m_pVolumeManager->GetMount( &pVolumeInfo, &iVolumeNum );

	char *pDirectory[iVolumeNum];
	for( int32_t i = 0; i < iVolumeNum; i++ )
	{
		pDirectory[i] = (char*)malloc( sizeof(char) * 256 );
		sprintf( pDirectory[i], "%s", pVolumeInfo[i].szVolume );
	}

	m_pMediaScanner->Scan( pDirectory, iVolumeNum);

	for( int32_t i = 0; i < iVolumeNum; i++ )
	{
		free( pDirectory[i] );
	}
	NXLOGI("[%s] <2>", __PRETTY_FUNCTION__);
}
