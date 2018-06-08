#include <stdint.h>
#include <stdio.h>
#include <pthread.h>

#include "RemoteKeyServer.h"
#include "NX_KeyReceiver.h"

#define MAX_KEY_INPUTS	32

class CKeyPrcessor{
public:
	CKeyPrcessor();
	virtual ~CKeyPrcessor();

	//	Define & Implement Singletone.
	static CKeyPrcessor *m_sKeyProcessor;
	static CKeyPrcessor *GetInstance()
	{
		if( NULL == m_sKeyProcessor )
		{
			m_sKeyProcessor = new CKeyPrcessor();
		}
		return m_sKeyProcessor;
	}

	int32_t AddKey( int32_t key, int32_t value );

	int32_t Start( void *pAppPrivate, void (*callback)(void*,int32_t,int32_t) );
	void Stop();

	//	Thread
private:
	bool m_bStarted;
	bool m_bExitLoop;
	pthread_t m_hKeyProcThread;
	static void *KeyThreadStub( void *arg ){
		((CKeyPrcessor *)arg)->KeyProcessingLoop();
		return (void*)0xdeaddead;
	}
	void KeyProcessingLoop();

private:
	int32_t m_Key[MAX_KEY_INPUTS];
	int32_t m_Value[MAX_KEY_INPUTS];
	int32_t m_CurSize;
	int32_t m_ReadPos;
	int32_t m_WritePos;
	pthread_mutex_t m_KeyLock;
	pthread_cond_t m_KeyCond;

private:
	void *m_pAppPrivate;
	void (*m_KeyCallback)(void *, int32_t, int32_t);
};

CKeyPrcessor *CKeyPrcessor::m_sKeyProcessor = NULL;


CKeyPrcessor::CKeyPrcessor()
	: m_bStarted(false)
	, m_CurSize(0)
	, m_ReadPos(0)
	, m_WritePos(0)
	, m_pAppPrivate(NULL)
	, m_KeyCallback(NULL)
{
	pthread_mutex_init( &m_KeyLock, NULL );
	pthread_cond_init( &m_KeyCond, NULL );
	//	Make default value into invalid.
	for( int32_t i=0 ; i<MAX_KEY_INPUTS ; i++ )
	{
		m_Key[i] = -1;
		m_Value[i] = -1;
	}

	if( 0 != StartNetworkReceiver( REMOTE_KEY_PORT_NO ) )
	{
		printf("StartNetworkReceiver Failed!!!\n");
	}
}

CKeyPrcessor::~CKeyPrcessor()
{
	StopNetworkReceiver();
}

int32_t CKeyPrcessor::AddKey( int32_t key, int32_t value )
{
	int32_t ret = -1;
	pthread_mutex_lock (&m_KeyLock);
	if( MAX_KEY_INPUTS > m_CurSize  )
	{
		m_Key[m_WritePos] = key;
		m_Value[m_WritePos] = value;
		m_WritePos = (m_WritePos + 1)%MAX_KEY_INPUTS;
		ret = ++m_CurSize;
		pthread_cond_signal(&m_KeyCond);
	}
	pthread_mutex_unlock (&m_KeyLock);
	return ret;
}
int32_t CKeyPrcessor::Start( void *pAppPrivate, void (*callback)(void*,int32_t,int32_t) )
{
	if( !m_bStarted )
	{
		m_bExitLoop = 0;
		m_pAppPrivate = pAppPrivate;
		m_KeyCallback = callback;
		if( 0 != pthread_create( &m_hKeyProcThread, NULL, KeyThreadStub, this ) )
		{
			m_KeyCallback = NULL;
			return -1;
		}
		m_bStarted = true;
	}
	return 0;
}
void CKeyPrcessor::Stop()
{
	if( m_bStarted )
	{
		m_bExitLoop = true;
		pthread_cond_signal(&m_KeyCond);	//	Send dummy for closing
		pthread_join( m_hKeyProcThread, NULL );
	}
}
void CKeyPrcessor::KeyProcessingLoop()
{
	int32_t key, value;
	while( !m_bExitLoop )
	{
		key = -1;
		pthread_mutex_lock (&m_KeyLock);
		if( 0 < m_CurSize )
		{
			m_CurSize --;
			//printf("key = 0x%08x, 0x%08x\n", m_Key[m_ReadPos], m_Value[m_ReadPos]);
			//	make old value into invalid.
			key = m_Key[m_ReadPos];
			value = m_Value[m_ReadPos];
			m_Key[m_ReadPos] = -1;
			m_Value[m_ReadPos] = -1;
			m_ReadPos = (m_ReadPos + 1)%MAX_KEY_INPUTS;
		}
		else{
			pthread_cond_wait( &m_KeyCond, &m_KeyLock );
		}
		pthread_mutex_unlock (&m_KeyLock);
		if( -1 != key && m_KeyCallback )
		{
			m_KeyCallback( m_pAppPrivate, key, value );
		}
	}
}



//============================================================================
//																			//
//						Exported C Type API									//
//																			//
//============================================================================
static pthread_mutex_t gstCtrlMutex = PTHREAD_MUTEX_INITIALIZER;

int32_t NXDA_StartKeyProcessing( void *pAppPrivate, void (*callback)(void *, int32_t, int32_t) )
{
	int32_t ret = -1;
	pthread_mutex_lock( &gstCtrlMutex );
	CKeyPrcessor *pKeyProc = CKeyPrcessor::GetInstance();
	if( pKeyProc )
	{
		ret = pKeyProc->Start( pAppPrivate, callback );
	}
	pthread_mutex_unlock( &gstCtrlMutex );
	return ret;
}

int32_t NXDA_AddKey( int32_t key, int32_t value )
{
	int32_t ret = -1;
	pthread_mutex_lock( &gstCtrlMutex );
	CKeyPrcessor *pKeyProc = CKeyPrcessor::GetInstance();
	if( pKeyProc )
	{
		ret = pKeyProc->AddKey( key, value );
	}
	pthread_mutex_unlock( &gstCtrlMutex );
	return ret;
}

void NXDA_StopKeyProcessing()
{
	pthread_mutex_lock( &gstCtrlMutex );
	CKeyPrcessor *pKeyProc = CKeyPrcessor::GetInstance();
	if( pKeyProc )
	{
		pKeyProc->Stop();
	}
	pthread_mutex_unlock( &gstCtrlMutex );
}
