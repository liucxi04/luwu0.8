//
// Created by liucxi on 2022/5/29.
//
#include "fd_manager.h"

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
    }

    bool FdContext::close() {

    }

    void FdContext::setTimeout(int type, uint64_t v) {

    }

    uint64_t FdContext::getTimeout(int type) {

    }

    FdManager::FdManager() {

    }

    FdContext::ptr FdManager::get(int fd, bool auto_create) {

    }

    void FdManager::del(int fd) {

    }

}

