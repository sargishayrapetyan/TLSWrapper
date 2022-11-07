#include "SecureServer.h"
#include <iostream>

TLS::SecureServer::SecureServer()
//    : m_Ssl(nullptr)
    : m_Ctx(SSL_CTX_new(TLS_server_method()))
{
    initContext();
    createSocket();
}

TLS::SecureServer::SecureServer(const ServerConfig& aServerConfig)
    : m_ServerConfig(aServerConfig)
//    , m_Ssl(nullptr)
    , m_Ctx(SSL_CTX_new(TLS_server_method()))
{
    initContext();
    createSocket();
}

TLS::SecureServer::SecureServer(const ServerConfig& aServerConfig, const SSL_METHOD* aTlsVersion)   
    : m_ServerConfig(aServerConfig)
//    , m_Ssl(nullptr)
    , m_Ctx(SSL_CTX_new((aTlsVersion)))
{
    initContext();
    createSocket();
    acceptClient();
}

int  TLS::
SecureServer::createSocket() {
    
    struct sockaddr_in lSockAddrIn;
    struct hostent *lHostent;
    std::cout << "host is  " << m_ServerConfig.getHost() << ":" <<m_ServerConfig.getPort() <<std::endl;
    lHostent = gethostbyname(m_ServerConfig.getHost().c_str());
    lSockAddrIn.sin_family = AF_INET;
    lSockAddrIn.sin_port = htons(m_ServerConfig.getPort());
    lSockAddrIn.sin_addr.s_addr = inet_addr(inet_ntoa(*((struct in_addr *)lHostent->h_addr_list[0])));

    m_SocketFD = socket(AF_INET, SOCK_STREAM, 0);
    if (m_SocketFD < 0) {
        perror("Unable to create socket");
        exit(EXIT_FAILURE);
    }

    if (bind(m_SocketFD, (struct sockaddr*)&lSockAddrIn, sizeof(lSockAddrIn)) < 0) {
        perror("Unable to bind");
        exit(EXIT_FAILURE);
    }
    std::cout << "socket fd " << m_SocketFD << std::endl;

    if (listen(m_SocketFD, 2) < 0) {
        perror("Unable to listen");
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }
    return m_SocketFD;
}

void TLS::SecureServer::initContext() {
    if (SSL_CTX_use_certificate_file(m_Ctx, m_ServerConfig.getCertificate().c_str(), SSL_FILETYPE_PEM) <= 0) {
        std::cout << "could not load certificate here\n";
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    if (SSL_CTX_use_PrivateKey_file(m_Ctx, m_ServerConfig.getPriateKey().c_str(), SSL_FILETYPE_PEM) <= 0 ) {
        std::cout << "could not load private key\n";
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }
}

SSL* TLS::
SecureServer::createSSLConnection(SOCKET aAcceptedClient) {
    //NOTE: Be carefull returning ssl pointer
    SSL* lSsl = SSL_new(m_Ctx);
    SSL_set_fd(lSsl, aAcceptedClient);
    std::cout << __LINE__ << std::endl;
    if (SSL_accept(lSsl) <= 0) {
        ERR_print_errors_fp(stderr);
        return nullptr;
    }
    return lSsl;
}

bool TLS::
SecureServer::receiveMessage(SSL* aSsl, SOCKET aAcceptedClient) {
    char inBuffer[1024];
    int lReceiveMessageSize = SSL_read(aSsl, inBuffer, 1024);
    int lReadErrorCode = SSL_get_error(aSsl, lReceiveMessageSize);
    if (SSL_ERROR_NONE == lReadErrorCode) {
        std::cout << lReceiveMessageSize << " bytes has been read" << std::endl;
        return true;
    } else if (SSL_ERROR_WANT_READ == lReadErrorCode) {
        std::cout << "need to read rest data " << std::endl;
        return true;
    }
    return false;
}

//TLS::SecureServer::SOCKET TLS::
//SecureServer::getConnectionId() {
//    return acceptedSocket;//todo
//}

TLS::SecureServer::SOCKET TLS::SecureServer::accpetExternal() {
    std::cout << __LINE__ << std::endl;
    struct sockaddr_in addr;
    unsigned int lSize = sizeof(addr);
    std::cout << __LINE__ << "socker fd " << m_SocketFD << std::endl;
    SOCKET acceptedClient = accept(m_SocketFD, (sockaddr*)&addr, &lSize);

    if (acceptedClient == -1) {
        perror("Unable to accept");
        exit(EXIT_FAILURE);
    }
    SSL* lSsl = createSSLConnection(acceptedClient);
    if (lSsl == nullptr) {
        std::cout << "could not create secure connection " << std::endl;
        return;
    }
    //TODO lock this part we should track accpeted client to delete properly in the end
    //ServerClientConnection[acceptedClient] = std::make_pair(connectionThread, lSsl)
    return acceptedClient;
//  std::thread connectionThread(&startListen, this, lSsl, acceptedClient);
}

void TLS::SecureServer::acceptClient() {
    while(true) {
        std::cout << __LINE__ << std::endl;
        struct sockaddr_in addr;
        std::cout << __LINE__ << "socker fd " << m_SocketFD << std::endl;
        unsigned int lSize = sizeof(addr);
        SOCKET acceptedClient = accept(m_SocketFD, (sockaddr*)&addr, &lSize);

        if (acceptedClient == -1) {
            perror("Unable to accept");
            exit(EXIT_FAILURE);
        }
        SSL* lSsl = createSSLConnection(acceptedClient);
        if (lSsl == nullptr) {
            std::cout << "could not create secure connection " << std::endl;
            return;
        }
        std::thread connectionThread(&receiveMessage, this, lSsl, acceptedClient);
        //TODO receiveMessage is wrong in this case
        //std::thread connectionThread(&startListen, this, lSsl, acceptedClient);
        //TODO lock this part
        ServerClientConnection[acceptedClient] = std::make_pair(connectionThread, lSsl);
    }
    std::cout << __LINE__ << std::endl;
}

TLS::SecureServer::~SecureServer() {
    //TODO close all connections properly
//    SSL_shutdown(m_Ssl);
//    SSL_free(m_Ssl);
    close(m_SocketFD);
    SSL_CTX_free(m_Ctx);
}
