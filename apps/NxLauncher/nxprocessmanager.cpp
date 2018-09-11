#include <QDebug>
#include <QProcess>

#include <NX_Type.h>
#include <INX_IpcManager.h>
#include <NX_IpcPacket.h>
#include <NX_IpcUtils.h>

#include "nxprocessmanager.h"
#include "nxlauncher.h"

#include <NX_DAudioUtils.h>

#define ENABLE_QPROCESS         1

NxProcessManager::NxProcessManager(QObject *parent)
     : QObject(parent)
     , m_pParent( parent )
{
}

NxProcessManager::~NxProcessManager()
{
}

void NxProcessManager::execute( QString exec, int32_t bDirect )
{
#if 0
    if( !bDirect )
    {
        int32_t iRet=0;

        NX_PROCESS_INFO info;
        memset( &info, 0x00, sizeof(info) );
        QByteArray byteArray = exec.toLatin1();
        memcpy(info.szAppName, byteArray.data(), byteArray.size());

        iRet = NX_RequestCommand( NX_REQUEST_PROCESS_CHECK, &info );
        if( 0 > iRet )
        {
            printf("Fail, NX_RequestCommand(). ( cmd: 0x%04X )\n", NX_REQUEST_PROCESS_CHECK);
            return;
        }

        if( NX_REPLY_FAIL == iRet )
        {
            iRet = NX_RequestCommand( NX_REQUEST_PROCESS_SHOW, &info );
            if( 0 > iRet )
            {
                printf("Fail, NX_RequestCommand(). ( cmd: 0x%04X )\n", NX_REQUEST_PROCESS_SHOW);
                return;
            }

            printf("Show Application.\n");
            return;
        }
    }

#if ENABLE_QPROCESS
    printf("Run Application.\n");
    QProcess* pProcess = new QProcess(this);
    pProcess->setProcessChannelMode( QProcess::ForwardedChannels );

#ifdef QT_X11
    pProcess->start( exec );
#else
    pProcess->start( exec + " -platform wayland" );
#endif
#else
    NX_RunProcess( exec.toStdString().c_str() );
#endif

    // Wait for Process Terminate.
    // pProcess->waitForFinished();
#endif
}

bool NxProcessManager::check( QString exec )
{
#if 0
    int32_t iRet=0;
    NX_PROCESS_INFO info;
    memset( &info, 0x00, sizeof(info) );
    QByteArray byteArray = exec.toLatin1();
    memcpy(info.szAppName, byteArray.data(), byteArray.size());

    iRet = NX_RequestCommand( NX_REQUEST_PROCESS_CHECK, &info );
    if( 0 > iRet )
    {
        printf("Fail, NX_RequestCommand(). ( cmd: 0x%04X )\n", NX_REQUEST_PROCESS_CHECK);
        return false;
    }

    return (iRet == NX_REPLY_FAIL) ? true : false;
#endif

	return false;
}

void NxProcessManager::show( QString exec )
{
#if 0
    int32_t iRet=0;
    NX_PROCESS_INFO info;
    memset( &info, 0x00, sizeof(info) );
    QByteArray byteArray = exec.toLatin1();
    memcpy(info.szAppName, byteArray.data(), byteArray.size());

    iRet = NX_RequestCommand( NX_REQUEST_PROCESS_CHECK, &info );
    if( 0 > iRet )
    {
        printf("Fail, NX_RequestCommand(). ( cmd: 0x%04X )\n", NX_REQUEST_PROCESS_CHECK);
        return;
    }

    if( NX_REPLY_FAIL == iRet )
    {
        iRet = NX_RequestCommand( NX_REQUEST_PROCESS_SHOW, &info );
        if( 0 > iRet )
        {
            printf("Fail, NX_RequestCommand(). ( cmd: 0x%04X )\n", NX_REQUEST_PROCESS_SHOW);
            return;
        }

        printf("Show Process.\n");
        return;
    }
#endif
}

void NxProcessManager::hide( QString exec )
{
#if 0
    int32_t iRet=0;
    NX_PROCESS_INFO info;
    memset( &info, 0x00, sizeof(info) );
    QByteArray byteArray = exec.toLatin1();
    memcpy(info.szAppName, byteArray.data(), byteArray.size());

    iRet = NX_RequestCommand( NX_REQUEST_PROCESS_CHECK, &info );
    if( 0 > iRet )
    {
        printf("Fail, NX_RequestCommand(). ( cmd: 0x%04X )\n", NX_REQUEST_PROCESS_CHECK);
        return;
    }

    if( NX_REPLY_FAIL == iRet )
    {
        iRet = NX_RequestCommand( NX_REQUEST_PROCESS_HIDE, &info );
        if( 0 > iRet )
        {
            printf("Fail, NX_RequestCommand(). ( cmd: 0x%04X )\n", NX_REQUEST_PROCESS_HIDE);
            return;
        }

        printf("Hide Process.\n");
        return;
    }
#endif
}
