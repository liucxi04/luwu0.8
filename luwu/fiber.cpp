//
// Created by liucxi on 2022/4/18.
//
#include "macro.h"
#include "config.h"
#include "fiber.h"

#include <utility>
#include "log.h"

namespace liucxi {
    static Logger::ptr g_logger = LUWU_LOG_NAME("system");

    static std::atomic<uint64_t> s_fiberId{0};
    static std::atomic<uint64_t> s_fiberCount{0};

    static thread_local Fiber *t_fiber = nullptr;
    static thread_local Fiber::ptr t_threadFiber = nullptr;

    static ConfigVar<uint32_t>::ptr g_fiberStackSize =
            Config::lookup<uint32_t>("fiber.stack_size", 128 * 1024, "fiber stack size");

    class MallocStackAllocator {
    public:
        static void *Alloc(size_t size) {
            return malloc(size);
        }
        static void Dealloc(void *vp, size_t size) {
            return free(vp);
        }
    };

    using StackAllocator = MallocStackAllocator;

    Fiber::Fiber() {
        SetThis(this);
        m_state = RUNNING;

        if (getcontext(&m_context)) {
            LUWU_ASSERT2(false, "getcontext");
        }

        ++s_fiberCount;
        m_id = s_fiberId++;
    }
    Fiber::Fiber(std::function<void()> cb, size_t stackSize)
        : m_id(s_fiberId++)
        , m_cb(std::move(cb)){
        ++s_fiberId;
        m_stackSize = stackSize ? stackSize : g_fiberStackSize->getValue();
        m_stack = StackAllocator::Alloc(m_stackSize);

        if (getcontext(&m_context)) {
            LUWU_ASSERT2(false, "getcontext");
        }

        m_context.uc_link = nullptr;
        m_context.uc_stack.ss_sp = m_stack;
        m_context.uc_stack.ss_size = m_stackSize;

        makecontext(&m_context, &Fiber::MainFunc, 0);
    }

    Fiber::~Fiber() {
        --s_fiberCount;
        if (m_stack) {
            LUWU_ASSERT(m_state == TERM);
            StackAllocator::Dealloc(m_stack, m_stackSize);
        } else {
            LUWU_ASSERT(!m_cb);
            LUWU_ASSERT(m_state == RUNNING);
            Fiber *cur = t_fiber;
            if (cur == this) {
                SetThis(nullptr);
            }
        }
    }

    void Fiber::reset(std::function<void()> cb) {
        LUWU_ASSERT(m_stack);
        LUWU_ASSERT(m_state == TERM);
        m_cb = std::move(cb);

        if (getcontext(&m_context)) {
            LUWU_ASSERT2(false, "getcontext");
        }

        m_context.uc_link = nullptr;
        m_context.uc_stack.ss_sp = m_stack;
        m_context.uc_stack.ss_size = m_stackSize;

        makecontext(&m_context, &Fiber::MainFunc, 0);
        m_state = READY;
    }

    void Fiber::resume() {
        LUWU_ASSERT(m_state != TERM && m_state != RUNNING);
        SetThis(this);
        m_state = RUNNING;

        if (swapcontext(&(t_threadFiber->m_context), &m_context)) {
            LUWU_ASSERT2(false, "swapcontext error");
        }
    }

    void Fiber::yield() {
        LUWU_ASSERT(m_state ==TERM || m_state == RUNNING);
        SetThis(t_threadFiber.get());
        if (m_state != TERM) {
            m_state = RUNNING;
        }

        if (swapcontext(&m_context, &(t_threadFiber->m_context))) {
            LUWU_ASSERT2(false, "swapcontext error");
        }
    }

    void Fiber::SetThis(Fiber *fiber) {
        t_fiber = fiber;
    }

    Fiber::ptr Fiber::GetThis() {
        if (t_fiber) {
            return t_fiber->shared_from_this();
        }

        Fiber::ptr main_fiber(new Fiber);
        LUWU_ASSERT(t_fiber == main_fiber.get());
        t_threadFiber = main_fiber;
        return t_fiber->shared_from_this();
    }

    uint64_t Fiber::TotalFibers() {
        return s_fiberCount;
    }

    uint64_t Fiber::GetFiberId() {
        if (t_fiber) {
            return t_fiber->getId();
        }
        return 0;
    }

    void Fiber::MainFunc() {
        Fiber::ptr cur = GetThis();
        LUWU_ASSERT(cur);

        cur->m_cb();
        cur->m_cb = nullptr;
        cur->m_state = TERM;

        auto raw_ptr = cur.get();
        cur.reset();
        raw_ptr->yield();
    }

}

