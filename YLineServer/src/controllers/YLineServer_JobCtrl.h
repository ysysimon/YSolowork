#pragma once

#include <drogon/HttpController.h>
#include "drogon/utils/coroutine.h"


using namespace drogon;

namespace YLineServer
{
class JobCtrl : public drogon::HttpController<JobCtrl>
{
  public:
    METHOD_LIST_BEGIN
    // use METHOD_ADD to add your custom processing function here;
    ADD_METHOD_TO(
      JobCtrl::queueJob,
      "/api/job/queue",
      Post,
      "YLineServer::LoginFilter"
    );

    METHOD_LIST_END
  private:

  drogon::Task<void> 
  queueJob(const HttpRequestPtr req, std::function<void(const HttpResponsePtr &)> callback);
  
};
}
