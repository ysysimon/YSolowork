#include "utils/AMQP_Trantor.h"

namespace YLineServer{

void
TrantorHandler::onData(AMQP::Connection *connection, const char *data, size_t size)
{
    if (m_tcpClient && m_tcpClient->connection()) 
    {
        m_tcpClient->connection()->send(data, size); // 发送数据到 TCP 连接
    } 
    else 
    {
        spdlog::error("AMQP TCP connection is not available TCP 连接不可用");
    }
}

void 
TrantorHandler::onReady(AMQP::Connection *connection)
{
    // the input connection is _amqpConnection
    spdlog::debug("AMQP connection is ready AMQP 连接已准备就绪");

    // 声明队列并消费消息
    // AMQP::Channel channel(connection);
    _channel = std::make_shared<AMQP::Channel>(connection);
    _channel->declareQueue("test-queue").onSuccess([]() {
        spdlog::info("Queue declared successfully! 队列声明成功!");
    });
    _channel->consume("test-queue").onReceived
    (
        [this](const AMQP::Message &message, uint64_t deliveryTag, bool redelivered)
        {
            std::string messageBody(message.body(), message.bodySize());
            spdlog::info("Received message 接收到新的队列消息: {}", messageBody);
            _channel->ack(deliveryTag);
        }
    );
    _channel->publish("", "test-queue", "Hello, AMQP!");
}

void 
TrantorHandler::setConnection(const trantor::TcpConnectionPtr& conn)
{
    if (conn && conn->connected()) 
    {
        spdlog::debug("Initializing AMQP connection 初始化 AMQP 连接 ...");
        // pass `this` to AMQP::Connection, when it's ready, it will call onReady()
        _amqpConnection = std::make_shared<AMQP::Connection>(this, AMQP::Login("guest", "guest"), "/");
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

}

} // namespace YLineServer