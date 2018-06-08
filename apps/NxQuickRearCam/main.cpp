#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>	//	usleep
#include <stdint.h>
#include "mainwindow.h"
#include <QApplication>
#include <QObject>
#include <QShowEvent>
#include <QHideEvent>

#include <NX_DAudioUtils.h>

static int32_t cbServerData( int32_t iSock, uint8_t *pSendBuf, uint8_t *pRecvBuf, int32_t iMaxBufSize, void *pObj )
{
	INX_IpcManager* pIpcManager = GetIpcManagerHandle();
	MainWindow* pMainWindow = (MainWindow*)pObj;

	uint32_t iKey;
	void* pPayload;
	int32_t iPayloadSize;
	int32_t iRecvSize, iSendSize = 0;

	iRecvSize = pIpcManager->Read( iSock, pRecvBuf, iMaxBufSize );
	iSendSize = NX_IpcMakePacket( NX_REPLY_DONE, NULL, 0, pSendBuf, iMaxBufSize );

	NX_IpcParsePacket( pRecvBuf, iRecvSize, &iKey, &pPayload, &iPayloadSize );

	switch( iKey )
	{
	case NX_REQUEST_PROCESS_HIDE:
	{
		pMainWindow->HideCamera();
		break;
	}

	case NX_REQUEST_FOCUS_VIDEO:
	case NX_REQUEST_FOCUS_VIDEO_TRANSIENT:
	{
		if( pMainWindow->isActiveWindow() )
		{
			if( ((NX_PROCESS_INFO*)pPayload)->iVideoPriority < NX_FOCUS_PRIORITY_MOST )
			{
				iSendSize = NX_IpcMakePacket( NX_REPLY_FAIL, NULL, 0, pSendBuf, iMaxBufSize );
			}
		}
		break;
	}

	default:
		break;
	}

	NxEvent *pEvent = new NxEvent( QEvent::Type(NX_QT_CUSTOM_EVENT_TYPE) );
	pEvent->m_iEventType = iKey;
	QCoreApplication::postEvent( pMainWindow, reinterpret_cast<QEvent*>(pEvent));

	if( ((NX_REQUEST_PROCESS_HIDE == iKey) && pMainWindow->isActiveWindow() && !pMainWindow->isHidden()) ||
		((NX_REQUEST_PROCESS_SHOW == iKey) && !pMainWindow->isActiveWindow() && !pMainWindow->isHidden()) )
	{
		printf(">>>>> NX_ReplyWait(). ( %d )\n", NX_ReplyWait() );
	}

	iSendSize = pIpcManager->Write( iSock, pSendBuf, iSendSize );
	return iSendSize;
}

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	MainWindow w;

	w.setWindowFlags( Qt::Window|Qt::FramelessWindowHint );
	w.setGeometry( 0, 0, 1024, 600 );

	NX_PROCESS_INFO info;
	NX_GetProcessInfo( &info );
	info.iFlags |= NX_PROCESS_FLAG_BACKGROUND;
	info.iVideoPriority = NX_FOCUS_PRIORITY_MOST;

	INX_IpcManager* pIpcManager = GetIpcManagerHandle();
	pIpcManager->RegServerCallbackFunc( &cbServerData, (void*)&w );
	pIpcManager->StartServer( info.szSockName );

	int32_t iRet = NX_RequestCommand( NX_REQUEST_PROCESS_ADD, &info );
	if( NX_REPLY_DONE != iRet )
	{
		printf( "Fail, NX_RequestCommand(). ( cmd: 0x%04X, iRet: %d )\n", NX_REQUEST_PROCESS_ADD, iRet );
		return -1;
	}

	w.hide();

	return a.exec();
}
