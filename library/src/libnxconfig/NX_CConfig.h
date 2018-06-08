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

#ifndef __NX_CCONFIG_H__
#define __NX_CCONFIG_H__

#include <stdio.h>

#include <libxml/parser.h>
#include <libxml/xmlmemory.h>

#include <NX_Type.h>
#include "NX_CBase.h"
#include "NX_IConfig.h"

#define NX_XML_CONFIG_ROOT_NODE		"map"
#define NX_XML_CONFIG_VERSION		"1.0"
#define NX_XML_CONFIG_ENCODE_TYPE	"UTF-8"
#define NX_XML_CONFIG_NODE			"string"
#define NX_XML_CONFIG_NODE_PROP		"name"

#define NX_XML_CONFIG_NODE_INDENT	"    "
#define NX_XML_CONFIG_NODE_NEW_LINE	"\n"

class NX_CConfig
	: public NX_IConfig
{
public:
	NX_CConfig();
	~NX_CConfig();

public:
	virtual int32_t Open( const char *pFile );
	virtual void	Close( void );

	virtual int32_t Write( const char *pKey, char *pValue );
	virtual int32_t Read( const char *pKey, char **ppValue );
	virtual int32_t Remove( const char *pKey );

	virtual void	Dump( void );
	virtual const char* GetVersion( void ) { return NX_VERSION_LIBCONFIG; }

private:
	NX_CMutex	m_hLock;
	char*		m_pFile;

	xmlDocPtr	m_hDoc;
	xmlNodePtr	m_hRoot;

	xmlChar*	m_pValue;
	xmlChar*	m_pResult;

private:
	NX_CConfig (const NX_CConfig &Ref);
	NX_CConfig &operator=(const NX_CConfig &Ref);
};

#endif	// __NX_CCONFIG_H__
