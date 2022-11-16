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

namespace TLS {
    class SecureServer {
        public:
            using SOCKET = int;

        public:
            SecureServer();
            SecureServer(const ServerConfig &aServerConfig, const SSL_METHOD *aTlsVersion);
            SecureServer(const ServerConfig &aServerConfig);
            virtual ~SecureServer() noexcept;

        private:
            void initialiseSSLContext() noexcept(false);
            int createSocketServer() noexcept(false);
            void initialiseServer();
            bool createSSLConnection(SOCKET aAcceptedCliet);
            int readBytes(SSL *aSsl);
            void closeConnection(SSL* aSsl, SOCKET aClient);
        
        public:
            bool receiveMessage(SOCKET aAcceptedCliet);    
            SOCKET accpetExternal();
            char* listenExternal();
            void acceptClient();        

        private:
            struct sockaddr_in m_Server;
            SOCKET m_SocketFD;
            SSL_CTX *m_Ctx;
            ServerConfig m_ServerConfig;
            std::map<SOCKET, std::thread> m_ClientConnectionThreads;
            std::map<SOCKET, SSL*> m_ClientSSLConnections;
            std::mutex mtx;
    };
}

#endif
