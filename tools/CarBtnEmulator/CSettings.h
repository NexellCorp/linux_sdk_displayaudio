#ifndef CSETTINGS_H
#define CSETTINGS_H

#include <stdint.h>

typedef struct BTN_SETTINGS{
	//	ADB Informations
	char adbCommand[1024];

	//	Network Informations
	char ipAddress[64];
	uint16_t portNumber;

	//	UART Informations
	char uartPortString[64];
} BTN_SETTINGS;


#ifdef __cplusplus
extern "C"{
#endif

int32_t GetSettings(BTN_SETTINGS **ppSettings);
int32_t SetSettings(BTN_SETTINGS *pSettings);

#ifdef __cplusplus
}
#endif

#endif // CSETTINGS_H
