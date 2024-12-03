#pragma once

#include <drogon/HttpController.h>
#include "drogon/utils/coroutine.h"

using namespace drogon;

namespace YLineServer
{

struct task
{
    int id;
    std::string name;
    std::string belongJob;
};

struct job
{
    std::string name;
};

class WorkCtrl : public drogon::HttpController<WorkCtrl>
{
  public:
    METHOD_LIST_BEGIN
    // use METHOD_ADD to add your custom processing function here;
    ADD_METHOD_TO(WorkCtrl::submitNonDependentJob, "/api/work/submitNonDependentJob", Post);
    ADD_METHOD_TO(WorkCtrl::submitDependentJob, "/api/work/submitDependentJob", Post);

    METHOD_LIST_END
    // your declaration of processing function maybe like this:
    drogon::Task<void> submitNonDependentJob(const HttpRequestPtr req, std::function<void(const HttpResponsePtr &)> callback);
    drogon::Task<void> submitDependentJob(const HttpRequestPtr req, std::function<void(const HttpResponsePtr &)> callback);

  private:
    bool 
    validJobJson(const Json::Value &json, std::string &err);

    bool
    resolveJob(const Json::Value &json, std::string &err, bool dependency = false);
};
}
