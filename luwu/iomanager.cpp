//
// Created by liucxi on 2022/4/25.
//

#include "iomanager.h"
#include "macro.h"
#include "log.h"

#include <cerrno>
#include <cstring>
#include <utility>
#include <fcntl.h>
#include <unistd.h>
#include <sys/epoll.h>

namespace liucxi {
    static Logger::ptr g_logger = LUWU_LOG_NAME("system");

    IOManager::FdContext::EventContext &IOManager::FdContext::getEventContext(Event e) {
        switch (e) {
            case IOManager::READ:
                return read;
            case IOManager::WRITE:
                return write;
            default:
                LUWU_ASSERT2(false, "getContext")
        }
    }

    void IOManager::FdContext::resetEventContext(EventContext &ctx) {
        ctx.scheduler = nullptr;
        ctx.fiber.reset();
        ctx.cb = nullptr;
    }

    void IOManager::FdContext::triggerEvent(IOManager::Event e) {
        LUWU_ASSERT(event & e)

        event = (Event)(event & ~e);
        EventContext &ctx = getEventContext(e);
        if (ctx.cb) {
            ctx.scheduler->scheduler(ctx.cb);
        } else {
            ctx.scheduler->scheduler(ctx.fiber);
        }
        resetEventContext(ctx);
    }

    IOManager::IOManager(size_t threads, bool use_caller, std::string name)
        : Scheduler(threads, use_caller, std::move(name)){
        m_epfd = epoll_create(5000);
        LUWU_ASSERT(m_epfd > 0)

        int rt = pipe(m_tickleFds);
        LUWU_ASSERT(!rt)

        epoll_event event{};
        memset(&event, 0, sizeof(event));
        event.events = EPOLLIN | EPOLLET;
        event.data.fd = m_tickleFds[0];

        rt = fcntl(m_tickleFds[0], F_SETFL, O_NONBLOCK);
        LUWU_ASSERT(!rt)

        rt = epoll_ctl(m_epfd, EPOLL_CTL_ADD, m_tickleFds[0], &event);
        LUWU_ASSERT(!rt)

        contextResize(32);
        start();
    }

    void IOManager::contextResize(size_t size) {
        m_fdContexts.resize(size);

        for (size_t i = 0; i < m_fdContexts.size(); ++i) {
            if (!m_fdContexts[i]) {
                m_fdContexts[i] = new FdContext;
                m_fdContexts[i]->fd = (int)i;
            }
        }
    }

    IOManager::~IOManager() {
        stop();
        close(m_epfd);
        close(m_tickleFds[0]);
        close(m_tickleFds[1]);

        for (auto &context : m_fdContexts) {
            delete context;
        }
    }

    bool IOManager::addEvent(int fd, Event event, std::function<void()> cb) {
        FdContext *fdContext = nullptr;
        RWMutexType::ReadLock lock(m_mutex);
        if ((int)m_fdContexts.size() > fd) {
            fdContext = m_fdContexts[fd];
            lock.unlock();
        } else {
            lock.unlock();
            RWMutexType::WriteLock lock1(m_mutex);
            contextResize(fd * 2);
            fdContext = m_fdContexts[fd];
        }

        FdContext::MutexType::Lock lock1(fdContext->mutex);
        if (fdContext->event & event) {
            LUWU_LOG_ERROR(g_logger) << "addEvent assert fd=" << fd
                                     << " event=" << (EPOLL_EVENTS)event
                                     << " fd_ctx.event=" << (EPOLL_EVENTS)fdContext->event;
            LUWU_ASSERT(!(fdContext->event & event));
        }

        int op = fdContext->event ? EPOLL_CTL_MOD : EPOLL_CTL_ADD;
        epoll_event epollEvent{};
        epollEvent.events = EPOLLET | fdContext->event | event;
        epollEvent.data.ptr = fdContext;

        int rt = epoll_ctl(m_epfd, op, fd, &epollEvent);
        if (rt) {
            LUWU_LOG_ERROR(g_logger) << "epoll_ctl(" << m_epfd << ", "
                                     << op << ", " << fd << ", " << (EPOLL_EVENTS)epollEvent.events << "):"
                                     << rt << "(" << errno << ")(" << strerror(errno) << ")";
            return false;
        }

        ++m_pendingEventCount;
        fdContext->event = (Event)(fdContext->event | event);
        FdContext::EventContext &eventContext = fdContext->getEventContext(event);
        LUWU_ASSERT(!eventContext.scheduler && !eventContext.fiber && !eventContext.cb);

        eventContext.scheduler = Scheduler::GetThis();
        if (cb) {
            eventContext.cb.swap(cb);
        } else {
            eventContext.fiber = Fiber::GetThis();
            LUWU_ASSERT(eventContext.fiber->getState() == Fiber::RUNNING)
        }
        return true;
    }

    bool IOManager::delEvent(int fd, Event event) {
        RWMutexType::ReadLock lock(m_mutex);
        if ((int)m_fdContexts.size() <= fd) {
            return false;
        }
        FdContext *fdContext = m_fdContexts[fd];
        lock.unlock();

        FdContext::MutexType::Lock lock1(fdContext->mutex);
        if (!(fdContext->event & event)) {
            return false;
        }

        auto new_event = (Event)(fdContext->event & ~event);
        int op = new_event ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
        epoll_event epollEvent{};
        epollEvent.events = EPOLLET | new_event;
        epollEvent.data.ptr = fdContext;

        int rt = epoll_ctl(m_epfd, op, fd, &epollEvent);
        if (rt) {
            LUWU_LOG_ERROR(g_logger) << "epoll_ctl(" << m_epfd << ", "
                                     << op << ", " << fd << ", " << (EPOLL_EVENTS)epollEvent.events << "):"
                                     << rt << "(" << errno << ")(" << strerror(errno) << ")";
            return false;
        }

        --m_pendingEventCount;
        fdContext->event = new_event;
        FdContext::EventContext &eventContext = fdContext->getEventContext(event);
        fdContext->resetEventContext(eventContext);

        return true;
    }

    bool IOManager::cancelEvent(int fd, Event event) {
        RWMutexType::ReadLock lock(m_mutex);
        if ((int)m_fdContexts.size() <= fd) {
            return false;
        }
        FdContext *fdContext = m_fdContexts[fd];
        lock.unlock();

        FdContext::MutexType::Lock lock1(fdContext->mutex);
        if (!(fdContext->event & event)) {
            return false;
        }

        auto new_event = (Event)(fdContext->event & ~event);
        int op = new_event ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
        epoll_event epollEvent{};
        epollEvent.events = EPOLLET | new_event;
        epollEvent.data.ptr = fdContext;

        int rt = epoll_ctl(m_epfd, op, fd, &epollEvent);
        if (rt) {
            LUWU_LOG_ERROR(g_logger) << "epoll_ctl(" << m_epfd << ", "
                                     << op << ", " << fd << ", " << (EPOLL_EVENTS)epollEvent.events << "):"
                                     << rt << "(" << errno << ")(" << strerror(errno) << ")";
            return false;
        }

        fdContext->triggerEvent(event);
        --m_pendingEventCount;
        return true;
    }

    bool IOManager::cancelAll(int fd) {
        RWMutexType::ReadLock lock(m_mutex);
        if ((int)m_fdContexts.size() <= fd) {
            return false;
        }
        FdContext *fdContext = m_fdContexts[fd];
        lock.unlock();

        FdContext::MutexType::Lock lock1(fdContext->mutex);
        if (!fdContext->event) {
            return false;
        }

        int op = EPOLL_CTL_DEL;
        epoll_event epollEvent{};
        epollEvent.events = 0;
        epollEvent.data.ptr = fdContext;

        int rt = epoll_ctl(m_epfd, op, fd, &epollEvent);
        if (rt) {
            LUWU_LOG_ERROR(g_logger) << "epoll_ctl(" << m_epfd << ", "
                                     << op << ", " << fd << ", " << (EPOLL_EVENTS)epollEvent.events << "):"
                                     << rt << "(" << errno << ")(" << strerror(errno) << ")";
            return false;
        }

        if (fdContext->event & READ) {
            fdContext->triggerEvent(READ);
            --m_pendingEventCount;
        }
        if (fdContext->event & WRITE) {
            fdContext->triggerEvent(WRITE);
            --m_pendingEventCount;
        }
        LUWU_ASSERT(fdContext->event == 0);
        return true;
    }

    IOManager *IOManager::GetThis() {
        return dynamic_cast<IOManager*>(Scheduler::GetThis());
    }

    void IOManager::tickle() {
        if (hasIdleThreads()) {
            return;
        }
        ssize_t rt = write(m_tickleFds[1], "T", 1);
        LUWU_ASSERT(rt == 1)
    }

    bool IOManager::stopping() {
        return m_pendingEventCount == 0 && Scheduler::stopping();
    }

    void IOManager::idle() {

    }

}