//
// Created by liucxi on 2022/4/8.
//

#include "utils.h"
#include <unistd.h>
#include <sys/syscall.h>

namespace liucxi {
    pid_t getThreadId() {
        return syscall(SYS_gettid);
    }

    u_int64_t getFiberId() {
        return 0;
    }

    uint64_t getElapseMS() {
        struct timespec ts{0};
        clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
        return ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
    }

    std::string getThreadName() {
        char threadName[16] = {0};
        pthread_getname_np(pthread_self(), threadName, 16);
        return std::string(threadName);
    }

    void setThreadName(const std::string &name) {
        pthread_setname_np(pthread_self(), name.substr(0, 15).c_str());
    }
}
