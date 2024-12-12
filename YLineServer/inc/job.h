#ifndef YLineServer_job_H
#define YLineServer_job_H

namespace YLineServer
{

namespace Components {

struct Task
{
    std::string task_id;
    int order;
    std::string name;
    bool dependency;
};

struct Job
{
    std::string name;
    std::string submit_user;
};




}

} // namespace YLineServer

#endif // YLineServer_job_H