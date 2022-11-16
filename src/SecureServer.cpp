#include "SecureServer.h"
#include <iostream>
#include <exception>

using namespace std::chrono_literals;

TLS::SecureServer::SecureServer()
    : m_Ctx(SSL_CTX_new(TLS_server_method()))
{
    initialiseSSLContext();
    createSocketServer();
}

TLS::SecureServer::SecureServer(const ServerConfig &aServerConfig)
    : m_Ctx(SSL_CTX_new(TLS_server_method()))
    , m_ServerConfig(aServerConfig)
    
{
    initialiseSSLContext();
    createSocketServer();
}

TLS::SecureServer::SecureServer(const ServerConfig &aServerConfig, const SSL_METHOD *aTlsVersion)
    : m_Ctx(SSL_CTX_new((aTlsVersion)))
    , m_ServerConfig(aServerConfig)
{
    initialiseSSLContext();
    createSocketServer();
}

void TLS::
SecureServer::initialiseServer() {
    struct hostent *lHostent;
    std::cout << "host is  " << m_ServerConfig.getHost() << ":" << m_ServerConfig.getPort() << std::endl;
    lHostent = gethostbyname(m_ServerConfig.getHost().c_str());
    m_Server.sin_family = AF_INET;
    m_Server.sin_port = htons(m_ServerConfig.getPort());
    m_Server.sin_addr.s_addr = inet_addr(inet_ntoa(*((struct in_addr *)lHostent->h_addr_list[0])));
}

int TLS::
SecureServer::createSocketServer() noexcept(false) {
    m_SocketFD = socket(AF_INET, SOCK_STREAM, 0);
    if (m_SocketFD < 0) {
        perror("Unable to create socket");
        throw std::runtime_error("could not create socket");
    }
    initialiseServer();
    if (bind(m_SocketFD, (struct sockaddr *)&m_Server, sizeof(m_Server)) < 0) {
        perror("Unable to bind");
        throw std::runtime_error("could not bind address");
    }

    if (listen(m_SocketFD, 5) < 0) { //TODO make int configurable, with default value
        perror("Unable to listen");
        throw std::runtime_error("could not start to listen for connections");
    }
    return m_SocketFD;
}

void TLS::SecureServer::initialiseSSLContext() noexcept(false) {
    if (SSL_CTX_use_certificate_file(m_Ctx, m_ServerConfig.getCertificate().c_str(), SSL_FILETYPE_PEM) <= 0) {
        std::cout << "could not load certificate here\n";
        ERR_print_errors_fp(stderr);
        std::runtime_error("could not load certificate");
    }

    if (SSL_CTX_use_PrivateKey_file(m_Ctx, m_ServerConfig.getPriateKey().c_str(), SSL_FILETYPE_PEM) <= 0) {
        std::cout << "could not load private key\n";
        ERR_print_errors_fp(stderr);
        std::runtime_error("could not load private key");
    }
}

bool TLS::
SecureServer::createSSLConnection(SOCKET aAcceptedClient)
{
    // NOTE: Be carefull returning ssl pointer
    SSL *lSsl = SSL_new(m_Ctx);
    SSL_set_fd(lSsl, aAcceptedClient);
    std::cout << __LINE__ << std::endl;
    if (SSL_accept(lSsl) <= 0) {
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
SecureServer::receiveMessage(SOCKET aAcceptedClient) {
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
    //TODO maybe lock needed?
    closeConnection(lSsl, aAcceptedClient);
    std::lock_guard<std::mutex> lk(mtx);
    if(m_ClientConnectionThreads.find(aAcceptedClient) != m_ClientConnectionThreads.end()) {
        m_ClientConnectionThreads[aAcceptedClient].join();
        m_ClientConnectionThreads.erase(aAcceptedClient);
        m_ClientSSLConnections.erase(aAcceptedClient);
    }
    return false;
}

int TLS::SecureServer::readBytes(SSL* lSsl) {
    char inBuffer[1024];
    int lReceiveMessageSize = SSL_read(lSsl, inBuffer, 1024);
    int lReadErrorCode = SSL_get_error(lSsl, lReceiveMessageSize);
    if (SSL_ERROR_NONE == lReadErrorCode) {
        std::cout << lReceiveMessageSize << " bytes has been read" << std::endl;
        return lReceiveMessageSize;
    } else if (SSL_ERROR_WANT_READ == lReadErrorCode) {
        std::cout << "need to read rest data " << std::endl;
        return 0;
    }
    return -1;
}

TLS::SecureServer::SOCKET TLS::SecureServer::accpetExternal() {
    std::cout << __LINE__ << std::endl;
    struct sockaddr_in addr;
    unsigned int lSize = sizeof(addr);
    std::cout << __LINE__ << "socker fd " << m_SocketFD << std::endl;
    SOCKET acceptedClient = accept(m_SocketFD, (sockaddr *)&addr, &lSize);

    if (acceptedClient == -1) {
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
    std::this_thread::sleep_for(2000ms);
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
        if (!createSSLConnection(acceptedClient)) {
            std::cout << "could not create secure connection\n";
            continue;
        }
        std::thread connectionThread(&TLS::SecureServer::receiveMessage, this, acceptedClient);
        std::this_thread::sleep_for(2000ms);
        std::lock_guard<std::mutex> l(mtx);
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
