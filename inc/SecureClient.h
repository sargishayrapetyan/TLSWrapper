#ifndef SECURECLIENT_H
#define SECURECLIENT_H

//System includes

#include "ServerConfig.h"

#include <string>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <openssl/ssl.h>
#include <openssl/crypto.h>
#include <openssl/err.h>

//using namespace TLS;

namespace TLS {
        class SecureClient {

            public:
                SecureClient(const ServerConfig& aServerConfig) noexcept;

                SecureClient(const ServerConfig& aServerConfig, const SSL_METHOD* aTlsVersion) noexcept;

                virtual ~SecureClient() noexcept;

            public:
                void send() noexcept;

            private:
                void prepareConnect() noexcept;
                bool sendPayload(char* aDeviceTokenBinary, char* aPayloadBuff, size_t aPayloadLength) noexcept;

            private:
                SSL* m_Ssl;
                SSL_CTX* m_Ctx;
                ServerConfig m_ServerConfig;
                int m_SocketFD;
        };
    }
#endif
