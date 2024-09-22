#ifndef YLINESERVER_DATABASE_H
#define YLINESERVER_DATABASE_H

#include <string>

#include "config.h"

namespace YLineServer {

std::string getPostgresConnectionString(const Config& config);

}

#endif // YLINESERVER_DATABASE_H