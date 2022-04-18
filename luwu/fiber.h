//
// Created by liucxi on 2022/4/18.
//

#ifndef LUWU_FIBER_H
#define LUWU_FIBER_H

#include <functional>
#include <memory>
#include <ucontext.h>

namespace liucxi {
    class Fiber : public std::enable_shared_from_this<Fiber> {
    public:
        typedef std::shared_ptr<Fiber> ptr;
        enum State {
            READY,
            RUNNING,
            TERM
        };
    private:
        Fiber();

    public:
        explicit Fiber(std::function<void()> cb, size_t stackSize = 0);

        ~Fiber();

        void reset(std::function<void()> cb);

        void resume();

        void yield();

        uint64_t getId() const { return m_id; }

    public:
        static uint64_t GetFiberId();

        static Fiber::ptr GetThis();

        static void SetThis(Fiber *fiber);

        static void MainFunc();

        static uint64_t TotalFibers();

    private:
        uint64_t m_id = 0;
        uint32_t m_stackSize = 0;
        State m_state = READY;
        ucontext_t m_context{};
        void *m_stack = nullptr;
        std::function<void()> m_cb;
    };
}
#endif //LUWU_FIBER_H
