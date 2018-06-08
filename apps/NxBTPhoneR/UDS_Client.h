#ifndef UDS_CLIENT_H
#define UDS_CLIENT_H

/*
 * IPC Client
 *  - IPC Tools : UDS(Unix Domain Socket)
 *  - Socket : UDP
 */

// for ipc (socket, uds)
#include <sys/types.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

class UDS_Client
{
public:
    UDS_Client();

    ~UDS_Client();

    //-------------------------------------------------------
    // [1] ups socket wrapping functions
    bool start(char* client_path, char* server_path);

    bool stop();

    bool isRunning();

    void read(char* buffer, int length);

    void write(char* buffer);

private:
    bool m_bRunning;

    // why array size is 108 ?
    //   A) reference - struct sockaddr_un -> char sun_path[] size
    //      it's defined <sys/un.h>
    char m_ClientPath[108];
    char m_ServerPath[108];

    // file descriptor for socket
    int m_nSocket;

    struct sockaddr_un m_ServerAddr;
    struct sockaddr_un m_ClientAddr;
    size_t m_nServerAddrSize;
};

#endif // UDS_CLIENT_H
