#include "UDS_Client.h"

#if __dmesg__
#define LOG(X)  do {                \
        fprintf(stderr, "%s\n", X); \
    } while(0)
#else /* !__dmesg__ */
#define LOG(X)  do { } while(0)
#endif

UDS_Client::UDS_Client()
{
    m_bRunning = false;
}

UDS_Client::~UDS_Client()
{
    if (m_bRunning)
        stop();
}

bool UDS_Client::start(char* client_path, char* server_path)
{
    if (m_bRunning) {
        LOG("already running...");
        return false;
    }

    // 1. copy for client/sever path
    strcpy(m_ClientPath, client_path);
    strcpy(m_ServerPath, server_path);

    // 2. if client path is exist, remove it.
    if (0 == access(m_ClientPath, F_OK))
        unlink(m_ClientPath);

    // 3. create socket
    //  -. domain: PF_FILE or PF_UNIX (using UDS)
    m_nSocket = socket(PF_FILE, SOCK_DGRAM, 0);
    if (m_nSocket < 0) {
        LOG("socket() failed!");
        return false;
    }

    // 4. initialize - struct sockaddr_un variable for client
    memset(&m_ClientAddr, 0, sizeof(m_ClientAddr));
    m_ClientAddr.sun_family = AF_UNIX; // or AF_FILE
    strcpy(m_ClientAddr.sun_path, m_ClientPath);

    // 5. bind
    if (bind(m_nSocket, (struct sockaddr*)&m_ClientAddr, sizeof(m_ClientAddr)) < 0) {
        LOG("bind() failed!");
        return false;
    }

    memset(&m_ServerAddr, 0, sizeof(m_ServerAddr));
    m_ServerAddr.sun_family = AF_UNIX;
    strcpy(m_ServerAddr.sun_path, m_ServerPath);
    m_nServerAddrSize = sizeof(m_ServerAddr);

    m_bRunning = true;

    LOG("start ok.");
    return true;
}

bool UDS_Client::stop()
{
    if (m_bRunning) {
        m_bRunning = close(m_nSocket);
        return !m_bRunning;
    } else {
        LOG("already stopped...");
        return false;
    }
}

bool UDS_Client::isRunning()
{
    return m_bRunning;
}

void UDS_Client::read(char* buffer, int length)
{
    recvfrom(m_nSocket, buffer, length, 0, NULL, 0);
}

void UDS_Client::write(char* buffer)
{
    // why buffer length + 1 ? +1 is null character.
    sendto(m_nSocket, buffer, strlen(buffer)+1, 0, (struct sockaddr*)&m_ServerAddr, m_nServerAddrSize);
}
