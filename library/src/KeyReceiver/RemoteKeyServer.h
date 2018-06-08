#ifndef __RemoteKeyServer_h__
#define __RemoteKeyServer_h__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#define REMOTE_KEY_PORT_NO	5151

int32_t StartNetworkReceiver( int16_t portNo );
void StopNetworkReceiver();

#ifdef __cplusplus
}
#endif // __cplusplus


#endif //	__RemoteKeyServer_h__