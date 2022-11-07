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

#include <map>
#include <thread>

#include "ServerConfig.h"

namespace TLS {
    class SecureServer {
        public:
            using SOCKET = int;
        public:
            SecureServer();
            SecureServer(const ServerConfig &aServerConfig, const SSL_METHOD *aTlsVersion);
            SecureServer(const ServerConfig &aServerConfig);

            int createSocket();
            void acceptClient();
            void initContext();
            SSL* createSSLConnection(SOCKET aAcceptedCliet);
            void startListen();
            bool receiveMessage(SSL* aSsl, SOCKET aAcceptedCliet);

            SOCKET accpetExternal();
            char* listenExternal();
            //SOCKET getConnectionId();
            virtual ~SecureServer() noexcept;

        private:
            SOCKET m_SocketFD;
            ServerConfig m_ServerConfig;
            std::map<SOCKET, std::pair<std::thread, SSL*>> ServerClientConnection;
            SSL_CTX* m_Ctx;
    };
}

#endif
