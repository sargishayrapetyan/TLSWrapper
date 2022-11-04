#ifndef SECURESERVER_H
#define SECURESERVER_H

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h> 
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#include "ServerConfig.h"

namespace TLS {
    class SecureServer {
        public:
            SecureServer();
            SecureServer(const ServerConfig &aServerConfig, const SSL_METHOD *aTlsVersion);
            SecureServer(const ServerConfig &aServerConfig);

            int createSocket();
            void init_context();
            void listen_client();
            virtual ~SecureServer() noexcept;

        private:
            int m_SocketFD;
            ServerConfig m_ServerConfig;
            SSL* m_Ssl;
            SSL_CTX* m_Ctx;
    };
}

#endif