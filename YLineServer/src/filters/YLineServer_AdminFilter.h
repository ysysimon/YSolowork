/**
 *
 *  YLineServer_AdminFilter.h
 *
 */

#pragma once

#include <drogon/HttpFilter.h>
using namespace drogon;
namespace YLineServer
{

class AdminFilter : public HttpFilter<AdminFilter>
{
  public:
    AdminFilter() {}
    void doFilter(const HttpRequestPtr &req,
                  FilterCallback &&fcb,
                  FilterChainCallback &&fccb) override;
};

}
