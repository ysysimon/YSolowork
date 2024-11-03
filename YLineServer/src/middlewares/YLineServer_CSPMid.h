#pragma once

#include <drogon/HttpMiddleware.h>

namespace YLineServer
{

using namespace drogon;

class CSPMiddleware : public HttpMiddleware<CSPMiddleware> {
public:
    // 覆盖 invoke 方法
    void invoke(const HttpRequestPtr &req,
                          MiddlewareNextCallback &&nextCb,
                          MiddlewareCallback &&mcb) override;
};

}

