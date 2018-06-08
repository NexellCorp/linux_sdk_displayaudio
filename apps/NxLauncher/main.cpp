#include "nxlauncher.h"
#include <QApplication>
#include <QShowEvent>
#include <QHideEvent>

#include <unistd.h>
#include <sys/time.h>

#include <NX_Type.h>
#include <DAudioKeyDef.h>
#include <INX_IpcManager.h>
#include <NX_IpcPacket.h>
#include <NX_IpcUtils.h>
#include <NX_KeyReceiver.h>
#include <NX_PacpClient.h>

#define SCREEN_WIDTH		1024
#define SCREEN_HEIGHT		600

static void cbKeyReceiver( void *pObj, int32_t iKey, int32_t iValue )
{
	Q_UNUSED( iKey );

	if( iValue == KEY_RELEASED )
	{
		QMainWindow* pMainWindow = (QMainWindow*)pObj;

		NxEvent *pEvent = new NxEvent( QEvent::Type(NX_QT_CUSTOM_EVENT_TYPE) );
		pEvent->m_iEventType = iKey;

		QCoreApplication::postEvent( pMainWindow, reinterpret_cast<QEvent*>(pEvent) );
	}
}

static int32_t cbServerData( int32_t iSock, uint8_t *pSendBuf, uint8_t *pRecvBuf, int32_t iMaxBufSize, void *pObj )
{
	INX_IpcManager* pIpcManager = GetIpcManagerHandle();
	QMainWindow* pMainWindow = (QMainWindow*)pObj;

	uint32_t iKey;
	void* pPayload;
	int32_t iPayloadSize;
	int32_t iRecvSize, iSendSize = 0;

	iRecvSize = pIpcManager->Read( iSock, pRecvBuf, iMaxBufSize );
	iSendSize = NX_IpcMakePacket( NX_REPLY_DONE, NULL, 0, pSendBuf, iMaxBufSize );

	NX_IpcParsePacket( pRecvBuf, iRecvSize, &iKey, &pPayload, &iPayloadSize );

	switch( iKey )
	{
	case NX_REQUEST_PROCESS_SHOW:
		printf("NxLauncher: receive NX_REQUEST_PROCESS_SHOW.\n" );
		break;

	case NX_REQUEST_PROCESS_HIDE:
		printf("NxLauncher: receive NX_REQUEST_PROCESS_HIDE.\n" );
		break;

	case NX_REQUEST_FOCUS_VIDEO:
	case NX_REQUEST_FOCUS_VIDEO_TRANSIENT:
	case NX_REQUEST_FOCUS_AUDIO:
	case NX_REQUEST_FOCUS_AUDIO_TRANSIENT:
	{
		NX_DumpProcessInfo( pPayload, iPayloadSize );
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
	NxLauncher w;

#ifdef Q_PROCESSOR_X86
	w.resize( SCREEN_WIDTH, SCREEN_HEIGHT );
#else
	w.setFixedSize( SCREEN_WIDTH, SCREEN_HEIGHT );
	w.setWindowFlags(Qt::FramelessWindowHint);
#endif

	NX_PROCESS_INFO info;
	NX_GetProcessInfo( &info );
	info.iFlags |= NX_PROCESS_FLAG_LAUNCHER;

	INX_IpcManager* pIpcManager = GetIpcManagerHandle();
	pIpcManager->RegServerCallbackFunc( &cbServerData, (void*)&w );
	pIpcManager->StartServer( info.szSockName );

	NXDA_StartKeyProcessing( (void*)&w, cbKeyReceiver );

	//  Process Add
	if( 0 > NX_RequestCommand( NX_REQUEST_PROCESS_ADD, &info ) )
	{
		printf( "Fail, NX_RequestCommand(). ( cmd: 0x%04X )\n", NX_REQUEST_PROCESS_ADD );
		goto ERROR_EXIT;
	}

	//  Process Show
	w.show();

	// bootanimation exit
	system("killall bootanimation");

	return a.exec();

ERROR_EXIT:
	 if( 0 > NX_RequestCommand( NX_REQUEST_PROCESS_REMOVE) )
	{
		printf( "Fail, NX_RequestCommand(). ( cmd: 0x%04X )\n", NX_REQUEST_PROCESS_REMOVE );
	}
	return -1;
}
