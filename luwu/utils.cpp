//
// Created by liucxi on 2022/4/8.
//

#include "log.h"
#include "utils.h"
#include "fiber.h"
#include <unistd.h>
#include <execinfo.h>  // backtrace
#include <sys/syscall.h>

namespace liucxi {

    liucxi::Logger::ptr g_logger = LUWU_LOG_NAME("system");

    pid_t getThreadId() {
        return (pid_t)syscall(SYS_gettid);
    }

    u_int64_t getFiberId() {
        return Fiber::GetFiberId();
    }

    uint64_t getElapseMS() {
        struct timespec ts{0};
        clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
        return ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
    }

    std::string getThreadName() {
        char threadName[16] = {0};
        pthread_getname_np(pthread_self(), threadName, 16);
        return std::string{threadName};
    }

    void setThreadName(const std::string &name) {
        pthread_setname_np(pthread_self(), name.substr(0, 15).c_str());
    }

    void Backtrace(std::vector<std::string> &bt, int size, int skip) {
        void **array = (void **) malloc((sizeof(void *) * size));
        int s = ::backtrace(array, size);

        char ** strings = backtrace_symbols(array, s);
        if (strings == nullptr) {
            LUWU_LOG_ERROR(g_logger) << "backtrace_symbols error" << std::endl;
            return;
        }

        for (int i = skip; i < s; ++i) {
            bt.emplace_back(strings[i]);
        }

        free(strings);
        free(array);
    }

    std::string BacktraceToString(int size, int skip, const std::string &prefix) {
        std::vector<std::string> bt;
        Backtrace(bt, size, skip);
        std::stringstream ss;

        for (const auto &i : bt) {
            ss << prefix << i << std::endl;
        }
        return ss.str();
    }

    uint64_t GetCurrentMS() {
        struct timeval tv{};
        gettimeofday(&tv, nullptr);
        return tv.tv_sec * 1000ul + tv.tv_usec / 1000;
    }

    uint64_t GetCurrentUS() {
        struct timeval tv{};
        gettimeofday(&tv, nullptr);
        return tv.tv_sec * 1000 * 1000ul + tv.tv_usec;
    }
}
