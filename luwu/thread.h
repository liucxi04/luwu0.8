//
// Created by liucxi on 2022/4/12.
//

#ifndef LUWU_THREAD_H
#define LUWU_THREAD_H

#include <memory>
#include <functional>
#include "noncopyable.h"
#include "mutex.h"

namespace liucxi {
    /**
     * @brief 线程类
     * */
    class Thread : Noncopyable {
    public:
        typedef std::shared_ptr<Thread> ptr;

        Thread(std::function<void()> callback, std::string name);

        ~Thread();

        /**
         * @brief 等待线程执行完成
         * */
        void join();

        pid_t getId() const { return m_id; }

        const std::string &getName() const { return m_name; }

        /**
         * @brief 获取当前线程指针
         * */
        static Thread *GetThis();

        /**
         * @brief 获取当前线程名称
         * */
        static const std::string &GetName();

        /**
         * @brief 设置当前线程名称
         * */
        static void SetName(const std::string &name);

    private:
        /**
         * @brief 线程执行函数
         * */
        static void *run(void *arg);

    private:
        pid_t m_id = -1;                    // 线程 id
        pthread_t m_thread = 0;             // 线程标识符
        std::function<void()> m_callback;   // 线程执行函数，无参数，无返回值
        std::string m_name;                 // 线程名

        Semaphore m_sem;
    };
}

#endif //LUWU_THREAD_H
