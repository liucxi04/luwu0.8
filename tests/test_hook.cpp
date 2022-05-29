//
// Created by liucxi on 2022/5/29.
//
#include "macro.h"
#include "hook.h"
#include "iomanager.h"

static liucxi::Logger::ptr g_logger = LUWU_LOG_ROOT();

void test_hook() {
    liucxi::IOManager iom;
    iom.scheduler([](){
        sleep(2);
        LUWU_LOG_INFO(g_logger) << "sleep2";
    });
    iom.scheduler([](){
        sleep(3);
        LUWU_LOG_INFO(g_logger) << "sleep3";
    });
    LUWU_LOG_INFO(g_logger) << "test_hook";
}

int main() {
    test_hook();
    return 0;
}

