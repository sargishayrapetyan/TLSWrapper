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
#include <mutex>

#include "ServerConfig.h"
#include "ConnectionThread.h"

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
            bool createSSLConnection(SOCKET aAcceptedCliet);
            void startListen();
            int readBytes(SSL *aSsl);
            bool receiveMessage(SOCKET aAcceptedCliet);
            void closeConnection(SSL* aSsl, SOCKET aClient);
            SOCKET accpetExternal();
            char* listenExternal();
            //SOCKET getConnectionId();
            virtual ~SecureServer() noexcept;

        private:
            SOCKET m_SocketFD;
            ServerConfig m_ServerConfig;
            std::map<SOCKET, std::thread> m_ClientConnectionThreads;
            std::map<SOCKET, SSL*> m_ClientSSLConnections;
            SSL_CTX *m_Ctx;
            std::mutex mtx;
    };
}

#endif
