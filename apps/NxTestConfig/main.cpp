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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "NX_IConfig.h"

//------------------------------------------------------------------------------
static void help( void )
{
	printf(
		"usage: options\n"
		"-i [XML File]  : Input XML file.\n"
		"-k [Key]       : Key String.\n"
		"-w [Value]     : Value String for Writing.\n"
		"-r             : Remove.\n"
	);
}

//------------------------------------------------------------------------------
int32_t main( int32_t argc, char **argv )
{
	int32_t iOpt, iRet = -1;
	char *pInFile = NULL, *pKey = NULL, *pValue = NULL;
	int32_t bRemove = false;
	int32_t bWrite = false;

	NX_IConfig *pConfig = NULL;

	while( -1 != (iOpt = getopt(argc, argv, "hi:k:w:r")) )
	{
		switch( iOpt )
		{
		case 'i':
			pInFile = strdup( optarg );
			break;
		case 'k':
			pKey = strdup( optarg );
			break;
		case 'w':
			pValue = strdup( optarg );
			bWrite = true;
			break;
		case 'r':
			bRemove = true;
			break;
		case 'h':
			help();
			return 0;
		default:
			break;
		}
	}

	if( pInFile == NULL || pKey == NULL )
	{
		help();
		goto ERROR;
	}

	pConfig = GetConfigHandle();

	iRet = pConfig->Open( pInFile );
	if( 0 > iRet )
	{
		printf("Fail, Open().\n");
		goto ERROR;
	}

	if( bRemove )
	{
		pConfig->Dump();

		iRet = pConfig->Remove( pKey );
		if( 0 > iRet )
		{
			printf("Fail, Remove().\n");
			goto ERROR;
		}

		printf("[REMOVE] key: %s\n", pKey );
	}
	else if( bWrite )
	{
		iRet = pConfig->Write( pKey, pValue );
		if( 0 > iRet )
		{
			printf("Fail, Write(). ( key: %s, value: %s )\n", pKey, pValue );
			goto ERROR;
		}

		printf("[WRITE] key: %s, value: %s\n", pKey, pValue );
	}
	else
	{
		iRet = pConfig->Read( pKey, &pValue );
		if( 0 > iRet )
		{
			printf("Fail, Read(). ( key: %s )\n", pKey );
			goto ERROR;
		}

		printf("[READ] key: %s, value: %s\n", pKey, pValue );
	}

	pConfig->Dump();

ERROR:
	if( pInFile )	free( pInFile );
	if( pKey )		free( pKey );
	if( pValue )	free( pValue );
	if( pConfig )
	{
		pConfig->Close();
 		delete pConfig;
	}

	return iRet;
}
