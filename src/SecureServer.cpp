#include "SecureServer.h"
#include <iostream>

TLS::SecureServer::SecureServer()
    : m_Ssl(nullptr)
    , m_Ctx(SSL_CTX_new(TLS_server_method()))
{
    init_context();
    createSocket();
}

TLS::SecureServer::SecureServer(const ServerConfig& aServerConfig)
    : m_ServerConfig(aServerConfig)
    , m_Ssl(nullptr)
    , m_Ctx(SSL_CTX_new(TLS_server_method()))
{
    init_context();
    createSocket();
}

TLS::SecureServer::SecureServer(const ServerConfig& aServerConfig, const SSL_METHOD* aTlsVersion)   
    : m_ServerConfig(aServerConfig)
    , m_Ssl(nullptr)
    , m_Ctx(SSL_CTX_new((aTlsVersion)))
{
    init_context();
    createSocket();
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

void TLS::SecureServer::init_context() {
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


void TLS::SecureServer::listen_client() {
    while(1) {
        std::cout << __LINE__ << std::endl;
        struct sockaddr_in addr;
        socklen_t len = sizeof(addr);
std::cout << __LINE__ << "socker fd " << m_SocketFD << std::endl;
        int client = accept(m_SocketFD, (struct sockaddr*)&addr, &len); //NOTE: The client is important 
        if (client < 0) {
            perror("Unable to accept");
            exit(EXIT_FAILURE);
        }
std::cout << __LINE__ << std::endl;
        m_Ssl = SSL_new(m_Ctx);
        SSL_set_fd(m_Ssl, client);
std::cout << __LINE__ << std::endl;
        if (SSL_accept(m_Ssl) <= 0) {
            ERR_print_errors_fp(stderr);
        } else {
            SSL_write(m_Ssl, "aaaaa", 5);
        }
    }
    std::cout << __LINE__ << std::endl;
}

TLS::SecureServer::~SecureServer() {
    SSL_shutdown(m_Ssl);
    SSL_free(m_Ssl);
    close(m_SocketFD);
    SSL_CTX_free(m_Ctx);
}
