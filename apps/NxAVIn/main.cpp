#include "avinmainwindow.h"
#include <QApplication>
#include <QShowEvent>
#include <QHideEvent>
#include <unistd.h>

static int32_t cbServerData( int32_t iSock, uint8_t *pSendBuf, uint8_t *pRecvBuf, int32_t iMaxBufSize, void *pObj )
{
	INX_IpcManager* pIpcManager = GetIpcManagerHandle();
	AVInMainWindow* pMainWindow = (AVInMainWindow*)pObj;

	uint32_t iKey;
	void* pPayload;
	int32_t iPayloadSize;
	int32_t iRecvSize, iSendSize = 0;

	iRecvSize = pIpcManager->Read( iSock, pRecvBuf, iMaxBufSize );
	iSendSize = NX_IpcMakePacket( NX_REPLY_DONE, NULL, 0, pSendBuf, iMaxBufSize );

	NX_IpcParsePacket( pRecvBuf, iRecvSize, &iKey, &pPayload, &iPayloadSize );

	switch( iKey )
	{
	case NX_REQUEST_FOCUS_VIDEO:
	case NX_REQUEST_FOCUS_VIDEO_TRANSIENT:
	{
		pMainWindow->StopAVIn();
		break;
	}

	case NX_REQUEST_PROCESS_HIDE:
		pMainWindow->StopAVIn();
		break;

	case NX_REQUEST_FOCUS_VIDEO_LOSS:
	{
		if( pMainWindow->isActiveWindow() )
		{
			pMainWindow->ShowAVIn();
		}
		break;
	}

	default:
		break;
	}

	NxEvent *pEvent = new NxEvent( QEvent::Type(NX_QT_CUSTOM_EVENT_TYPE) );
	pEvent->m_iEventType = iKey;
	QCoreApplication::postEvent( pMainWindow, reinterpret_cast<QEvent*>(pEvent));

	if( (NX_REQUEST_PROCESS_SHOW == iKey) && !pMainWindow->isActiveWindow() )
	{
		printf(">>>>> NX_ReplyWait(). ( %d )\n", NX_ReplyWait() );
	}

	iSendSize = pIpcManager->Write( iSock, pSendBuf, iSendSize );
	return iSendSize;
}

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	AVInMainWindow w;
	int32_t iRet;

	w.setWindowFlags( Qt::Window|Qt::FramelessWindowHint );
	w.setGeometry( 0, 0, 1024, 600 );

	NX_PROCESS_INFO info;
	NX_GetProcessInfo( &info );

	INX_IpcManager* pIpcManager = GetIpcManagerHandle();
	pIpcManager->RegServerCallbackFunc( &cbServerData, (void*)&w );
	pIpcManager->StartServer( info.szSockName );

	iRet = NX_RequestCommand( NX_REQUEST_PROCESS_ADD, &info );
	if( NX_REPLY_DONE != iRet )
	{
		printf( "Fail, NX_RequestCommand(). ( cmd: 0x%04X, iRet: %d )\n", NX_REQUEST_PROCESS_ADD, iRet );
		return -1;
	}

	iRet = NX_RequestCommand( NX_REQUEST_FOCUS_VIDEO );
	if( NX_REPLY_DONE != iRet )
	{
		printf( "Fail, NX_RequestCommand(). ( cmd: 0x%04X, iRet: %d )\n", NX_REQUEST_FOCUS_VIDEO, iRet );
		goto ERROR;
	}

#if ENABLE_REQUEST_FOCUS_AUDIO
	iRet = NX_RequestCommand( NX_REQUEST_FOCUS_AUDIO );
	if( NX_REPLY_DONE != iRet )
	{
		printf( "Fail, NX_RequestCommand(). ( cmd: 0x%04X, iRet: %d )\n", NX_REQUEST_FOCUS_AUDIO, iRet );
		goto ERROR;
	}
#endif

	w.show();
	w.ShowAVIn();

	return a.exec();

ERROR:
	iRet = NX_RequestCommand( NX_REQUEST_PROCESS_REMOVE );
	if( NX_REPLY_DONE != iRet )
	{
		printf( "Fail, NX_RequestCommand(). ( cmd: 0x%04X, iRet: %d )\n", NX_REQUEST_PROCESS_REMOVE, iRet );
	}

	return -1;
}
