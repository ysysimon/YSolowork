#include "AMQP/TrantorHandler.h"
#include "amqpcpp/connection.h"
#include "spdlog/spdlog.h"
#include <memory>

namespace YLineServer{

void
TrantorHandler::reConnectTcpClient
(
    const std::weak_ptr<TrantorHandler> weakPtr,
    const std::string & username,
    const std::string & password,
    const std::string & host,
    const uint16_t port
)
{
    // 销毁旧客户端
    m_tcpClient.reset();
    // 再次设置 m_tcpClient 并设置相应回调
    m_loop->runAfter
    (
        1.0,
        [weakPtr, username, password, host, port, name = m_name]()
        {
            if(auto sharedPtr = weakPtr.lock())
            {
                spdlog::warn("{} Reconnecting TCP 重新连接 TCP", name);
                sharedPtr->setupTcpClient(sharedPtr->m_loop, host, port, username, password);
                // 执行重连
                sharedPtr->connect();
            }
            else 
            {
                spdlog::debug("Weak pointer `{}` is expired 弱指针已过期", name);
            }
        }
    );
}

void
TrantorHandler::setupTcpClient
(
    trantor::EventLoop * loop, 
    const std::string & host, 
    const uint16_t port,
    const std::string & username,
    const std::string & password
)
{
    if (!loop)
    {
        spdlog::error("{} Event loop is not available 事件循环不可用", m_name);
        return;
    }

    // 创建 TCP 客户端
    m_tcpClient = std::make_shared<trantor::TcpClient>
    (
        loop,
        trantor::InetAddress(host, port),
        m_name
    );

    // 设置当前对象弱引用
    auto weakPtr = std::weak_ptr<TrantorHandler>(shared_from_this());

    // 设置错误回调
    m_tcpClient->setConnectionErrorCallback
    (
        [weakPtr, username, password, host, port, name = m_name]()
        {
            // 获取当前对象的强引用
            if(auto sharedPtr = weakPtr.lock())
            {
                spdlog::error("{} connection failed - TCP connection error 连接失败 - TCP 连接错误", name);

                // 重新连接
                sharedPtr->reConnectTcpClient(weakPtr, username, password, host, port);
            }
        }
    );

    // 设置连接回调
    m_tcpClient->setConnectionCallback
    (
        [weakPtr, username, password, host, port, name = m_name](const trantor::TcpConnectionPtr &conn) 
        {
            if(auto sharedPtr = weakPtr.lock())
            {
                if (conn && conn->connected()) 
                {
                    sharedPtr->setConnection(conn, username, password);
                    spdlog::debug("{} connection established TCP 连接已建立", name);
                } 
                else 
                {
                    spdlog::error("{} set connection callback failed - TCP connection is not available 设置连接回调失败 - TCP 连接不可用", name);

                    // 重新连接
                    sharedPtr->reConnectTcpClient(weakPtr, username, password, host, port);
                }
            }
            else
            {
                spdlog::debug("Weak pointer `{}` is expired 弱指针已过期", name);
            }
        }
    );
}

void
TrantorHandler::onData(AMQP::Connection *connection, const char *data, size_t size)
{
    if (m_tcpClient && m_tcpClient->connection()) 
    {
        spdlog::debug("{} Sending data to TCP connection 发送数据到 TCP 连接", m_name);
        m_tcpClient->connection()->send(data, size); // 发送数据到 TCP 连接
    } 
    // else 
    // {
    //     spdlog::error("`{}` TCP connection is not available TCP 连接不可用", m_name);
    // }
}

void 
TrantorHandler::onReady(AMQP::Connection *connection)
{
    // the input connection is _amqpConnection
    spdlog::info("{} connection is ready 连接已准备就绪", m_name);
}

void
TrantorHandler::setConnection(
    const trantor::TcpConnectionPtr& conn,
    const std::string &username,
    const std::string &password
)
{
    if (conn && conn->connected()) 
    {
        spdlog::debug("Initializing {} AMQP connection 初始化 AMQP 连接 ...", m_name);
        // pass `this` to AMQP::Connection, when it's ready, it will call onReady()
        _amqpConnection = std::make_unique<AMQP::Connection>(
            this, 
            AMQP::Login( username, password),
            "/"
        );

        // 设置接收数据的回调
        conn->setRecvMsgCallback
        (
            [this](const trantor::TcpConnectionPtr& conn, trantor::MsgBuffer* buffer) 
            {
                if (_amqpConnection) 
                {
                    // 将数据传递给 AMQP::Connection 进行解析
                    _amqpConnection->parse(buffer->peek(), buffer->readableBytes());
                    buffer->retrieveAll(); // 清空缓冲区
                }
            }
        );
    }
    else 
    {
        spdlog::error("Initializing {} AMQP connection failed - TCP connection is not available 连接失败 - TCP 连接不可用", m_name);
    }
}

} // namespace YLineServer