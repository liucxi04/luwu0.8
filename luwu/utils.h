//
// Created by liucxi on 2022/4/8.
//

#ifndef LUWU_UTILS_H
#define LUWU_UTILS_H

#include <sys/types.h>
#include <string>
#include <vector>

namespace liucxi {

    pid_t getThreadId();

    u_int64_t getFiberId();

    u_int64_t getElapseMS();

    std::string getThreadName();

    void setThreadName(const std::string &name);

    void Backtrace(std::vector<std::string> &bt, int size = 64, int skip = 1);

    std::string BacktraceToString(int size = 64, int skip = 1, const std::string &prefix="");
}


#endif //LUWU_UTILS_H
