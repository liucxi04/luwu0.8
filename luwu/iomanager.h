//
// Created by liucxi on 2022/4/25.
//

#ifndef LUWU_IOMANAGER_H
#define LUWU_IOMANAGER_H

#include <memory>
#include <vector>
#include <functional>

#include "timer.h"
#include "scheduler.h"

namespace liucxi {
    class IOManager : public Scheduler, public TimerManager{
    public:
        typedef std::shared_ptr<IOManager> ptr;
        typedef RWMutex RWMutexType;

        enum Event {
            NONE = 0x0,
            READ = 0x1,
            WRITE = 0x4,
        };

    private:
        struct FdContext {
            typedef Mutex MutexType;
            struct EventContext {
                Scheduler *scheduler = nullptr;
                Fiber::ptr fiber;
                std::function<void()> cb;
            };

            EventContext &getEventContext(Event event);
            static void resetEventContext(EventContext &ctx);
            void triggerEvent(Event event);
            EventContext read;
            EventContext write;
            int fd = 0;
            Event event = NONE;
            MutexType mutex;
        };

    public:
        explicit IOManager(size_t threads = 1, bool use_caller = true, std::string name = "");

        ~IOManager() override;

        bool addEvent(int fd, Event event, std::function<void()> cb = nullptr);

        bool delEvent(int fd, Event event);

        bool cancelEvent(int fd, Event event);

        bool cancelAll(int fd);

        static IOManager *GetThis();

    protected:
        void tickle() override;

        void idle() override;

        bool stopping() override;

        bool stopping(uint64_t &timeout);

        void onTimerInsertAtFront() override;

        void contextResize(size_t size);

    private:
        int m_epfd{};
        int m_tickleFds[2]{};

        std::atomic<size_t> m_pendingEventCount = {0};
        RWMutexType m_mutex;
        std::vector<FdContext*> m_fdContexts;

    };
}

#endif //LUWU_IOMANAGER_H
