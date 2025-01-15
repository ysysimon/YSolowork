#ifndef YLineServer_CONSUMER_H
#define YLineServer_CONSUMER_H

#include "utils/config.h"

namespace YLineServer::task
{

void // 初始化消费者线程
initConsumerLoop(const Config & config);

void // 检测并恢复无效的消费者通道
recoverConsumerChannel();

}

#endif // YLineServer_CONSUMER_H