//
// Created by liucxi on 2022/4/27.
//

#include "luwu.h"
#include "timer.h"

static liucxi::Logger::ptr g_logger = LUWU_LOG_ROOT();

static int timeout = 1000;
static liucxi::Timer::ptr s_timer;

void timer_callback() {
    LUWU_LOG_INFO(g_logger) << "timer callback, timeout = " << timeout;
    timeout += 1000;
    if(timeout < 5000) {
        s_timer->reset(timeout, true);
    } else {
        s_timer->cancel();
    }
}

void test_timer() {
    liucxi::IOManager iom;

    // 循环定时器
    s_timer = iom.addTimer(10, timer_callback, true);

    // 单次定时器
    iom.addTimer(50, []{
        LUWU_LOG_INFO(g_logger) << "500ms timeout";
    });
    iom.addTimer(50, []{
        LUWU_LOG_INFO(g_logger) << "5000ms timeout";
    });
}

int main(int argc, char *argv[]) {
    test_timer();

    LUWU_LOG_INFO(g_logger) << "end";

    return 0;
}