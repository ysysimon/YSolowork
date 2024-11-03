#include "YLineServer_CSPMid.h"
#include "spdlog/spdlog.h"
#include "utils/api.h"

using namespace YLineServer;

void CSPMiddleware::invoke(const HttpRequestPtr &req,
                          MiddlewareNextCallback &&nextCb,
                          MiddlewareCallback &&mcb)
{
    nextCb
    (
        [mcb = std::move(mcb), req](const HttpResponsePtr &resp)
        {
            // 设置 Content-Security-Policy
            resp->addHeader(
                "Content-Security-Policy", 
                "connect-src ws:" 
            );
            // Api::addCORSHeader(resp, req);
            //resp->addHeader("Access-Control-Allow-Methods", "GET,POST,PUT,DELETE,OPTIONS");
            resp->addHeader("Access-Control-Allow-Headers", "content-type");
            //resp->addHeader("Access-Control-Allow-Credentials", "true");
            spdlog::debug("CSPMiddleware invoked");
            mcb(resp);
        }
    );

}