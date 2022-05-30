//
// Created by liucxi on 2022/5/29.
//
#include "hook.h"

#include "fiber.h"
//#include "macro.h"
#include "config.h"
#include "iomanager.h"
#include "fd_manager.h"

#include <dlfcn.h>
#include <cstdarg>

namespace liucxi {

    static ConfigVar<int>::ptr g_tcp_connect_timeout =
            Config::lookup("tcp.connect.timeout", 5000, "tcp connect timeout");

    static uint64_t s_connect_timeout = -1;

    static thread_local bool t_hook_enable = false;

    bool is_hook_enable() {
        return t_hook_enable;
    }

    void set_hook_enable(bool flag) {
        t_hook_enable = flag;
    }

#define HOOK_FUN(XX)    \
    XX(sleep)           \
    XX(usleep)          \
    XX(socket)          \
    XX(connect)         \
    XX(accept)          \
    XX(close)           \
    XX(read)            \
    XX(recv)            \
    XX(write)           \
    XX(send)            \
    XX(fcntl)           \
    XX(ioctl)           \
    XX(getsockopt)      \
    XX(setsockopt)      \

    void hook_init() {
        static bool is_init = false;
        if (is_init) {
            return;
        }
#define XX(name) name ## _f = (name ## _fun)dlsym(RTLD_NEXT, #name);
        HOOK_FUN(XX)
#undef XX
    }

    struct HookInit {
        HookInit() {
            hook_init();

            s_connect_timeout = g_tcp_connect_timeout->getValue();
            g_tcp_connect_timeout->addListener([](const int &oldVal, const int &newVal){
                LUWU_LOG_INFO(LUWU_LOG_NAME("system")) << "tcp connect timeout changed from "
                    << oldVal << " to " << newVal;
                s_connect_timeout = newVal;
            });
        }
    };

    static HookInit s_hook_init;
}

struct timer_info {
    bool cancelled = false;
};

template<typename OriginFun, typename ... Args>
static ssize_t do_io(int fd, OriginFun fun, const char *hook_fun_name,
                     uint32_t event, int timeout_so, Args &&... args) {
    if (!liucxi::is_hook_enable()) {
        return fun(fd, std::forward<Args>(args)...);
    }

    liucxi::FdContext::ptr ctx = liucxi::FdMgr::getInstance()->get(fd);
    if (!ctx) {
        return fun(fd, std::forward<Args>(args)...);
    }

    if (ctx->isClose()) {
        errno = EBADF;
        return -1;
    }

    if (!ctx->isSocket() || ctx->getUserNonBlock()) {
        return fun(fd, std::forward<Args>(args)...);
    }

    uint64_t timeout = ctx->getTimeout(timeout_so);
    std::shared_ptr<timer_info> tinfo(new timer_info);

    retry:
    ssize_t n = fun(fd, std::forward<Args>(args)...);
    while (n == -1 && errno == EINTR) {
        n = fun(fd, std::forward<Args>(args)...);
    }

    if (n == -1 && errno == EAGAIN) {
        liucxi::IOManager *iom = liucxi::IOManager::GetThis();
        std::weak_ptr<timer_info> winfo(tinfo);
        liucxi::Timer::ptr timer;

        if (timeout != -1) {
            timer = iom->addConditionTimer(timeout, [winfo, fd, iom, event]() {
                auto t = winfo.lock();
                if (!t || t->cancelled) {
                    return;
                }
                t->cancelled = ETIMEDOUT;
                iom->cancelEvent(fd, (liucxi::IOManager::Event) (event));
            }, winfo);
        }

        int rt = iom->addEvent(fd, (liucxi::IOManager::Event) (event));
        if (!rt) {
            LUWU_LOG_ERROR(LUWU_LOG_NAME("system")) << hook_fun_name << " addEvent("
                                                    << fd << " ," << event << ")";
            if (timer) {
                timer->cancel();
            }
            return -1;
        } else {
            liucxi::Fiber::GetThis()->yield();
            if (timer) {
                timer->cancel();
            }
            if (tinfo->cancelled) {
                errno = tinfo->cancelled;
                return -1;
            }
            goto retry;
        }
    }
    return n;
}

extern "C" {
#define XX(name) name ## _fun name ## _f = nullptr;
HOOK_FUN(XX)
#undef XX

unsigned int sleep(unsigned int seconds) {
    if (!liucxi::is_hook_enable()) {
        return sleep_f(seconds);
    }
    auto fiber = liucxi::Fiber::GetThis();
    auto iom = liucxi::IOManager::GetThis();
    iom->addTimer(seconds * 1000, [iom, fiber]() {
        iom->scheduler(fiber);
    });
    liucxi::Fiber::GetThis()->yield();
    return 0;
}

int usleep(useconds_t usec) {
    if (!liucxi::is_hook_enable()) {
        return usleep_f(usec);
    }
    auto fiber = liucxi::Fiber::GetThis();
    auto iom = liucxi::IOManager::GetThis();
    iom->addTimer(usec / 1000, [iom, fiber]() {
        iom->scheduler(fiber);
    });
    liucxi::Fiber::GetThis()->yield();
    return 0;
}

int socket(int domain, int type, int protocol) {
    if (!liucxi::is_hook_enable()) {
        return socket_f(domain, type, protocol);
    }
    int fd = socket_f(domain, type, protocol);
    if (fd >= 0) {
        liucxi::FdMgr::getInstance()->get(fd, true);
    }
    return fd;
}

int connect_with_timeout(int sockfd, const struct sockaddr *addr, socklen_t addlen, uint64_t timeout) {
    if (!liucxi::is_hook_enable()) {
        return connect_f(sockfd, addr, addlen);
    }

    liucxi::FdContext::ptr ctx = liucxi::FdMgr::getInstance()->get(sockfd);
    if (!ctx || ctx->isClose()) {
        errno = EBADF;
        return -1;
    }

    if (!ctx->isSocket() || ctx->getUserNonBlock()) {
        return connect_f(sockfd, addr, addlen);
    }

    int n = connect_f(sockfd, addr, addlen);
    if (n == 0) {
        return 0;
    } else if (n != -1 || errno != EINPROGRESS) {
        return n;
    }

    liucxi::IOManager *iom = liucxi::IOManager::GetThis();
    liucxi::Timer::ptr timer;
    std::shared_ptr<timer_info> tinfo(new timer_info);
    std::weak_ptr<timer_info> winfo(tinfo);

    if (timeout != -1) {
        timer = iom->addConditionTimer(timeout, [winfo, iom, sockfd](){
           auto t = winfo.lock();
            if (!t || t->cancelled) {
                return;
            }
            t->cancelled = ETIMEDOUT;
            iom->cancelEvent(sockfd, liucxi::IOManager::WRITE);
        }, winfo);
    }

    bool rt = iom->addEvent(sockfd, liucxi::IOManager::WRITE);
    if (rt) {
        liucxi::Fiber::GetThis()->yield();
        if (timer) {
            timer->cancel();
        }
        if (tinfo->cancelled) {
            errno = tinfo->cancelled;
            return -1;
        }
    } else {
        if (timer) {
            timer->cancel();
        }
        LUWU_LOG_ERROR(LUWU_LOG_NAME("system")) << "connect addEvent("
            << sockfd << ", WRITE) error";
    }

    int error = 0;
    socklen_t len = sizeof(int);
    if (-1 == getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, &len)) {
        return -1;
    }
    if (!error) {
        return 0;
    } else {
        errno = error;
        return -1;
    }
}

int connect(int sockfd, const struct sockaddr *addr, socklen_t addlen) {
    return connect_with_timeout(sockfd, addr, addlen, liucxi::s_connect_timeout);
}

int accept(int sockfd, struct sockaddr *addr, socklen_t *addlen) {
    auto fd = (int) do_io(sockfd, accept_f, "accept",
                          liucxi::IOManager::READ, SO_RCVTIMEO, addr, addlen);
    if (fd >= 0) {
        liucxi::FdMgr::getInstance()->get(fd, true);
    }
    return fd;
}

int close(int fd) {
    if (!liucxi::is_hook_enable()) {
        return close_f(fd);
    }
    auto ctx = liucxi::FdMgr::getInstance()->get(fd);
    if (ctx) {
        auto iom = liucxi::IOManager::GetThis();
        if (iom) {
            iom->cancelAll(fd);
        }
        liucxi::FdMgr::getInstance()->del(fd);
    }
    return close_f(fd);
}

ssize_t read(int fd, void *buf, size_t count) {
    return do_io(fd, read_f, "read",
                 liucxi::IOManager::READ, SO_RCVTIMEO, buf, count);
}

ssize_t recv(int sockfd, void *buf, size_t len, int flags) {
    return do_io(sockfd, recv_f, "recv",
                 liucxi::IOManager::READ, SO_RCVTIMEO, buf, len, flags);
}

ssize_t write(int fd, const void *buf, size_t count) {
    return do_io(fd, write_f, "write",
                 liucxi::IOManager::WRITE, SO_SNDTIMEO, buf, count);
}

ssize_t send(int sockfd, const void *buf, size_t len, int flags) {
    return do_io(sockfd, send_f, "send",
                 liucxi::IOManager::WRITE, SO_SNDTIMEO, buf, len, flags);
}

int fcntl(int fd, int cmd, ... /* arg */) {
    va_list va;
    va_start(va, cmd);
    switch(cmd) {
        case F_SETFL:
        {
            int arg = va_arg(va, int);
            va_end(va);
            liucxi::FdContext::ptr ctx = liucxi::FdMgr::getInstance()->get(fd);
            if (!ctx || ctx->isClose() || !ctx->isSocket()) {
                return fcntl_f(fd, cmd, arg);
            }

            ctx->setUserNonBlock(arg & O_NONBLOCK);
            if (ctx->getSysNonBlock()) {
                arg |= O_NONBLOCK;
            } else {
                arg &= ~O_NONBLOCK;
            }
            return fcntl_f(fd, cmd, arg);
        }
        case F_GETFL:
        {
            va_end(va);
            int flags = fcntl_f(fd, cmd);
            liucxi::FdContext::ptr ctx = liucxi::FdMgr::getInstance()->get(fd);
            if (!ctx || ctx->isClose() || !ctx->isSocket()) {
                return flags;
            }

            if (ctx->getUserNonBlock()) {
                return flags | O_NONBLOCK;
            } else {
                return flags & ~O_NONBLOCK;
            }
        }
        case F_DUPFD:
        case F_DUPFD_CLOEXEC:
        case F_SETFD:
        case F_SETOWN:
        case F_SETSIG:
        case F_SETLEASE:
        case F_NOTIFY:
        case F_SETPIPE_SZ:
        {
            int arg = va_arg(va, int);
            va_end(va);
            return fcntl_f(fd, cmd, arg);
        }
        case F_GETFD:
        case F_GETOWN:
        case F_GETSIG:
        case F_GETLEASE:
        case F_GETPIPE_SZ:
        {
            va_end(va);
            return fcntl_f(fd, cmd);
        }
        case F_SETLK:
        case F_SETLKW:
        case F_GETLK:
        {
            struct flock *arg = va_arg(va, struct flock*);
            va_end(va);
            return fcntl_f(fd, cmd, arg);
        }
        case F_GETOWN_EX:
        case F_SETOWN_EX:
        {
            struct f_owner_exlock *arg = va_arg(va, struct f_owner_exlock*);
            va_end(va);
            return fcntl_f(fd, cmd, arg);
        }
        default:
            va_end(va);
            return fcntl_f(fd, cmd);

    }
}

int ioctl(int fd, unsigned long int request, ...) {
    va_list va;
    va_start(va, request);
    void *arg = va_arg(va, void*);
    va_end(va);

    if (request == FIONBIO) {
        bool user_nonblock = !!*(int*)arg;
        liucxi::FdContext::ptr ctx = liucxi::FdMgr::getInstance()->get(fd);
        if (!ctx || ctx->isClose() || !ctx->isSocket()) {
            return ioctl_f(fd, request, arg);
        }
        ctx->setUserNonBlock(user_nonblock);
    }
    return ioctl_f(fd, request, arg);
}

int getsockopt(int sockfd, int level, int optname, void *optval, socklen_t *optlen) {
    return getsockopt_f(sockfd, level, optname, optval, optlen);
}

int setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen) {
    if (liucxi::is_hook_enable()) {
        return setsockopt_f(sockfd, level, optname, optval, optlen);
    }
    if (level == SOL_SOCKET) {
        if (optname == SO_RCVTIMEO || optname == SO_SNDTIMEO) {
            liucxi::FdContext::ptr ctx = liucxi::FdMgr::getInstance()->get(sockfd);
            if (ctx) {
                auto v = (const timeval*)optval;
                ctx->setTimeout(optname, v->tv_sec * 1000 + v->tv_usec / 1000);
            }
        }
    }
    return setsockopt_f(sockfd, level, optname, optval, optlen);
}

}
