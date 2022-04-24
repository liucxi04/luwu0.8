//
// Created by liucxi on 2022/4/20.
//

#include "scheduler.h"

#include <utility>
#include "log.h"
#include "macro.h"
#include "utils.h"

namespace liucxi {
    static Logger::ptr g_logger = LUWU_LOG_NAME("system");

    /// 当前线程的调度器，同一个调度器下的所有线程共享同一个实例
    static thread_local Scheduler *t_scheduler = nullptr;
    /// 当前线程的调度协程，每个线程独有一份
    static thread_local Fiber *t_scheduler_fiber = nullptr;

    Scheduler::Scheduler(size_t threads, bool use_call, std::string name)
        : m_name(std::move(name))
        , m_useCaller(use_call){

        LUWU_ASSERT(threads > 0);
        if (use_call) {
            --threads;
            Fiber::GetThis(); // 初始化当前线程的主协程
            LUWU_ASSERT(GetThis() == nullptr); // 当前线程还没有调度器
            t_scheduler = this;

            /**
             * 调度器所在线程的主协程和调度协程不是同一个
             * 把主协程暂时保存起来，等调度协程结束时，再执行
             */
            m_rootFiber.reset(new Fiber([this] { run(); }));

            Thread::SetName(m_name);
            t_scheduler_fiber = m_rootFiber.get();
            m_rootThread = getThreadId();
            m_threadIds.push_back(m_rootThread);
        } else {
            m_rootThread = -1;
        }
        m_threadCount = threads;
    }

    Scheduler::~Scheduler() {
        LUWU_ASSERT(m_stopping);
        if (GetThis() == this) {
            t_scheduler = nullptr;
        }
    }

    Scheduler *Scheduler::GetThis() {
        return t_scheduler;
    }

    void Scheduler::setThis() {
        t_scheduler = this;
    }

    Fiber *Scheduler::GetMainFiber() {
        return t_scheduler_fiber;
    }

    void Scheduler::start() {
        MutexType::Lock lock(m_mutex);
        if (m_stopping) {
            return;
        }
        LUWU_ASSERT(m_threads.empty())
        m_threads.resize(m_threadCount);
        for (size_t i = 0; i < m_threadCount; ++i) {
            m_threads[i].reset(new Thread([this] { run(); },
                                          m_name + "_" + std::to_string(i)));
            m_threadIds.push_back(m_threads[i]->getId());
        }
    }

    bool Scheduler::stopping() {
        MutexType::Lock lock(m_mutex);
        return m_stopping && m_fibers.empty() && m_activeThreadCount == 0;
    }

    void Scheduler::tickle() {

    }

    void Scheduler::idle() {
        while (!stopping()) {
            Fiber::GetThis()->yield();
        }
    }

    void Scheduler::stop() {
        if (stopping()) {
            return;
        }
        m_stopping = true;

        /// 如果 use caller，那么只能由 caller 线程发起 stop
        if (m_useCaller) {
            LUWU_ASSERT(GetThis() == this);
        } else {
            LUWU_ASSERT(GetThis() != this);
        }

        for (size_t i = 0; i < m_threadCount; ++i) {
            tickle();
        }

        if (m_rootFiber) {
            tickle();
        }

        /// 在 use caller 情况下，协程调度器结束时，应该返回 caller 协程
        if (m_rootFiber) {
            m_rootFiber->resume();
        }

        std::vector<Thread::ptr> thrs;
        {
            MutexType::Lock lock(m_mutex);
            thrs.swap(m_threads);
        }
        for (auto &i : thrs) {
            i->join();
        }
    }

    void Scheduler::run() {
        setThis();
        // 当前线程不是调度器所在的线程
        if (getThreadId() != m_rootThread) {
            t_scheduler_fiber = Fiber::GetThis().get();
        }

        Fiber::ptr idle_fiber(new Fiber([this] { idle(); }));
        Fiber::ptr cb_fiber;

        SchedulerTask task;
        while (true) {
            task.reset();
            bool tickle_me = false; // 是否需要 tickle 其他线程进行任务调度
            {
                MutexType::Lock lock(m_mutex);
                auto it = m_fibers.begin();
                while (it != m_fibers.end()) {
                    if (it->thread != -1 && it->thread != getThreadId()) {
                        ++it;
                        tickle_me = true;
                        continue;
                    }

                    LUWU_ASSERT(it->fiber || it->cb);

                    //后续模块需要
//                    if (it->fiber && it->fiber->getState() == Fiber::RUNNING) {
//                        ++it;
//                        continue;
//                    }
                    // 找到了一个任务，开始调度
                    task = *it;
                    m_fibers.erase(it++);
                    ++m_activeThreadCount;
                    break;
                }
                // 当前线程拿完一个任务后，任务队列还有剩余，也需要 tickle 其他线程
                tickle_me |= (it != m_fibers.end());
            }

            if (tickle_me) {
                tickle();
            }

            if (task.fiber) {
                task.fiber->resume();
                --m_activeThreadCount;
                task.reset();
            } else if (task.cb) {
                if (cb_fiber) {
                    cb_fiber->reset(task.cb);
                } else {
                    cb_fiber.reset(new Fiber(task.cb));
                }
                task.reset();
                cb_fiber->resume();
                --m_activeThreadCount;
                cb_fiber.reset();
            } else {
                // 进到这里说明队列空了，调度 idle 协程即可
                if (idle_fiber->getState() == Fiber::TERM) {
                    break;
                }
                ++m_idleThreadCount;
                idle_fiber->resume();
                --m_idleThreadCount;
            }
        }
    }
}
