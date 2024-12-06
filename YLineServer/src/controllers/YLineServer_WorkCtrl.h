#pragma once

#include <cstddef>
#include <drogon/HttpController.h>
#include <string>
#include "drogon/utils/coroutine.h"

using namespace drogon;

namespace YLineServer
{

namespace Components {
struct Task
{
    std::string task_id;
    std::size_t order;
    std::string name;
    std::string belongJob_id;
    bool dependency;
    bool complete;
};

struct Job
{
    std::string job_id;
    std::string name;
    std::string submit_user;
    std::string submit_time;
};
}



class WorkCtrl : public drogon::HttpController<WorkCtrl>
{
  public:
    METHOD_LIST_BEGIN
    // use METHOD_ADD to add your custom processing function here;
    ADD_METHOD_TO(
      WorkCtrl::submitNonDependentJob, 
      "/api/work/submitNonDependentJob", 
      Post,
      "YLineServer::LoginFilter"
    );
    ADD_METHOD_TO(
      WorkCtrl::submitDependentJob, 
      "/api/work/submitDependentJob", 
      Post,
      "YLineServer::LoginFilter"
    );

    METHOD_LIST_END
    // your declaration of processing function maybe like this:
    drogon::Task<void> submitNonDependentJob(const HttpRequestPtr req, std::function<void(const HttpResponsePtr &)> callback);
    drogon::Task<void> submitDependentJob(const HttpRequestPtr req, std::function<void(const HttpResponsePtr &)> callback);

  private:
    bool 
    validJobJson(const Json::Value &json, std::string &err);

    bool
    resolveJob(const Json::Value &json, std::string &err, std::vector<Components::Task> &task_Components, bool dependency = false);
};
}
