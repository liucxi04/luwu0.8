//
// Created by liucx on 2022/4/8.
//

#ifndef LUWU_SINGLETON_H
#define LUWU_SINGLETON_H

#include <memory>

namespace liucxi {
    template<typename T, typename X, int N>
    T &getInstance() {
        static T v;
        return v;
    }

    template<typename T, typename X, int N>
    std::shared_ptr<T> getInstance() {
        static std::shared_ptr<T> v(new T);
        return v;
    }

    template<typename T, typename X = void, int N = 0>
    class Singleton {
    public:
        static T &getInstance() {
            static T v;
            return v;
        }
    };

    template<typename T, typename X = void, int N = 0>
    class SingletonPtr {
    public:
        static std::shared_ptr<T> getInstance() {
            static std::shared_ptr<T> v(new T);
            return v;
        }
    };
}
#endif //LUWU_SINGLETON_H
