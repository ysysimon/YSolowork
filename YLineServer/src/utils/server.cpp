#include "utils/server.h"

namespace YLineServer {

EnTTidType ServerSingleton::addUserEntt(const Users::PrimaryKeyType& userId, const std::string& username, bool isAdmin)
{
    // 创建用户实体
    EnTTidType userEntt = Registry.create();
    // 添加用户组件
    Registry.emplace<Components::User>(userEntt, userId, username, isAdmin, std::vector<WebSocketConnectionPtr>());
    // 添加用户名到用户实体映射
    usernameToEnTTid[username] = userEntt;
    // 返回用户实体 ID
    return userEntt;
}

} // namespace YLineServer