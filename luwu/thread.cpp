//
// Created by liucxi on 2022/4/16.
//

#include <utility>
#include "thread.h"
#include "macro.h"
#include "log.h"

namespace liucxi {
    static thread_local Thread *t_thread;
    static thread_local std::string t_thread_name = "UNKNOWN";

    static Logger::ptr g_logger = LUWU_LOG_NAME("system");

    Thread::Thread(std::function<void()> callback, std::string name)
        : m_callback(std::move(callback))
        , m_name(std::move(name)){

        if (m_name.empty()) {
            m_name = "UNKNOWN";
        }
        int rt = pthread_create(&m_thread, nullptr, &run, this);
        if (rt) {
            LUWU_LOG_ERROR(g_logger) << "pthread_create fail, rt=" << rt << " name=" << name;
            throw std::logic_error("pthread_create error");
        }
        m_sem.wait();
    }

    Thread::~Thread() {
        if (m_thread) {
            pthread_detach(m_thread);
        }
    }

    Thread *Thread::GetThis() {
        return t_thread;
    }

    const std::string &Thread::GetName() {
        return t_thread_name;
    }

    void Thread::SetName(const std::string &name) {
        if (name.empty()) {
            return;
        }
        if (t_thread) {
            t_thread->m_name = name;
        }
        t_thread_name = name;
    }

    void Thread::join() {
        if (m_thread) {
            int rt = pthread_join(m_thread, nullptr);
            if (rt) {
                LUWU_LOG_ERROR(g_logger) << "pthread_join fail, rt=" << rt << " name=" << m_name;
                throw std::logic_error("pthread_join error");
            }
            m_thread = 0;
        }
    }

    void *Thread::run(void *arg) {
        auto *thread = (Thread *)arg;
        t_thread = thread;
        t_thread_name = thread->m_name;
        thread->m_id = liucxi::getThreadId();
        pthread_setname_np(pthread_self(), thread->m_name.substr(0, 15).c_str());

        std::function<void()> callback;
        callback.swap(thread->m_callback);

        thread->m_sem.notify();
        callback();
        return nullptr;
    }
}
