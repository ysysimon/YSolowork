#ifndef YLINESERVER_AMQP_CONNECTION_POOL_H
#define YLINESERVER_AMQP_CONNECTION_POOL_H

#include <memory>
#include <string>
#include <vector>
#include <amqpcpp/include/amqpcpp.h>
#include <drogon/HttpAppFramework.h>
#include "amqp/TrantorHandler.h"
#include "amqpcpp/channel.h"

namespace YLineServer
{

class AMQPConnectionPool
{
public:
    explicit
    AMQPConnectionPool(
        const std::string &host,
        const uint16_t port,
        const std::string &username,
        const std::string &password
    );

    ~AMQPConnectionPool() = default;
    
    inline const std::string&
    host() const { return m_host; }

    inline const uint16_t
    port() const { return m_port; }

    std::optional<AMQP::Channel>
    make_channel();

private:
    std::vector<std::shared_ptr<TrantorHandler>> m_AMQPHandler;
    std::string m_host;
    uint16_t m_port;
};

} // namespace YLineServer

#endif // YLINESERVER_AMQP_CONNECTION_POOL_H