#ifndef __NX_KeyReceiver_h__
#define __NX_KeyReceiver_h__

#include <stdint.h>

#ifdef __cplusplus
extern "C"{
#endif// __cplusplus

//
//	Description
//		This function start key processing engine.
//	Parameter
//		pAppPrivate : Application's private pointer address.
//
int32_t NXDA_StartKeyProcessing( void *pAppPrivate, void (*callback)(void *, int32_t, int32_t) );
int32_t NXDA_AddKey( int32_t key, int32_t value );
void NXDA_StopKeyProcessing();

#ifdef __cplusplus
}
#endif// __cplusplus

#endif 	//__NX_KeyReceiver_h__