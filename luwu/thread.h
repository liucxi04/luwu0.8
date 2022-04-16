//
// Created by liucxi on 2022/4/12.
//

#ifndef LUWU_THREAD_H
#define LUWU_THREAD_H

#include <memory>
#include <functional>
#include "noncopyable.h"

namespace liucxi {
    class Thread : Noncopyable {
    public:
        typedef std::shared_ptr<Thread> ptr;

        Thread(std::function<void()> callback, std::string name);

        ~Thread();

        pid_t getId() const { return m_id; }

        const std::string &getName() const { return m_name; }

        void join();

        static Thread *GetThis();

        static const std::string &GetName();

        static void SetName(const std::string &name);

    private:
        static void *run(void *arg);

    private:
        pid_t m_id = -1;
        pthread_t m_thread = 0;
        std::function<void()> m_callback;
        std::string m_name;
    };
}

#endif //LUWU_THREAD_H
