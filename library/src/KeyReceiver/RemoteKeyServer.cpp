#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>

#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <errno.h>	//	errno

#include "NX_KeyReceiver.h"
#include "RemoteKeyServer.h"


class CUDPNetworkReceiver{
public:
	CUDPNetworkReceiver();
	virtual ~CUDPNetworkReceiver();

	bool Start( int16_t portNo );
	void Stop();


	//	for Network Thread
private:
	pthread_t m_hSvrThread;
	bool m_bStarted;
	bool m_bExit;
	int16_t m_Port;
	static void *ServerThreadStub(void *arg)
	{
		( (CUDPNetworkReceiver*) arg )->ServerLoop();
		return (void*)0xdeaddead;
	}
	void ServerLoop();

	//	for Singletone
public:
	static CUDPNetworkReceiver *m_stNetworkReceiver;
	static CUDPNetworkReceiver *GetInstance(){
		if( NULL == m_stNetworkReceiver )
		{
			m_stNetworkReceiver = new CUDPNetworkReceiver();
		}
		return m_stNetworkReceiver;
	}
	int32_t UDPOpen( short port );
};

CUDPNetworkReceiver *CUDPNetworkReceiver::m_stNetworkReceiver = NULL;


CUDPNetworkReceiver::CUDPNetworkReceiver()
	: m_bStarted(false)
	, m_bExit(true)
{
}

CUDPNetworkReceiver::~CUDPNetworkReceiver()
{
}


bool CUDPNetworkReceiver::Start( int16_t portNo )
{
	//	Already Started
	if( m_bStarted )
		return false;

	m_Port = portNo;
	m_bExit = false;
	if( 0 != pthread_create( &m_hSvrThread, NULL, ServerThreadStub, this ) )
	{
		return false;
	}
	return true;
}

void CUDPNetworkReceiver::Stop()
{
	if( m_bStarted )
	{
		m_bExit = false;
		pthread_join( m_hSvrThread, NULL );
	}
}

//
//	UDP Server Part APIs
//
int32_t CUDPNetworkReceiver::UDPOpen( short port )
{
	int32_t svrSock = -1;
	int32_t yes=1;
	struct sockaddr_in svrAddr;

	svrSock  = socket( AF_INET, SOCK_DGRAM, 0 );
	if( svrSock < 0 )
	{
		printf( "Error : server socket \n");
		goto ErrorExit;
	}

	memset( &svrAddr, 0, sizeof( svrAddr ) );
	svrAddr.sin_family = AF_INET;
	svrAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	svrAddr.sin_port = htons( port );

	if( setsockopt(svrSock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int32_t)) == -1 )
	{
		printf( "Error : setsockopt()\n");
		goto ErrorExit;
	}
	if( -1 == bind( svrSock, (struct sockaddr*)&svrAddr, sizeof( svrAddr ) ) )
	{
		printf( "Error : bind()\n");
		goto ErrorExit;
	}
	return svrSock;

ErrorExit:
	if( svrSock > 0 )
	{
		close( svrSock );
	}
	return -1;
}

void CUDPNetworkReceiver::ServerLoop()
{
	int32_t svrSock = UDPOpen(m_Port);
	char data[64];
	struct sockaddr clntAddr;
	socklen_t clientLen = sizeof(clntAddr);
	int32_t byteSize;

	while( !m_bExit )
	{
		byteSize = recvfrom( svrSock, data, sizeof(data), 0, &clntAddr, &clientLen );
		if( byteSize == 8 )
		{
			int32_t *pKeyVal = (int32_t *)((void*)data);
			NXDA_AddKey( pKeyVal[0], pKeyVal[1] );
		}

		if( byteSize < 0 )
			break;
	}

	if( svrSock )
	{
		close( svrSock );
	}
}

//============================================================================
//																			//
//						Exported C Type API									//
//																			//
//============================================================================
static pthread_mutex_t gstCtrlMutex = PTHREAD_MUTEX_INITIALIZER;

int32_t StartNetworkReceiver( int16_t portNo )
{
	int32_t ret = 0;
	pthread_mutex_lock( &gstCtrlMutex );

	CUDPNetworkReceiver *pReceiver = CUDPNetworkReceiver::GetInstance();
	if( pReceiver )
	{
		if( !pReceiver->Start( portNo ) )
			ret = -1;
	}

	pthread_mutex_unlock( &gstCtrlMutex );
	return ret;
}

void StopNetworkReceiver()
{
	pthread_mutex_lock( &gstCtrlMutex );

	CUDPNetworkReceiver *pReceiver = CUDPNetworkReceiver::GetInstance();
	if( pReceiver )
	{
		pReceiver->Stop();
	}

	pthread_mutex_unlock( &gstCtrlMutex );
}
