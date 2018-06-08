//------------------------------------------------------------------------------
//
//	Copyright (C) 2017 Nexell Co. All Rights Reserved
//	Nexell Co. Proprietary & Confidential
//
//	NEXELL INFORMS THAT THIS CODE AND INFORMATION IS PROVIDED "AS IS" BASE
//  AND	WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING
//  BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS
//  FOR A PARTICULAR PURPOSE.
//
//	Module		:
//	File		:
//	Description	:
//	Author		:
//	Export		:
//	History		:
//
//------------------------------------------------------------------------------

#include <string.h>
#include <unistd.h>

#include "CNX_ProcessManager.h"
#include "NX_Utils.h"

#define NX_DTAG	"[CNX_ProcessManager]"
#include "NX_DbgMsg.h"

//------------------------------------------------------------------------------
CNX_ProcessManager::CNX_ProcessManager()
	: m_pList( NULL )
	, m_cbTerminateFunc( NULL )
	, m_pObj( NULL )
{
	m_pList = new CNX_LinkedList();

	Start();
}

//------------------------------------------------------------------------------
CNX_ProcessManager::~CNX_ProcessManager()
{
	Stop();

	if( m_pList )
		delete m_pList;
}

//------------------------------------------------------------------------------
int32_t CNX_ProcessManager::Add( NX_PROCESS_INFO* pInfo )
{
	NX_DbgMsg( NX_DBG_VBS, "%s()++\n", __FUNCTION__ );
	CNX_AutoLock lock( &m_hLock );

	for( int32_t i = 0; i < m_pList->GetCount(); i++ )
	{
		NX_PROCESS_INFO* pSearchInfo = NULL;
		if( 0 > m_pList->Search( (void**)&pSearchInfo, i ) )
			continue;

		if( NULL == pSearchInfo )
			continue;

		if( !strcmp( pSearchInfo->szAppName, pInfo->szAppName) &&
			(pSearchInfo->iProcessId == pInfo->iProcessId) )
		{
			NX_DbgMsg( NX_DBG_INFO, "Fail, Already exist process.\n" );
			NX_DbgMsg( NX_DBG_VBS, "%s()--\n", __FUNCTION__ );
			return -1;
		}
	}

	NX_PROCESS_INFO *pData = (NX_PROCESS_INFO*)malloc( sizeof(NX_PROCESS_INFO) );
	*pData = *pInfo;
	m_pList->Add( pData, CNX_LinkedList::NODE_FRONT );

	NX_DbgMsg( NX_DBG_DEBUG, "Add Process. ( name: %s, pid: %d, flag: 0x%08X, vid-pri: %d, aud-pri: %d, disp: %d )\n",
		pData->szAppName, pData->iProcessId, pData->iFlags, pData->iVideoPriority, pData->iAudioPriority, pData->iDisplayDevice );

	NX_DbgMsg( NX_DBG_VBS, "%s()--\n", __FUNCTION__ );
	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_ProcessManager::Remove( NX_PROCESS_INFO *pInfo )
{
	NX_DbgMsg( NX_DBG_VBS, "%s()++\n", __FUNCTION__ );
	CNX_AutoLock lock( &m_hLock );

	int32_t bRemoved = false;
	for( int32_t i = 0; i < m_pList->GetCount(); i++ )
	{
		NX_PROCESS_INFO* pSearchInfo = NULL;
		if( 0 > m_pList->Search( (void**)&pSearchInfo, i ) )
			continue;

		if( NULL == pSearchInfo )
			continue;

		if( !strcmp( pSearchInfo->szAppName, pInfo->szAppName) &&
			(pSearchInfo->iProcessId == pInfo->iProcessId) )
		{
			NX_DbgMsg( NX_DBG_DEBUG, "Remove Process. ( name: %s, pid: %d )\n",
				pInfo->szAppName, pInfo->iProcessId );

			m_pList->Remove( i );
			free( pSearchInfo );
			bRemoved = true;
			break;
		}
	}

	if( !bRemoved )
	{
		NX_DbgMsg( NX_DBG_INFO, "Fail, Invalid Process.\n");
		NX_DbgMsg( NX_DBG_VBS, "%s()--\n", __FUNCTION__ );
		return -1;
	}

	NX_DbgMsg( NX_DBG_VBS, "%s()--\n", __FUNCTION__ );
	return bRemoved ? 0 : -1;
}

//------------------------------------------------------------------------------
int32_t CNX_ProcessManager::GetCount()
{
	CNX_AutoLock lock( &m_hLock );
	return m_pList->GetCount();
}

//------------------------------------------------------------------------------
int32_t CNX_ProcessManager::GetAt( int32_t iIndex, NX_PROCESS_INFO **ppInfo )
{
	CNX_AutoLock lock( &m_hLock );
	*ppInfo = NULL;
	if( iIndex < 0 || iIndex >= m_pList->GetCount() )
		return -1;

	m_pList->Search( (void**)ppInfo, iIndex );
	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_ProcessManager::IndexOf( NX_PROCESS_INFO *pInfo )
{
	CNX_AutoLock lock( &m_hLock );
	for( int32_t i = 0; i < m_pList->GetCount(); i++ )
	{
		NX_PROCESS_INFO* pSearchInfo = NULL;
		if( 0 > m_pList->Search( (void**)&pSearchInfo, i ) )
			continue;

		if( NULL == pSearchInfo )
			continue;

		if( !strcmp( pSearchInfo->szAppName, pInfo->szAppName) )
		{
			return i;
		}
	}
	return -1;
}

//------------------------------------------------------------------------------
int32_t CNX_ProcessManager::IndexOf( int32_t iFlags )
{
	CNX_AutoLock lock( &m_hLock );
	for( int32_t i = 0; i < m_pList->GetCount(); i++ )
	{
		NX_PROCESS_INFO* pSearchInfo = NULL;
		if( 0 > m_pList->Search( (void**)&pSearchInfo, i ) )
			continue;

		if( NULL == pSearchInfo )
			continue;

		if( pSearchInfo->iFlags & iFlags )
		{
			return i;
		}
	}
	return -1;
}

//------------------------------------------------------------------------------
int32_t CNX_ProcessManager::Swap( int32_t iIndex1, int32_t iIndex2 )
{
	CNX_AutoLock lock( &m_hLock );
	return m_pList->Swap( iIndex1, iIndex2 );
}

//------------------------------------------------------------------------------
int32_t CNX_ProcessManager::Request( int32_t (*cbFunc)( int32_t, NX_PROCESS_INFO*, NX_PROCESS_INFO*, void* ), NX_PROCESS_INFO* pReqInfo, void *pObj )
{
	NX_DbgMsg( NX_DBG_VBS, "%s()++\n", __FUNCTION__ );
	CNX_AutoLock lock( &m_hLock );

	int32_t iRet = 0;
	for( int32_t i = 0; i < m_pList->GetCount(); i++ )
	{
		NX_PROCESS_INFO* pSearchInfo = NULL;
		if( 0 > m_pList->Search( (void**)&pSearchInfo, i ) )
			continue;

		if( NULL == pSearchInfo )
			continue;

		if( cbFunc != NULL )
		{
			iRet = cbFunc( i, pSearchInfo, pReqInfo, pObj );
			if( 0 > iRet )
				break;
		}
	}

	NX_DbgMsg( NX_DBG_VBS, "%s()--\n", __FUNCTION__ );
	return iRet;
}

//------------------------------------------------------------------------------
int32_t CNX_ProcessManager::Request( int32_t (*cbFunc)( int32_t, NX_PROCESS_INFO*, void*, int32_t, void* ), void* pPayload, int32_t iPayloadSize, void *pObj )
{
	NX_DbgMsg( NX_DBG_VBS, "%s()++\n", __FUNCTION__ );
	CNX_AutoLock lock( &m_hLock );

	int32_t iRet = 0;
	for( int32_t i = 0; i < m_pList->GetCount(); i++ )
	{
		NX_PROCESS_INFO* pSearchInfo = NULL;
		if( 0 > m_pList->Search( (void**)&pSearchInfo, i ) )
			continue;

		if( NULL == pSearchInfo )
			continue;

		if( cbFunc != NULL )
		{
			iRet = cbFunc( i, pSearchInfo, pPayload, iPayloadSize, pObj );
			if( 0 > iRet )
				break;
		}
	}

	NX_DbgMsg( NX_DBG_VBS, "%s()--\n", __FUNCTION__ );
	return iRet;
}

//------------------------------------------------------------------------------
int32_t CNX_ProcessManager::Request( int32_t (*cbFunc)( NX_PROCESS_INFO*, void*, int32_t, void* ), NX_PROCESS_INFO* pReqInfo, void *pPayload, int32_t iPayloadSize, void *pObj )
{
	NX_DbgMsg( NX_DBG_VBS, "%s()++\n", __FUNCTION__ );
	CNX_AutoLock lock( &m_hLock );

	if( cbFunc )
	{
		cbFunc( pReqInfo, pPayload, iPayloadSize, pObj );
	}

	NX_DbgMsg( NX_DBG_VBS, "%s()--\n", __FUNCTION__ );
	return 0;
}

//------------------------------------------------------------------------------
void CNX_ProcessManager::RegTerminateCallback( void (*cbTerminateFunc)( void* ), void *pObj )
{
	NX_DbgMsg( NX_DBG_VBS, "%s()++\n", __FUNCTION__ );
	CNX_AutoLock lock( &m_hLock );

	m_cbTerminateFunc = cbTerminateFunc;
	m_pObj = pObj;

	NX_DbgMsg( NX_DBG_VBS, "%s()--\n", __FUNCTION__ );
}

//------------------------------------------------------------------------------
void CNX_ProcessManager::SetProcessInfoToPayload( void **ppPayload, int32_t *iPayloadSize )
{
	NX_DbgMsg( NX_DBG_VBS, "%s()++\n", __FUNCTION__ );
	CNX_AutoLock lock( &m_hLock );

	int32_t iResultSize = sizeof(NX_PROCESS_INFO) * m_pList->GetCount();
	NX_PROCESS_INFO *pResult = (NX_PROCESS_INFO*)malloc( iResultSize );

	for( int32_t i = 0; i < m_pList->GetCount(); i++ )
	{
		NX_PROCESS_INFO *pInfo = NULL;
		if( 0 > m_pList->Search( (void**)&pInfo, i ) )
			continue;

		if( NULL == pInfo )
			continue;

		memcpy( &pResult[i], pInfo, sizeof(NX_PROCESS_INFO) );
	}

	*ppPayload    = (void*)pResult;
	*iPayloadSize = iResultSize;

	NX_DbgMsg( NX_DBG_VBS, "%s()--\n", __FUNCTION__ );
}

//------------------------------------------------------------------------------
int32_t CNX_ProcessManager::PrintInfo()
{
	CNX_AutoLock lock( &m_hLock );

	if( m_pList->GetCount() )
	{
		printf("================================================================================\n");
		for( int32_t i = 0; i < m_pList->GetCount(); i++ )
		{
			NX_PROCESS_INFO* pInfo = NULL;
			if( 0 > m_pList->Search( (void**)&pInfo, i ) )
				continue;

			if( NULL == pInfo )
				continue;

			printf(" [%d] Process Information. ( name: %s, pid: %d, flag: 0x%08X, priority: %d / %d, disp: %d )\n",
				i, pInfo->szAppName, pInfo->iProcessId, pInfo->iFlags,
				pInfo->iVideoPriority, pInfo->iAudioPriority, pInfo->iDisplayDevice );
		}
		printf("================================================================================\n");
	}

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_ProcessManager::RemoveInvalidProcess()
{
	CNX_AutoLock lock( &m_hLock );

	int32_t bIsInvalid = false;
	for( int32_t i = 0; i < m_pList->GetCount(); i++ )
	{
		NX_PROCESS_INFO* pSearchInfo = NULL;
		if( 0 > m_pList->Search( (void**)&pSearchInfo, i ) )
			continue;

		if( NULL == pSearchInfo )
			continue;

		if( !NX_IsValidProcess(pSearchInfo->szAppName, pSearchInfo->iProcessId) )
		{
			NX_DbgMsg( NX_DBG_DEBUG, "Detect Invalid Process. ( %s, %d )\n",
				pSearchInfo->szAppName, pSearchInfo->iProcessId );

			m_pList->Remove( i );
			free( pSearchInfo );

			bIsInvalid = true;
		}
	}

	return bIsInvalid;
}

//------------------------------------------------------------------------------
int32_t CNX_ProcessManager::TerminateProc()
{
	NX_DbgMsg( NX_DBG_VBS, "%s()++\n", __FUNCTION__ );

	if( NULL == m_cbTerminateFunc )
	{
		NX_DbgMsg( NX_DBG_VBS, "%s()--\n", __FUNCTION__ );
		return -1;
	}

	m_cbTerminateFunc( m_pObj );

	NX_DbgMsg( NX_DBG_VBS, "%s()--\n", __FUNCTION__ );
	return 0;
}

//------------------------------------------------------------------------------
void CNX_ProcessManager::ThreadProc()
{
	NX_DbgMsg( NX_DBG_VBS, "%s()++\n", __FUNCTION__ );

	int32_t iDebugCount = 0;
	while( m_bThreadRun )
	{
		if( RemoveInvalidProcess() )
		{
			TerminateProc();
			PrintInfo();
		}

		if( ++iDebugCount > 50 )
		{
			iDebugCount = 0;
			// PrintInfo();
		}

		usleep(100000);
	}

	NX_DbgMsg( NX_DBG_VBS, "%s()--\n", __FUNCTION__ );
}
