//
// Created by liucxi on 2022/5/29.
//

#ifndef LUWU_FD_MANAGER_H
#define LUWU_FD_MANAGER_H

#include <memory>
#include <vector>

#include "mutex.h"

namespace liucxi {
    class FdContext : public std::enable_shared_from_this<FdContext> {
    public:
        typedef std::shared_ptr<FdContext> ptr;

        explicit FdContext(int fd);

        ~FdContext() = default;

        bool init();

        bool close();

        bool isInit() const { return m_isInit; }

        bool isSocket() const { return m_isSocket; }

        bool isClose() const { return m_isClosed; }

        void setUserNonBlock(bool v) { m_isUserNonBlock = v; }

        bool getUserNonBlock() const { return m_isUserNonBlock; }

        void setSysNonBlock(bool v) { m_isSysNonBlock =v; }

        bool getSysNonBlock() const { return m_isSysNonBlock; }

        void setTimeout(int type, uint64_t v);

        uint64_t getTimeout(int type);

    private:
        bool m_isInit:1;
        bool m_isSocket:1;
        bool m_isSysNonBlock:1;
        bool m_isUserNonBlock:1;
        bool m_isClosed:1;
        int m_fd;

        uint64_t m_recvTimeout;
        uint64_t m_sendTimeout;
    };

    class FdManager {
    public:
        typedef RWMutex RWMutexType;

        FdManager();

        FdContext::ptr get(int fd, bool auto_create = false);

        void del(int fd);

    private:
        RWMutexType m_mutex;
        std::vector<FdContext::ptr> m_fds;
    };
}
#endif //LUWU_FD_MANAGER_H
