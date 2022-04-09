//
// Created by liucxi on 2022/4/8.
//

#ifndef LUWU_UTILS_H
#define LUWU_UTILS_H

#include <sys/types.h>
#include <string>

namespace liucxi {

    pid_t getThreadId();

    u_int64_t getFiberId();

    u_int64_t getElapseMS();

    std::string getThreadName();

    void setThreadName(const std::string &name);
}


#endif //LUWU_UTILS_H
