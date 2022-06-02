//
// Created by liucxi on 2022/5/29.
//

#ifndef LUWU_HOOK_H
#define LUWU_HOOK_H

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstdint>

namespace liucxi {
    bool is_hook_enable();
    void set_hook_enable(bool flag);
}

extern "C" {
// sleep
typedef unsigned int (*sleep_fun)(unsigned int seconds);
/**
 * @details 指在其他文件有一个 sleep_fun 类型的 sleep_f，
 * 即在 hook.cpp 文件还有一个函数指针 sleep_f，和 sleep_fun 函数指针签名相同
 * sleep_f 指向原始系统调用，sleep_fun 用来定义函数签名，下同。
 */
extern sleep_fun sleep_f;

typedef int (*usleep_fun)(useconds_t usec);
extern usleep_fun usleep_f;

//socket
typedef int (*socket_fun)(int domain, int type, int protocol);
extern socket_fun socket_f;

typedef int (*connect_fun)(int sockfd, const struct sockaddr *addr, socklen_t addlen);
extern connect_fun connect_f;

typedef int (*accept_fun)(int sockfd, struct sockaddr *addr, socklen_t *addlen);
extern accept_fun accept_f;

typedef int (*close_fun)(int fd);
extern close_fun close_f;

// read
typedef ssize_t (*read_fun)(int fd, void *buf, size_t count);
extern read_fun read_f;

typedef ssize_t (*recv_fun)(int sockfd, void *buf, size_t len, int flags);
extern recv_fun recv_f;

// write
typedef ssize_t (*write_fun)(int fd, const void *buf, size_t count);
extern write_fun write_f;

typedef ssize_t (*send_fun)(int sockfd, const void *buf, size_t len, int flags);
extern send_fun send_f;

// fcntl
typedef int (*fcntl_fun)(int fd, int cmd, ... /* arg */);
extern fcntl_fun fcntl_f;

typedef int (*ioctl_fun)(int fd, unsigned long int request, ...);
extern ioctl_fun ioctl_f;

typedef int (*getsockopt_fun)(int sockfd, int level, int optname, void *optval, socklen_t *optlen);
extern getsockopt_fun getsockopt_f;

typedef int (*setsockopt_fun)(int sockfd, int level, int optname, const void *optval, socklen_t optlen);
extern setsockopt_fun setsockopt_f;

extern int connect_with_timeout(int sockfd, const struct sockaddr *addr, socklen_t addlen, uint64_t timeout);
}
#endif //LUWU_HOOK_H
