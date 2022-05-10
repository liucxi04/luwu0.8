//
// Created by liucxi on 2022/4/8.
//

#ifndef LUWU_SINGLETON_H
#define LUWU_SINGLETON_H

namespace liucxi {

    template<typename T>
    class Singleton {
    public:
        static T *getInstance() {
            static T v;
            return &v;
        }
    };
}
#endif //LUWU_SINGLETON_H
