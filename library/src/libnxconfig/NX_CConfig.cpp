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

#include <libxml/xmlstring.h>

#include "NX_CConfig.h"

//------------------------------------------------------------------------------
NX_CConfig::NX_CConfig()
	: m_pFile( NULL )
	, m_hDoc( NULL )
	, m_hRoot( NULL )
{

}

//------------------------------------------------------------------------------
NX_CConfig::~NX_CConfig()
{
	Close();
}

//------------------------------------------------------------------------------
int32_t NX_CConfig::Open( const char *pFile )
{
	NX_CAutoLock lock( &m_hLock );

	m_pFile = strdup( pFile );

	if( !access( m_pFile, F_OK ) )
	{
		m_hDoc = xmlParseFile( m_pFile );
		if( NULL == m_hDoc )
		{
			printf("Fail, xmlParseFile(). ( %s )\n", m_pFile );
			goto ERROR;
		}

		m_hRoot = xmlDocGetRootElement( m_hDoc );
		if( NULL == m_hRoot )
		{
			printf("Fail, xmlDocGetRootElement().\n");	// Empty Document.
			goto ERROR;
		}

		if( xmlStrcmp( m_hRoot->name, (const xmlChar*)NX_XML_CONFIG_ROOT_NODE ) )
		{
			printf("Fail, invalid root node name. ( %s )\n", m_hRoot->name );
			goto ERROR;
		}
	}
	else
	{
		m_hDoc = xmlNewDoc( (const xmlChar*)NX_XML_CONFIG_VERSION );
		if( NULL == m_hDoc )
		{
			printf("Fail, xmlNewDoc().\n" );
			goto ERROR;
		}

		m_hRoot = xmlNewNode( NULL, (const xmlChar*)NX_XML_CONFIG_ROOT_NODE );
		xmlDocSetRootElement( m_hDoc, m_hRoot );

		xmlNodePtr hNew = xmlNewText( (const xmlChar*)NX_XML_CONFIG_NODE_NEW_LINE );
		xmlAddChild( m_hRoot, hNew );

		xmlSaveFileEnc( m_pFile, m_hDoc, NX_XML_CONFIG_ENCODE_TYPE );
	}

	return 0;

ERROR:
	if( m_pFile )	free( m_pFile );
	if( m_hDoc )	xmlFreeDoc( m_hDoc );

	return -1;
}

//------------------------------------------------------------------------------
void NX_CConfig::Close( void )
{
	NX_CAutoLock lock( &m_hLock );

	if( m_hRoot )
	{

	}

	if( m_hDoc )
	{
		xmlFreeDoc( m_hDoc );
		m_hDoc = NULL;
	}

	if( m_pFile )
	{
		free( m_pFile );
	}

	xmlCleanupParser();
}

//------------------------------------------------------------------------------
int32_t NX_CConfig::Write( const char *pKey, char *pValue )
{
	NX_CAutoLock lock( &m_hLock );

	if( !m_hDoc || !m_hRoot || !m_pFile )
		return -1;

	xmlNodePtr hCur = m_hRoot->xmlChildrenNode;
	while( NULL != hCur )
	{
		xmlChar* pProperty = xmlGetProp( hCur, (const xmlChar*)NX_XML_CONFIG_NODE_PROP );
		if( !xmlStrcmp( pProperty, (const xmlChar*)pKey ) )
		{
			xmlNodeSetContent( hCur, (const xmlChar*)pValue );
			xmlSaveFileEnc( m_pFile, m_hDoc, NX_XML_CONFIG_ENCODE_TYPE );

			return 0;
		}

		hCur = hCur->next;
	}

	xmlNodePtr hNew;
	hNew = xmlNewText( (const xmlChar*)NX_XML_CONFIG_NODE_INDENT );
	xmlAddChild( m_hRoot, hNew );

	hNew = xmlNewNode( NULL, (const xmlChar*)NX_XML_CONFIG_NODE );
	xmlNewProp( hNew, (const xmlChar*)NX_XML_CONFIG_NODE_PROP, (const xmlChar*)pKey );
	xmlNodeSetContent( hNew, (const xmlChar*)pValue );
	xmlAddChild( m_hRoot, hNew );

	hNew = xmlNewText( (const xmlChar*)NX_XML_CONFIG_NODE_NEW_LINE );
	xmlAddChild( m_hRoot, hNew );

	xmlSaveFileEnc( m_pFile, m_hDoc, NX_XML_CONFIG_ENCODE_TYPE );

	return 0;
}

//------------------------------------------------------------------------------
int32_t NX_CConfig::Read( const char *pKey, char **pValue )
{
	NX_CAutoLock lock( &m_hLock );

	if( !m_hDoc || !m_hRoot || !m_pFile )
		return -1;

	xmlNodePtr hCur = m_hRoot->xmlChildrenNode;
	while( NULL != hCur )
	{
		xmlChar* pProperty = xmlGetProp( hCur, (const xmlChar*)NX_XML_CONFIG_NODE_PROP );
		if( !xmlStrcmp( pProperty, (const xmlChar*)pKey ) )
		{
			xmlChar* pContents = xmlNodeGetContent( hCur );
			*pValue = (char*)pContents;
			return 0;
		}

		hCur = hCur->next;
	}

	return -1;
}

//------------------------------------------------------------------------------
int32_t NX_CConfig::Remove( const char *pKey )
{
	NX_CAutoLock lock( &m_hLock );

	if( !m_hDoc || !m_hRoot || !m_pFile )
		return -1;

	xmlNodePtr hCur = m_hRoot->xmlChildrenNode;
	while( NULL != hCur )
	{
		xmlChar* pProperty = xmlGetProp( hCur, (const xmlChar*)NX_XML_CONFIG_NODE_PROP );
		if( !xmlStrcmp( pProperty, (const xmlChar*)pKey ) )
		{
			xmlUnlinkNode( hCur->prev );
			xmlUnlinkNode( hCur );

			xmlFreeNode( hCur->prev );
			xmlFreeNode( hCur );

			xmlSaveFileEnc( m_pFile, m_hDoc, NX_XML_CONFIG_ENCODE_TYPE );
			return 0;
		}

		hCur = hCur->next;
	}

	return -1;
}

//------------------------------------------------------------------------------
void NX_CConfig::Dump( void )
{
	if( !m_hDoc )
		return ;

	int32_t iSize;
	xmlChar* pResult;
	xmlDocDumpMemory( m_hDoc, &pResult, &iSize );

	printf("============================================================\n");
	printf(" XML File : %s\n", m_pFile );
	printf("============================================================\n");
	printf("%s", pResult );
	printf("============================================================\n");

	xmlFree( pResult );
}

//------------------------------------------------------------------------------
NX_IConfig* GetConfigHandle()
{
	return (NX_IConfig*)new NX_CConfig();
}
