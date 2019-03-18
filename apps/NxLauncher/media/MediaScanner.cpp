#include "MediaScanner.h"
#include "MediaConf.h"

#define LOG_TAG "[MediaScanner]"
#include <NX_Log.h>

MediaScanner::MediaScanner()
{
	qRegisterMetaType<uint32_t>("uint32_t");
	m_pDiskManager = new CNX_DiskManager();
	connect(m_pDiskManager, SIGNAL(signalDetectUevent(uint32_t,uint8_t*)), this, SLOT(slotDetectUevent(uint32_t,uint8_t*)));
	m_pDiskManager->Start();

	m_pVolumeManager = new CNX_VolumeManager();
	connect(m_pVolumeManager, SIGNAL(signalDetectUevent(uint32_t,uint8_t*)), this, SLOT(slotDetectUevent(uint32_t,uint8_t*)));
	m_pVolumeManager->Start();

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
	if (m_pDiskManager)
		delete m_pDiskManager;
}

void MediaScanner::slotDetectUevent(uint32_t iEventType, uint8_t *pDevice)
{
	NxEventTypes eType = (NxEventTypes)iEventType;
	emit signalMediaEvent(eType);

	NXLOGI("[%s] %s", __FUNCTION__, pDevice);

	switch (eType) {
	case E_NX_EVENT_SDCARD_REMOVE:
	case E_NX_EVENT_USB_REMOVE:
	{
		char command[1024];
		sprintf(command, "umount -f %s", pDevice);
		system(command);
		return;
	}

	case E_NX_EVENT_SDCARD_INSERT:
	case E_NX_EVENT_USB_INSERT:
		return;


	default:
		break;
	}

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
}
