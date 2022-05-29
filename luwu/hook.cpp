//
// Created by liucxi on 2022/5/29.
//
#include "hook.h"
#include "fiber.h"
#include "iomanager.h"

#include <dlfcn.h>

namespace liucxi {
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

    struct _HookInit {
        _HookInit() {
            hook_init();
        }
    };

    static _HookInit s_hook_init;


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

}
