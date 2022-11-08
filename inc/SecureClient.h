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

#include <memory>

//using namespace TLS;

namespace TLS {
        class SecureClient {

            public:
                SecureClient();
                SecureClient(const ServerConfig& aServerConfig) noexcept;

                SecureClient(const ServerConfig& aServerConfig, const SSL_METHOD* aTlsVersion) noexcept;

                virtual ~SecureClient() noexcept;

            public:
                void send(char* aPayload, size_t aPayloadSize) noexcept;
                void send(const std::string& aPayload) noexcept;
                void receive();
                void closeConncetion();
                bool connectToServer();

            private:
                void prepareConnect() noexcept;
                bool sendPayload(const char* aPayload, size_t aPayloadSize) noexcept;

            private:
                int m_SocketFD;
                ServerConfig m_ServerConfig;
                SSL* m_Ssl;
                SSL_CTX* m_Ctx;
        };
    }
#endif
