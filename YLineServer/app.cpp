#include "app.h"
#include "drogon/IntranetIpFilter.h"
#include "drogon/LocalHostFilter.h"
#include "logger.h"
#include "database.h"
#include "middlewares/YLineServer_CORSMid.h"

#include <spdlog/logger.h>
#include <spdlog/spdlog.h>
#include <drogon/drogon.h>
#include <trantor/utils/Logger.h>


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

    drogon::app().addListener(config.server_ip, config.server_port);

    // 设置自定义 404 页面
    auto resp404 = drogon::HttpResponse::newNotFoundResponse();
    resp404->setStatusCode(drogon::k404NotFound);
    resp404->setContentTypeCode(drogon::CT_TEXT_HTML);
    resp404->setBody("404 Not Found, YLineServer 不存在该端点");
    drogon::app().setCustom404Page(resp404);

    // 启用 Brotli 和 Gzip 压缩
    drogon::app().enableBrotli(true).enableGzip(true);

    // 添加 postgresql 数据库客户端
    drogon::app().addDbClient(YLineServer::DB::getDrogonPostgresConfig(config));

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
    // 运行跨域请求
    if (config.cors)
    {
        // drogon::app().registerMiddleware(std::make_shared<YLineServer::CORSMiddleware>(config.allowed_origins));
    }

    spdlog::info("Start Listening 开始监听");
    drogon::app().run();

}

}