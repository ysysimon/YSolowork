#include "database.h"
#include <string>

namespace YLineServer {

std::string getPostgresConnectionString(const Config& config) {
    std::string conn_str = "postgres://username:password@host:port/database";
    return conn_str;

}

}