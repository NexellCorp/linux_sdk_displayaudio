#include "UDS_Server.h"

//-------------------------------------------------------------------------------------------------
// DEFINE CLASS MEMBER FUNCTIONS
UDS_Server::UDS_Server()
{
    m_bRunning = false;
}

UDS_Server::~UDS_Server()
{
    if (m_bRunning)
        stop();
}

bool UDS_Server::start(char *server_path)
{
    if (m_bRunning) {
        LOG("already running...");
        return false;
    }

    // 1. copy for server path
    strcpy(m_ServerPath, server_path);

    // 2. if server path is exist, remove it.
    if (0 == access(m_ServerPath, F_OK))
        unlink(m_ServerPath);

    // 3. create socket
    //  -. domain: PF_FILE or PF_UNIX (using UDS)
    m_nSocket = socket(PF_FILE, SOCK_DGRAM, 0);
    if (m_nSocket < 0) {
        LOG("socket() failed!");
        return false;
    }

    // 4. initialize - struct sockaddr_un variable for server
    memset(&m_ServerAddr, 0, sizeof(m_ServerAddr));
    m_ServerAddr.sun_family = AF_FILE; // or AF_UNIX
    strcpy(m_ServerAddr.sun_path, m_ServerPath);

    // 5. bind
    if (bind(m_nSocket, (struct sockaddr*)&m_ServerAddr, sizeof(m_ServerAddr)) < 0) {
        LOG("bind() failed!");
        return false;
    }

    memset(&m_ClientAddr, 0, sizeof(m_ClientAddr));
    m_ClientAddr.sun_family = AF_FILE;
    m_nClientAddrSize = sizeof(m_ClientAddr);

    m_bRunning = true;

    return true;
}

bool UDS_Server::stop()
{
    if (m_bRunning) {
        m_bRunning = close(m_nSocket);
        return !m_bRunning;
    } else {
        LOG("already stopped...");
        return false;
    }
}

bool UDS_Server::isRunning()
{
    return m_bRunning;
}

void UDS_Server::read(char* buffer, int length)
{
    recvfrom(m_nSocket, buffer, length, 0, (struct sockaddr*)&m_ClientAddr, (socklen_t*)&m_nClientAddrSize);
}

void UDS_Server::write(char* buffer)
{
    // why buffer length + 1 ? +1 is null character.
    sendto(m_nSocket, buffer, strlen(buffer)+1, 0, (struct sockaddr*)&m_ClientAddr, sizeof(m_ClientAddr));
}

void UDS_Server::write_broadcast(char* buffer)
{
    for (size_t i = 0; i < m_ClientPathList.size(); i++) {        
        strcpy(m_ClientAddr.sun_path, m_ClientPathList[i].c_str());
        write(buffer);
    }
}

void UDS_Server::setClientList(std::vector<std::string> clients)
{
    m_ClientPathList = clients;
}

void UDS_Server::pushClient(std::string client)
{
    m_ClientPathList.push_back(client);
}

std::vector<std::string> UDS_Server::clientList()
{
    return m_ClientPathList;
}

int UDS_Server::indexOfClientList(std::string client)
{
    for (size_t i = 0; i < m_ClientPathList.size(); i++) {
        if (!client.compare(m_ClientPathList[i]))
            return (int)i;
    }

    return -1;
}

void UDS_Server::removeAllClientList()
{
    m_ClientPathList.clear();
}

void UDS_Server::removeOneClientList(int index)
{
    m_ClientPathList.erase(m_ClientPathList.begin() + index);
}

void UDS_Server::removeOneClientList(std::string client)
{
    int index = indexOfClientList(client);
    if (index < 0)
        return;

    removeOneClientList(index);
}
