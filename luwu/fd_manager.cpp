//
// Created by liucxi on 2022/5/29.
//
#include "fd_manager.h"
#include "hook.h"

#include <sys/stat.h>

namespace liucxi {
    FdContext::FdContext(int fd)
        : m_isInit(false)
        , m_isSocket(false)
        , m_isSysNonBlock(false)
        , m_isUserNonBlock(false)
        , m_isClosed(false)
        , m_fd(fd)
        , m_recvTimeout(-1)
        , m_sendTimeout(-1){
        init();
    }

    bool FdContext::init() {
        if (isInit()) {
            return true;
        }
        m_recvTimeout = -1;
        m_sendTimeout = -1;

        struct stat fd_stat{};
        if (fstat(m_fd, &fd_stat) == -1) {
            m_isInit = false;
            m_isSocket = false;
        } else {
            m_isInit = true;
            m_isSocket = S_ISSOCK(fd_stat.st_mode);
        }

        if (m_isSocket) {
            int flags = fcntl_f(m_fd, F_GETFL, 0);
            if (!(flags & O_NONBLOCK)) {
                fcntl_f(m_fd, F_SETFL, flags | O_NONBLOCK);
            }
            m_isSysNonBlock = true;
        } else {
            m_isSysNonBlock = false;
        }

        m_isUserNonBlock = false;
        m_isClosed = false;
        return m_isInit;
    }

    void FdContext::setTimeout(int type, uint64_t v) {
        if (type == SO_RCVTIMEO) {
            m_recvTimeout = v;
        } else {
            m_sendTimeout = v;
        }
    }

    uint64_t FdContext::getTimeout(int type) const {
        if (type == SO_RCVTIMEO) {
            return m_recvTimeout;
        } else {
            return m_sendTimeout;
        }
    }

    FdManager::FdManager() {
        m_fds.resize(64);
    }

    FdContext::ptr FdManager::get(int fd, bool auto_create) {
        if (fd == -1) {
            return nullptr;
        }
        RWMutexType::ReadLock lock(m_mutex);
        if ((int)m_fds.size() <= fd) {
            if (!auto_create) {
                return nullptr;
            }
        } else {
            if (m_fds[fd] || !auto_create) {
                return m_fds[fd];
            }
        }
        lock.unlock();

        RWMutexType::WriteLock lock1(m_mutex);
        FdContext::ptr ctx(new FdContext(fd));
        if (fd >= (int)m_fds.size()) {
            m_fds.resize(fd * 2);
        }
        m_fds[fd] = ctx;
        return ctx;
    }

    void FdManager::del(int fd) {
        RWMutexType::WriteLock lock(m_mutex);
        if (fd >= (int)m_fds.size()) {
            return;
        }
        m_fds[fd].reset();
    }

}

