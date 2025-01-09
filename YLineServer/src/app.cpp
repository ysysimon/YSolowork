#include "app.h"

#include "AMQP/AMQPconnectionPool.h"
#include "drogon/HttpAppFramework.h"
#include "drogon/IntranetIpFilter.h"
#include "drogon/LocalHostFilter.h"
#include "utils/logger.h"
#include "database.h"
#include "middlewares/YLineServer_CORSMid.h"
#include "utils/api.h"

#include <memory>
#include <spdlog/logger.h>
#include <spdlog/spdlog.h>
#include <drogon/drogon.h>
#include <string>
#include <trantor/utils/Logger.h>

#include "utils/server.h"

namespace YLineServer {

void spawnApp(const Config& config, const std::shared_ptr<spdlog::logger> custom_logger)
{
    // 检查是否支持 spdlog
    // 注意: 需要使用 开启了 spdlog 支持的 Drogon 构建
    if (!trantor::Logger::hasSpdLogSupport()) {
        spdlog::error("Current Drogon build does not support spdlog. 当前 Drogon 构建不支持 spdlog.");
        // throw exception
        // throw std::runtime_error("Current Drogon build does not support spdlog. 当前 Drogon 构建不支持 spdlog.");
    }

    // set drogon logger
    trantor::Logger::enableSpdLog(custom_logger);
    trantor::Logger::setLogLevel(YLineServer::spdlogToDrogonLogLevel.at(config.log_level));

    // 设置工作线程数量 默认为 4
    drogon::app().setThreadNum(config.server_thread_num);

    drogon::app().addListener(config.server_ip, config.server_port);

    // 设置自定义 404 页面
    // auto resp404 = drogon::HttpResponse::newNotFoundResponse();
    // resp404->setStatusCode(drogon::k404NotFound);
    // resp404->setContentTypeCode(drogon::CT_TEXT_HTML);
    // resp404->setBody("404 Not Found, YLineServer 不存在该端点");
    auto resp404 = YLineServer::Api::makeHttpResponse(
        "404 Not Found, YLineServer 不存在该端点", 
        drogon::HttpStatusCode::k404NotFound, 
        nullptr, 
        drogon::CT_TEXT_HTML);
    drogon::app().setCustom404Page(resp404);

    // 启用 Brotli 和 Gzip 压缩
    drogon::app().enableBrotli(true).enableGzip(true);

    // 添加 postgresql 数据库客户端
    drogon::app().addDbClient(YLineServer::DB::getDrogonPostgresConfig(config));

    // 添加 redis 数据库客户端
    drogon::app().createRedisClient(
        config.redis_host, 
        config.redis_port, 
        "YLineRedis", 
        config.redis_password, 
        config.redis_connection_number, 
        true, 
        config.redis_timeout, 
        config.redis_index
    );

    // 内置 中间件 middleware
    // 仅允许内网 IP 访问
    if (config.intranet_ip_filter)
    {
        // drogon::app().registerFilter<IntranetIpFilter>(); ;
        spdlog::info("Intranet IP filter enabled 内网 IP 过滤器已启用");
    }
    // 仅允许本机访问
    if (config.local_host_filter)
    {
        // drogon::app().registerMiddleware(std::make_shared<drogon::LocalHostFilter>());
        spdlog::info("Local host filter enabled 本地主机过滤器已启用");
    }
    // 允许跨域请求
    if (config.cors)
    {
        auto corsMiddleware = std::make_shared<YLineServer::CORSMiddleware>(config.allowed_origins);
        drogon::app().registerMiddleware(corsMiddleware);
        // 将协程中间件注册为 Pre-Routing Advice
        // 注册 Pre-Routing Advice
        drogon::app().registerPreRoutingAdvice([corsMiddleware](const HttpRequestPtr &req, AdviceCallback &&callback, AdviceChainCallback &&chainCallback) {
            // 调用中间件的 invoke 方法
            corsMiddleware->invoke(req,
                // nextCb：继续执行下一个中间件或控制器的回调
            [chainCallback = std::move(chainCallback)](const std::function<void(const HttpResponsePtr &)>& nextResponseCb) {
                        // 执行下一个 Pre-Routing Advice 或进入路由处理
                        chainCallback();
                    },
                // mcb：终止请求并返回响应的回调
                [callback = std::move(callback)](const HttpResponsePtr& resp) {
                    // 返回响应
                    callback(resp);
                }
            );
        });

        spdlog::info("CORS middleware enabled 跨域请求中间件已启用");
    }

    // AMQP 连接池
    auto & amqpConnectionPool = YLineServer::ServerSingleton::getInstance().amqpConnectionPool;
    app().getLoop()->queueInLoop
    (
        // 注意需要按引用捕获 amqpConnectionPool，因为上面的 shared_ptr 还未初始化，引用计数并未生效
        [ &config, &amqpConnectionPool]() mutable
        {
            // 创建 AMQP 连接池
            amqpConnectionPool = std::make_shared<YLineServer::AMQPConnectionPool>
            (
                config.amqp_host,
                config.amqp_port,
                config.amqp_user,
                config.amqp_password
            );

            if (!amqpConnectionPool)
            {
                throw std::runtime_error("AMQP Connection Pool creation failed AMQP 连接池创建失败");
            }

            spdlog::info("AMQP Connection Pool created AMQP 连接池已创建");

            // 声明已经创建的 AMQP 通道
            const auto PgClient = YLineServer::DB::getTempPgClient
            (
                config.db_host, config.db_port, config.db_name, config.db_user, config.db_password
            );

            // TODO: 从数据库中读取已经创建的 AMQP 队列

            // 声明默认队列
            auto channel = amqpConnectionPool->make_channel();
            while (!channel)
            {
                spdlog::warn("Failed to create default Queue, because of AMQP Channel creation failed 无法创建默认队列，因为 AMQP 通道创建失败");
                spdlog::warn("Retrying in 1 seconds 1 秒后重试");
                std::this_thread::sleep_for(std::chrono::seconds(1));
                channel = amqpConnectionPool->make_channel();
            }

            AMQP::Table arguments;
            arguments["x-max-priority"] = 100; // 设置最大优先级

            channel->declareQueue(Queue::default_queue, AMQP::durable, arguments)
                .onSuccess
                (
                    [](const std::string &name, uint32_t messageCount, uint32_t consumerCount)
                    {
                        spdlog::info("Queue `{}` declared Success, 默认队列声明成功", name);
                    }
                )
                .onError
                (
                    [](const char *message)
                    {
                        throw std::runtime_error("Queue `default` declare failed, 默认队列声明失败: " + std::string(message));
                    }
                );
                
        }
    );

    // 启动事件循环
    spdlog::info("Start Listening 开始监听");
    drogon::app().run();
}

}