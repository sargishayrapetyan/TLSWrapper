#include "SecureClient.h"

// TODO setup logger for project
#include <arpa/inet.h>
#include <sstream>
#include <iostream>

TLS::
SecureClient::SecureClient(const ServerConfig& aServerConfig) noexcept
    : m_ServerConfig(aServerConfig)
    , m_Ssl(nullptr)
    , m_Ctx(SSL_CTX_new(TLSv1_2_method()))//create another constructor to use other tls method oveload
{
    prepareConnect();
}

TLS::
SecureClient::SecureClient(const ServerConfig& aServerConfig, const SSL_METHOD* aTlsVersion) noexcept
    : m_ServerConfig(aServerConfig)
    , m_Ssl(nullptr)
    , m_Ctx(SSL_CTX_new(aTlsVersion)) //create another constructor to use other tls method oveload
{
    prepareConnect();
}

void TLS::
SecureClient::send(char* aPayload, size_t aPayloadSize) noexcept
{
    //TODO
}

void TLS::
SecureClient::send(const std::string& aPayload) noexcept
{
    //TODO
}


void TLS::
SecureClient::prepareConnect() noexcept
{
    struct hostent *lHostent;
    struct sockaddr_in lSockAddrIn;
    
    SSLeay_add_ssl_algorithms();
    SSL_load_error_strings();
    if (! m_Ctx) {
        // LOG_ERROR("FAILED TO CREATE SSL COTEXT! SSL_CTX_new()...faild");
        return;
    }   
    
    if (SSL_CTX_load_verify_locations(m_Ctx, NULL, ".") <= 0) {
        ERR_print_errors_fp(stderr);
        return;
    }   
    
    //TODO certificate
    if (SSL_CTX_use_certificate_file(m_Ctx, m_ServerConfig.getCertificate().c_str(),
                                     SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        return;
    }   
    
    //TODO PRIVATE file separete them maybe not need
    if (SSL_CTX_use_PrivateKey_file(m_Ctx, m_ServerConfig.getCertificate().c_str(), SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        return;
    }   
    
    if (SSL_CTX_check_private_key(m_Ctx) <= 0) {
        ERR_print_errors_fp(stderr);
        return;
    }   
    
    m_SocketFD = socket(AF_INET, SOCK_STREAM, 0); 
    if (m_SocketFD == -1) {
        // LOG_ERROR("FAILED TO CREATE SOCKET! socket()...-1");
        return;
    }   
    
    lSockAddrIn.sin_family = AF_INET;
    lHostent = gethostbyname(m_ServerConfig.getHost().c_str());
    
    if (! lHostent) {
        // LOG_ERROR("FAILED TO GET HOST BY NAME!");
        close(m_SocketFD);
        return;
    }   
    lSockAddrIn.sin_addr.s_addr = inet_addr(inet_ntoa(*((struct in_addr *) lHostent->h_addr_list[0])));
    lSockAddrIn.sin_port = htons(m_ServerConfig.getPort());
    
    if (connect(m_SocketFD, (struct sockaddr *) &lSockAddrIn, sizeof(lSockAddrIn)) == -1) {
        // LOG_ERROR("FAILED TO CONNECT TO HOST!");
        close(m_SocketFD);
        return;
    }   

    m_Ssl = SSL_new(m_Ctx);
    SSL_set_connect_state(m_Ssl); //TODO function to set client mode , NOTE: SSL_set_accept_state for server mode
    SSL_set_fd(m_Ssl, m_SocketFD);
    int lErrorCode = SSL_connect(m_Ssl);

    if (lErrorCode != 1) { //TODO handshake with server
        std::cout << "failed to connect to server " << std::endl;
        shutdown(m_SocketFD, 2); 
        close(m_SocketFD);
        return;
    }
    std::cout << "connection success" << std::endl; //create enum for this purpose
}

bool TLS::
SecureClient::sendPayload(char* aPayload, size_t aPayloadSize) noexcept {
    //TODO
        int lResult = SSL_write(m_Ssl, aPayload, aPayloadSize);
        if (lResult > 0) {
            return true; //TODO return enum
        }
        //TODO this part maybe redundant because if there SSL_write failed that means do nothing
    //     else {
    //         int lErrorCode = SSL_get_error(m_Ssl.get(), lResult);
    //         // LOG_ERROR("Failed to write in SSL, error code:" + std::to_string(lErrorCode));
    //         size_t lFailureCount = 3;
    //         for (size_t i = 0; i < lFailureCount; ++i) {
    //             SSL_shutdown(m_Ssl);
    //             shutdown(m_SocketFD, 2);
    //             close(m_SocketFD);
    //             prepareConnect();
    //             if (SSL_write(m_Ssl, lBinaryMessageBuff, (lBinaryMessagePt - lBinaryMessageBuff)) > 0) {
    //                 return true;
    //             }
    //         }
    //         return false;
    //     }
    // }
    return false; 
}

 TLS::
SecureClient::~SecureClient() noexcept
{
    SSL_shutdown(m_Ssl);
    SSL_free(m_Ssl);
    close(m_SocketFD);
    SSL_CTX_free(m_Ctx);
}
