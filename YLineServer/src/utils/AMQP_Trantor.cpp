#include "utils/AMQP_Trantor.h"

namespace YLineServer{

// 处理事件
void 
TrantorHandler::handleEvent(const std::shared_ptr<trantor::Channel> &channel, int events)
{
    int fd = channel->fd();  // 使用 Channel 提供的 fd() 方法获取文件描述符
    auto it = _connections.find(fd);
    if (it == _connections.end()) 
    {
        spdlog::warn("AMQP - Event received for unknown fd 未知的文件描述符: {}", fd);
    }

    auto *connection = it->second;

    // 通知 AMQP 连接处理事件
    connection->process(fd, events);
}

// 注册或更新文件描述符上的事件
void 
TrantorHandler::monitor(AMQP::TcpConnection *connection, int fd, int flags)
{
    auto it = _channels.find(fd);

    if (flags == 0) 
    {
        // 如果 flags 为 0，则移除事件监听
        if (it != _channels.end()) {
            it->second->disableAll();
            it->second->remove();
            _channels.erase(it);
            spdlog::debug("AMQP - Stopped monitoring fd 停止监控文件描述符: {}", fd);
        }
        return;
    }

    if (it == _channels.end()) 
    {
        // 如果是新文件描述符，则注册事件
        spdlog::debug("AMQP - Monitoring new fd 监控新文件描述符: {}", fd);
        auto channel = std::make_shared<trantor::Channel>(_loop, fd);
        channel->setReadCallback([this, channel]() { handleEvent(channel, AMQP::readable); });
        channel->setWriteCallback([this, channel]() { handleEvent(channel, AMQP::writable); });
        channel->enableReading();
        channel->enableWriting();

        _channels[fd] = channel;
    } 
    else 
    {
        // 如果是已存在的文件描述符，则更新事件
        spdlog::debug("AMQP - Updating monitoring for fd 更新文件描述符监控: {}", fd);
        auto channel = it->second;
        if (flags & AMQP::readable) {
            channel->enableReading();
        } else {
            channel->disableReading();
        }
        if (flags & AMQP::writable) {
            channel->enableWriting();
        } else {
            channel->disableWriting();
        }
    }

    // 将文件描述符与 AMQP::TcpConnection 关联
    _connections[fd] = connection;
}

} // namespace YLineServer