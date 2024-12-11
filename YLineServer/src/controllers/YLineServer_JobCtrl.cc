#include "YLineServer_JobCtrl.h"

using namespace YLineServer;

drogon::Task<void> 
JobCtrl::queueJob(const HttpRequestPtr req, std::function<void(const HttpResponsePtr &)> callback)
{
    
}