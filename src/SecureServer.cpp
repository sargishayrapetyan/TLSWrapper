#include "SecureServer.h"
#include <iostream>
using namespace std::chrono_literals;

TLS::SecureServer::SecureServer()
    : m_Ctx(SSL_CTX_new(TLS_server_method()))
{
    initContext();
    createSocket();
}

TLS::SecureServer::SecureServer(const ServerConfig &aServerConfig)
    : m_ServerConfig(aServerConfig)
    , m_Ctx(SSL_CTX_new(TLS_server_method()))
{
    initContext();
    createSocket();
}

TLS::SecureServer::SecureServer(const ServerConfig &aServerConfig, const SSL_METHOD *aTlsVersion)
    : m_ServerConfig(aServerConfig)
    , m_Ctx(SSL_CTX_new((aTlsVersion)))
{
    initContext();
    createSocket();
    acceptClient();
}

int TLS::
    SecureServer::createSocket()
{

    struct sockaddr_in lSockAddrIn;
    struct hostent *lHostent;
    std::cout << "host is  " << m_ServerConfig.getHost() << ":" << m_ServerConfig.getPort() << std::endl;
    lHostent = gethostbyname(m_ServerConfig.getHost().c_str());
    lSockAddrIn.sin_family = AF_INET;
    lSockAddrIn.sin_port = htons(m_ServerConfig.getPort());
    lSockAddrIn.sin_addr.s_addr = inet_addr(inet_ntoa(*((struct in_addr *)lHostent->h_addr_list[0])));

    m_SocketFD = socket(AF_INET, SOCK_STREAM, 0);
    if (m_SocketFD < 0)
    {
        perror("Unable to create socket");
        exit(EXIT_FAILURE);
    }

    if (bind(m_SocketFD, (struct sockaddr *)&lSockAddrIn, sizeof(lSockAddrIn)) < 0)
    {
        perror("Unable to bind");
        exit(EXIT_FAILURE);
    }

    if (listen(m_SocketFD, 2) < 0)
    {
        perror("Unable to listen");
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }
    return m_SocketFD;
}

void TLS::SecureServer::initContext()
{
    if (SSL_CTX_use_certificate_file(m_Ctx, m_ServerConfig.getCertificate().c_str(), SSL_FILETYPE_PEM) <= 0)
    {
        std::cout << "could not load certificate here\n";
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    if (SSL_CTX_use_PrivateKey_file(m_Ctx, m_ServerConfig.getPriateKey().c_str(), SSL_FILETYPE_PEM) <= 0)
    {
        std::cout << "could not load private key\n";
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }
}

bool TLS::
SecureServer::createSSLConnection(SOCKET aAcceptedClient)
{
    // NOTE: Be carefull returning ssl pointer
    SSL *lSsl = SSL_new(m_Ctx);
    SSL_set_fd(lSsl, aAcceptedClient);
    std::cout << __LINE__ << std::endl;
    if (SSL_accept(lSsl) <= 0)
    {
        std::cout << "could not create ssl connection with accepted client\n";
        ERR_print_errors_fp(stderr);
        return false;
    }
    std::cout << "connection with client" << aAcceptedClient << " has been accepted\n";
    m_ClientSSLConnections.emplace(aAcceptedClient, lSsl);
    return true;
}

void TLS::SecureServer::closeConnection(SSL *aSsl, SOCKET aClient)
{
    SSL_shutdown(aSsl);
    SSL_free(aSsl);
    shutdown(aClient, SHUT_RDWR);
    close(aClient);
}

bool TLS::
SecureServer::receiveMessage(SOCKET aAcceptedClient)
{
    std::cout << __FUNCTION__ << "\n";
    std::lock_guard<std::mutex> l(mtx);
    SSL *lSsl = m_ClientSSLConnections[aAcceptedClient];
    if (lSsl) {
        std::cout << "fine" << std::endl;
    } else {
        std::cout << "not fine" << std::endl;
    }
    while(readBytes(lSsl) >=0 ) {
        
    }
    std::cout << "failed to read from client\n";
    closeConnection(lSsl, aAcceptedClient);
    m_ClientConnectionThreads[aAcceptedClient].join();
    m_ClientConnectionThreads.erase(aAcceptedClient);
    m_ClientSSLConnections.erase(aAcceptedClient);
    return false;
}

int TLS::SecureServer::readBytes(SSL* lSsl) {
    char inBuffer[1024];
    std::cout << "read bytes\n";
    int lReceiveMessageSize = SSL_read(lSsl, inBuffer, 1024);
    std::cout << "read bytes      2222\n";
    int lReadErrorCode = SSL_get_error(lSsl, lReceiveMessageSize);
    if (SSL_ERROR_NONE == lReadErrorCode) {
        std::cout << lReceiveMessageSize << " bytes has been read" << std::endl;
        return lReceiveMessageSize;
    }
    else if (SSL_ERROR_WANT_READ == lReadErrorCode)
    {
        std::cout << "need to read rest data " << std::endl;
        return 0;
    }
    return -1;
}

// TLS::SecureServer::SOCKET TLS::
// SecureServer::getConnectionId() {
//     return acceptedSocket;//todo
// }

TLS::SecureServer::SOCKET TLS::SecureServer::accpetExternal() {
    std::cout << __LINE__ << std::endl;
    struct sockaddr_in addr;
    unsigned int lSize = sizeof(addr);
    std::cout << __LINE__ << "socker fd " << m_SocketFD << std::endl;
    SOCKET acceptedClient = accept(m_SocketFD, (sockaddr *)&addr, &lSize);

    if (acceptedClient == -1)
    {
        perror("Unable to accept");
        exit(EXIT_FAILURE);
    }
    
    if(!createSSLConnection(acceptedClient)) {
        return -1; //invlaid socket
    }
    std::thread connectionThread(&TLS::SecureServer::receiveMessage, this, acceptedClient);
    std::this_thread::sleep_for(2000ms);
    // std::cout << __FUNCTION__ << __LINE__ << std::endl;
    // clientConnectionThread.m_Thread.swap(connectionThread);
    // std::cout <<__FUNCTION__ <<__LINE__ << std::endl;
    // std::lock_guard<std::mutex> l(mtx);
    m_ClientConnectionThreads[acceptedClient].swap(connectionThread);
    std::cout << __FUNCTION__ << __LINE__ << std::endl;
    std::cout << "Client accepted....\n";
    return acceptedClient;
}

void TLS::SecureServer::acceptClient()
{
    while (true)
    {
        std::cout << "Starting to accpet clients....\n";
        struct sockaddr_in addr;
        unsigned int lSize = sizeof(addr);
        SOCKET acceptedClient = accept(m_SocketFD, (sockaddr *)&addr, &lSize);

        if (acceptedClient == -1) {
            perror("Unable to accept");
            exit(EXIT_FAILURE);
        }
        //ConnectionThread clientConnectionThread{};
        //createSSLConnection(acceptedClient);
        if (!createSSLConnection(acceptedClient)) {
            std::cout << "could not create secure connection\n";
            continue;
        }
        std::thread connectionThread(&TLS::SecureServer::receiveMessage, this, acceptedClient);
        std::this_thread::sleep_for(2000ms);
        //std::cout << __FUNCTION__ << __LINE__ << std::endl;
        //clientConnectionThread.m_Thread.swap(connectionThread);
        //std::cout <<__FUNCTION__ <<__LINE__ << std::endl;
        //std::lock_guard<std::mutex> l(mtx);
        m_ClientConnectionThreads[acceptedClient].swap(connectionThread);
        std::cout <<__FUNCTION__ <<__LINE__ << std::endl;
        std::cout << "Client accepted....\n";
        std::this_thread::sleep_for(2000ms);
    }
    std::cout << __LINE__ << std::endl;
}

TLS::SecureServer::~SecureServer() noexcept
{
    // TODO close all connections properly
    //    SSL_shutdown(m_Ssl);
    //    SSL_free(m_Ssl);
    close(m_SocketFD);
    SSL_CTX_free(m_Ctx);
}
