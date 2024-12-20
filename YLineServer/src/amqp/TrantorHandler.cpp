#include "amqp/TrantorHandler.h"
#include "amqpcpp/connection.h"

namespace YLineServer{

TrantorHandler::TrantorHandler(
    const std::string &name, 
    trantor::EventLoop *loop, 
    const std::string &host, 
    const uint16_t port,
    const std::string &username,
    const std::string &password
)
    : m_name(name)
{
    m_tcpClient = std::make_shared<trantor::TcpClient>(
        loop,
        trantor::InetAddress(host, port),
        m_name
    );

    // 设置回调
    m_tcpClient->setConnectionErrorCallback
    (
        [this]() 
        {
            spdlog::error("{} connection failed - TCP connection error 连接失败 - TCP 连接错误", m_name);
            auto loop = m_tcpClient->getLoop();
            loop->runAfter(1000, [this]() {
                m_tcpClient->connect();
            }); // reconnect after 1s, but seems not working
        }
    );

    m_tcpClient->setConnectionCallback
    (
        // need to copy username and password for lifetime
        [this, username, password](const trantor::TcpConnectionPtr &conn) 
        {
            if (conn && conn->connected()) 
            {
                setConnection(conn, username, password);
            } 
            else 
            {
                spdlog::error("{} connection failed - TCP connection is not available 连接失败 - TCP 连接不可用", m_name);
            }
        }
    );

    // 将 AMQP 服务器 TCP 连接设置为可重试，不知道这有什么意义，似乎没做什么
    m_tcpClient->enableRetry();

}

void
TrantorHandler::onData(AMQP::Connection *connection, const char *data, size_t size)
{
    if (m_tcpClient && m_tcpClient->connection()) 
    {
        spdlog::debug("{} Sending data to TCP connection 发送数据到 TCP 连接", m_name);
        m_tcpClient->connection()->send(data, size); // 发送数据到 TCP 连接
    } 
    else 
    {
        spdlog::error("{} TCP connection is not available TCP 连接不可用", m_name);
    }
}

void 
TrantorHandler::onReady(AMQP::Connection *connection)
{
    // the input connection is _amqpConnection
    spdlog::debug("{} connection is ready AMQP 连接已准备就绪", m_name);

    // 声明队列并消费消息
    // AMQP::Channel channel(connection);
    _channel = std::make_shared<AMQP::Channel>(connection);
    _channel->declareQueue("test-queue").onSuccess([this]() {
        spdlog::info("{} Queue declared successfully! 队列声明成功!", m_name);
    });
    _channel->consume("test-queue").onReceived
    (
        [this](const AMQP::Message &message, uint64_t deliveryTag, bool redelivered)
        {
            std::string messageBody(message.body(), message.bodySize());
            spdlog::info("{} Received message 接收到新的队列消息: {}", m_name, messageBody);
            _channel->ack(deliveryTag);
        }
    );
    _channel->publish("", "test-queue", "Hello, AMQP!");
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
        spdlog::debug("Initializing {} connection 初始化 AMQP 连接 ...", m_name);
        // pass `this` to AMQP::Connection, when it's ready, it will call onReady()
        _amqpConnection = std::make_shared<AMQP::Connection>(
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
        spdlog::error("{} connection failed - TCP connection is not available 连接失败 - TCP 连接不可用", m_name);
    }
}

} // namespace YLineServer