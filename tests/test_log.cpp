//
// Created by liucxi on 2022/4/7.
//

#include <iostream>
#include <unistd.h>
#include "../luwu/log.h"
using namespace liucxi;

Logger::ptr g_logger = LUWU_LOG_ROOT(); // 默认WARN级别

int main(int argc, char *argv[]) {

    auto ins = LoggerMgr::getInstance();
    std::cout << "********1********"<<std::endl;
    LUWU_LOG_FATAL(g_logger) << "fatal msg";
    LUWU_LOG_ERROR(g_logger) << "err msg";
    LUWU_LOG_INFO(g_logger) << "info msg";
    LUWU_LOG_DEBUG(g_logger) << "debug msg";
    std::cout << "********2********"<<std::endl;
    LUWU_LOG_FMT_FATAL(g_logger, "fatal %s:%d ", __FILE__, __LINE__);
    LUWU_LOG_FMT_ERROR(g_logger, "err %s:%d ", __FILE__, __LINE__);
    LUWU_LOG_FMT_INFO(g_logger, "info %s:%d ", __FILE__, __LINE__);
    LUWU_LOG_FMT_DEBUG(g_logger, "debug %s:%d ", __FILE__, __LINE__);
    std::cout << "********3********"<<std::endl;
    sleep(1);
    setThreadName("brand_new_thread");

    g_logger->setLevel(LogLevel::WARN);
    LUWU_LOG_FATAL(g_logger) << "fatal msg";
    LUWU_LOG_ERROR(g_logger) << "err msg";
    LUWU_LOG_INFO(g_logger) << "info msg"; // 不打印
    LUWU_LOG_DEBUG(g_logger) << "debug msg"; // 不打印
    std::cout << "********4********"<<std::endl;

    FileLogAppender::ptr fileAppender(new FileLogAppender("log.txt"));
    g_logger->addAppender(fileAppender);
    LUWU_LOG_FATAL(g_logger) << "fatal msg";
    LUWU_LOG_ERROR(g_logger) << "err msg";
    LUWU_LOG_INFO(g_logger) << "info msg"; // 不打印
    LUWU_LOG_DEBUG(g_logger) << "debug msg"; // 不打印

    Logger::ptr test_logger = LUWU_LOG_NAME("test_logger");
    StdoutLogAppender::ptr appender(new StdoutLogAppender);
    LogFormatter::ptr formatter(new LogFormatter("%d:%rms%T%p%T%c%T%f:%l %m%n")); // 时间：启动毫秒数 级别 日志名称 文件名：行号 消息 换行
    appender->setFormatter(formatter);
    test_logger->addAppender(appender);
    test_logger->setLevel(LogLevel::WARN);
    std::cout << "********5********"<<std::endl;
    LUWU_LOG_ERROR(test_logger) << "err msg";
    LUWU_LOG_INFO(test_logger) << "info msg"; // 不打印
    return 0;
}


