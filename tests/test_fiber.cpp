//
// Created by liucxi on 2022/4/18.
//

#include "luwu.h"
#include <iostream>

void run() {
    std::cout << "fiber in" << std::endl;
    liucxi::Fiber::GetThis()->yield();
    std::cout << "fiber out" << std::endl;
}

int main(int argc, char *argv[]) {
    liucxi::Fiber::GetThis();
    std::cout << "main in" << std::endl;
    liucxi::Fiber::ptr fib(new liucxi::Fiber(run, 128));
    fib->resume();
    std::cout << "main out" << std::endl;

    return 0;
}

