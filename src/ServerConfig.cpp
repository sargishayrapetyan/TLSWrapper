#include <ServerConfig.h>
#include <iostream>

TLS::ServerConfig::ServerConfig()
    : m_Certificate()
    , m_PrivateKey()
    , m_Host("0.0.0.0")
    , m_Port(8080)
{
    
}

TLS::ServerConfig::ServerConfig(const std::string& aCertificate, const std::string& aPrivateKey, const std::string& aHost, unsigned int aPort) noexcept
    : m_Certificate(aCertificate)
    , m_PrivateKey(aPrivateKey)
    , m_Host(aHost)
    , m_Port(aPort)
{
    if (aCertificate.empty()) {
        std::cout << "server certificate is empty" << std::endl;
    }

    if (aHost.empty()) {
        std::cout << "server host is empty" << std::endl;
    }
}

std::string
TLS::ServerConfig::getCertificate() const noexcept {
    return m_Certificate;
}

std::string
TLS::ServerConfig::getPriateKey() const noexcept {
    return m_PrivateKey;
}


std::string 
TLS::ServerConfig::getHost() const noexcept {
    return m_Host;
}

unsigned int 
TLS::ServerConfig::getPort() const noexcept {
    return m_Port;
}