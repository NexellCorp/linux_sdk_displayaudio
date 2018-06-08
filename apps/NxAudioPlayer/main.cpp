#include "mainwindow.h"
#include <QApplication>
//#include <QTextCodec>

#define LOG_TAG "[NxAudioPlayer|main]"
#include <NX_Log.h>

static int32_t cbServerData( int32_t iSock, uint8_t *pSendBuf, uint8_t *pRecvBuf, int32_t iMaxBufSize, void *pObj )
{
	INX_IpcManager*		pIpcManager	=	GetIpcManagerHandle();
	MainWindow*			pMainWindow	=	(MainWindow*)pObj;

	uint32_t	iKey;
	void*		pPayload;
	int32_t		iPayloadSize;
	int32_t		iRecvSize, iSendSize = 0;

	iRecvSize = pIpcManager->Read( iSock, pRecvBuf, iMaxBufSize );
	iSendSize = NX_IpcMakePacket( NX_REPLY_DONE, NULL, 0, pSendBuf, iMaxBufSize );

	NX_IpcParsePacket( pRecvBuf, iRecvSize, &iKey, &pPayload, &iPayloadSize );

	switch( iKey )
	{
	case NX_REQUEST_FOCUS_AUDIO:
		pMainWindow->SetTurnOffFlag(true);
		pMainWindow->SaveInfo();
		pMainWindow->Stop();
		pMainWindow->CloseAudio();
		break;

	case NX_REQUEST_FOCUS_AUDIO_TRANSIENT:
		pMainWindow->SetFocusTransientLossFlag(true);
		pMainWindow->SaveInfo();
		pMainWindow->Stop();
		pMainWindow->CloseAudio();
		break;

	case NX_REQUEST_FOCUS_AUDIO_LOSS:
		if(pMainWindow->GetFocusTransientLossFlag())
		{
			pMainWindow->RestoreState();
		}
		break;

	case NX_EVENT_SDCARD_REMOVE:
		pMainWindow->StorageRemoved();
		break;

	case NX_EVENT_USB_REMOVE:
		pMainWindow->StorageRemoved();
		break;

	case NX_EVENT_MEDIA_SCAN_DONE:
		pMainWindow->StorageScanDone();
		break;

	default:
		break;
	}

	NxEvent *pEvent = new NxEvent( QEvent::Type(NX_QT_CUSTOM_EVENT_TYPE) );
	pEvent->m_iEventType = iKey;
	QCoreApplication::postEvent( pMainWindow, reinterpret_cast<QEvent*>(pEvent));

	if(iKey == NX_REQUEST_PROCESS_SHOW)
	{
		NX_ReplyWait();
	}

	iSendSize = pIpcManager->Write( iSock, pSendBuf, iSendSize );
	return iSendSize;
}

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	MainWindow w;
	//QTextCodec::setCodecForLocale(QTextCodec::codecForName("eucKR"));
	w.setWindowFlags(Qt::Window|Qt::FramelessWindowHint);

	NX_PROCESS_INFO procInfo;
	NX_GetProcessInfo( &procInfo );

	INX_IpcManager* pIpcManager = GetIpcManagerHandle();
	pIpcManager->RegServerCallbackFunc( &cbServerData, (void*)&w );
	pIpcManager->StartServer( procInfo.szSockName );

	if( NX_REPLY_DONE != NX_RequestCommand( NX_REQUEST_PROCESS_ADD,  &procInfo ) )
	{
		NXLOGE( "Fail, NX_RequestCommand(). ( NX_REQUEST_PROCESS_ADD )\n");
		return -1;
	}

	int32_t iRet = NX_RequestCommand( NX_REQUEST_FOCUS_AUDIO );
	if( NX_REPLY_DONE != iRet )
	{
		NXLOGE( "Fail, NX_RequestCommand(). ( NX_REQUEST_FOCUS_AUDIO iRet: %d )\n", iRet );
		NX_RequestCommand( NX_REQUEST_PROCESS_REMOVE );
		return -1;
	}
	iRet = NX_RequestCommand( NX_REQUEST_FOCUS_VIDEO );
	if( NX_REPLY_DONE != iRet )
	{
		NXLOGE( "Fail, NX_RequestCommand(). ( NX_REQUEST_FOCUS_VIDEO iRet: %d )\n", iRet );
		NX_RequestCommand( NX_REQUEST_PROCESS_REMOVE );
		return -1;
	}

	w.show();
	return a.exec();
}
