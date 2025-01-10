#ifndef YLineServer_CONSUMER_H
#define YLineServer_CONSUMER_H

#include "utils/config.h"

namespace YLineServer::task
{
// 初始化消费者线程
void initConsumerLoop(const Config & config);

}

#endif // YLineServer_CONSUMER_H