/**
 *
 *  YLineServer_LoginFilter.h
 *
 */

#pragma once

#include <drogon/HttpFilter.h>
#include "utils/jwt.h"

using namespace drogon;
namespace YLineServer
{

class LoginFilter : public HttpFilter<LoginFilter>
{
  public:
    LoginFilter() {}
    void doFilter(const HttpRequestPtr &req,
                  FilterCallback &&fcb,
                  FilterChainCallback &&fccb) override;
  private:
    bool validateToken(const std::string &authHeader, Json::Value& payload);
};

}
