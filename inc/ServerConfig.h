#ifndef SERVERCONFIG_H
#define SERVERCONFIG_H

#include <string>

namespace TLS {
    class ServerConfig {
        public:
            ServerConfig();
            //TODO add note into documentation that default port is 8080 and default host is "0.0.0.0"            
            ServerConfig(const std::string& aCertificate,
                        const std::string& aPrivateKey,
                        const std::string& aHost = "localhost",
                        unsigned int aPort = 8080) noexcept;
            
            //getters only
            std::string getCertificate() const noexcept;
            std::string getPriateKey() const noexcept;
            std::string getHost() const noexcept;
            unsigned int getPort() const noexcept;
        
        private:
            std::string m_Certificate;
            std::string m_PrivateKey;
            std::string m_Host;
            unsigned int m_Port;
    };
}

#endif
