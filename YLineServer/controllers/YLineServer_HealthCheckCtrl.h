#pragma once

#include <drogon/HttpSimpleController.h>

using namespace drogon;

namespace YLineServer
{
class HealthCheckCtrl : public drogon::HttpSimpleController<HealthCheckCtrl>
{
  public:
    void asyncHandleHttpRequest(const HttpRequestPtr& req, std::function<void (const HttpResponsePtr &)> &&callback) override;
    PATH_LIST_BEGIN
    // list path definitions here;
    // PATH_ADD("/path", "filter1", "filter2", HttpMethod1, HttpMethod2...);

    PATH_ADD("/api", Get, Post);
    PATH_ADD("/alive", Get, Post);

    PATH_LIST_END
};
}
