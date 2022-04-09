//
// Created by liucxi on 2022/4/6.
//

#ifndef LUWU_LOG_H
#define LUWU_LOG_H

#include <list>
#include <map>
#include <cstdio>
#include <memory>
#include <string>
#include <cstdint>
#include <memory>
#include <sstream>
#include <fstream>
#include <vector>
#include "utils.h"
#include "singleton.h"


/**
 * 获取 root 日志器
 * */
#define LUWU_LOG_ROOT() liucxi::LoggerMgr::getInstance().getRoot()

/**
 * 获取指定名称的日志器
 * */
#define LUWU_LOG_NAME(name) liucxi::LoggerMgr::getInstance().getLogger(name)

/**
 * 流式输出
 * */
#define LUWU_LOG_LEVEL(logger, level) \
    if (level >= logger->getLevel())  \
        liucxi::LogEventWrap(logger, liucxi::LogEvent::ptr(new liucxi::LogEvent(logger->getName(), \
            level, __FILE__, __LINE__, liucxi::getElapseMS() - logger->getCreateTime(), \
            liucxi::getThreadId(), liucxi::getFiberId(), time(nullptr), liucxi::getThreadName()))).getLogEvent() \
            ->getSS()

#define LUWU_LOG_DEBUG(logger) LUWU_LOG_LEVEL(logger, liucxi::LogLevel::DEBUG)
#define LUWU_LOG_INFO(logger) LUWU_LOG_LEVEL(logger, liucxi::LogLevel::INFO)
#define LUWU_LOG_WARN(logger) LUWU_LOG_LEVEL(logger, liucxi::LogLevel::WARN)
#define LUWU_LOG_ERROR(logger) LUWU_LOG_LEVEL(logger, liucxi::LogLevel::ERROR)
#define LUWU_LOG_FATAL(logger) LUWU_LOG_LEVEL(logger, liucxi::LogLevel::FATAL)

/**
 * fmt 输出
 * */
#define LUWU_LOG_FMT_LEVEL(logger, level, fmt, ...) \
    if(level >= logger->getLevel())                 \
        liucxi::LogEventWrap(logger, liucxi::LogEvent::ptr(new liucxi::LogEvent(logger->getName(), \
            level, __FILE__, __LINE__, liucxi::getElapseMS() - logger->getCreateTime(), \
            liucxi::getThreadId(), liucxi::getFiberId(), time(nullptr), liucxi::getThreadName()))).getLogEvent() \
            ->printf(fmt, __VA_ARGS__)

#define LUWU_LOG_FMT_DEBUG(logger, fmt, ...) LUWU_LOG_FMT_LEVEL(logger, liucxi::LogLevel::DEBUG, fmt, __VA_ARGS__)
#define LUWU_LOG_FMT_INFO(logger, fmt, ...) LUWU_LOG_FMT_LEVEL(logger, liucxi::LogLevel::INFO, fmt, __VA_ARGS__)
#define LUWU_LOG_FMT_WARN(logger, fmt, ...) LUWU_LOG_FMT_LEVEL(logger, liucxi::LogLevel::WARN, fmt, __VA_ARGS__)
#define LUWU_LOG_FMT_ERROR(logger, fmt, ...) LUWU_LOG_FMT_LEVEL(logger, liucxi::LogLevel::ERROR, fmt, __VA_ARGS__)
#define LUWU_LOG_FMT_FATAL(logger, fmt, ...) LUWU_LOG_FMT_LEVEL(logger, liucxi::LogLevel::FATAL, fmt, __VA_ARGS__)


namespace liucxi {

    class Logger;

    /**
     * 日志级别
     * */
    class LogLevel {
    public:
        enum Level {
            UNKNOWN = 0,
            DEBUG = 1,
            INFO = 2,
            WARN = 3,
            ERROR = 4,
            FATAL = 5
        };

        static std::string toString(LogLevel::Level level);
        static LogLevel::Level fromString(const std::string &str);
    };

    /**
     * 日志事件
     * */
    class LogEvent {
    public:
        typedef std::shared_ptr<LogEvent> ptr;

        LogEvent(std::string logger_name, LogLevel::Level level, const char *file, int32_t line
                , int64_t elapse, uint32_t thread_id, uint64_t fiber_id, time_t time, std::string thread_name);

        LogLevel::Level getLevel() const { return m_level; }
        std::string getContent() const { return m_ss.str(); }
        const char *getFile() const { return m_file; }
        int32_t getLine() const { return m_line; }
        uint32_t getElapse() const { return m_elapse; }
        uint32_t getThreadId() const { return m_threadId; }
        uint32_t getFiberId() const { return m_fiberId; }
        uint64_t getTime() const { return m_time; }
        std::stringstream &getSS()  { return m_ss; }
        const std::string &getThreadName() const { return m_threadName; }
        const std::string &getLoggerName() const { return m_loggerName; }
        void printf(const char *fmt, ...);

    private:
        std::string m_loggerName;       // 日志器名称
        LogLevel::Level m_level;        // 日志级别
        std::stringstream m_ss;         //日志内容，使用 string_stream 存储，方便流式输出
        const char *m_file = nullptr;   // 文件名
        int32_t m_line = 0;             // 行号
        uint32_t m_elapse = 0;          // 程序启动到现在的毫秒数
        uint32_t m_threadId = 0;        // 线程 ID
        uint32_t m_fiberId = 0;         // 协程 ID
        uint64_t m_time;                // 时间戳
        std::string m_threadName;       // 线程名称
    };

    /**
     * 日志格式器
     * */
    class LogFormatter {
    public:
        typedef std::shared_ptr<LogFormatter> ptr;

        explicit LogFormatter(std::string pattern =
                "%d{%Y-%m-%d %H:%M:%S}%T[%rms]%T%t%T%n%T%f%T[%p][%c]%T%f:%l%T%m%N");

        std::string format(const LogEvent::ptr& event);

        std::ostream &format(std::ostream &os, const LogEvent::ptr& event);
        /**
         * 字符串解析
         * */
        void init();

        bool getError() const { return m_error; }

        std::string getPattern() const { return m_pattern; }

    public:
        class FormatItem {
        public:
            typedef std::shared_ptr<FormatItem> ptr;

            virtual ~FormatItem() = default;

            virtual void format(std::ostream &os, LogEvent::ptr event) = 0;
        };

    private:
        bool m_error = false;
        std::string m_pattern;
        std::vector<FormatItem::ptr> m_items;
    };

    /**
     * 日志输出地
     * */
    class LogAppender {
    public:
        typedef std::shared_ptr<LogAppender> ptr;

        explicit LogAppender(LogFormatter::ptr formatter)
            : m_defaultFormatter(formatter) {
        };

        virtual ~LogAppender() = default;

        void setFormatter(const LogFormatter::ptr &formatter) { m_formatter = formatter; };

        LogFormatter::ptr getFormatter() const { return m_formatter ? m_formatter : m_defaultFormatter; }

        virtual void log(LogEvent::ptr event) = 0;

    protected:
        LogFormatter::ptr m_formatter;
        LogFormatter::ptr m_defaultFormatter;
    };

    /**
     * 输出到控制台的 LogAppender
     * */
    class StdoutLogAppender : public LogAppender {
    public:
        typedef std::shared_ptr<StdoutLogAppender> ptr;

        StdoutLogAppender()
            : LogAppender(std::make_shared<LogFormatter>()) {
        };

        void log(LogEvent::ptr event) override;
    };

    /**
     * 输出到文件的 LogAppender
     * */
    class FileLogAppender : public LogAppender {
    public:
        typedef std::shared_ptr<FileLogAppender> ptr;

        explicit FileLogAppender(const std::string &filename);

        void log(LogEvent::ptr event) override;

        /**
         * 重新打开文件，文件打开成功返回 true
         * */
        bool reopen();

    private:
        bool m_reopenError;
        std::string m_filename;
        std::ofstream m_filestream;
    };

    /**
     * 日志器
     * */
    class Logger {
    public:
        typedef std::shared_ptr<Logger> ptr;

        explicit Logger(std::string name = "default")
            : m_name(std::move(name))
            , m_level(LogLevel::WARN) {
        };

        void log(const LogEvent::ptr& event);


        void addAppender(const LogAppender::ptr& appender);

        void delAppender(const LogAppender::ptr& appender);

        LogLevel::Level getLevel() const { return m_level; };

        void setLevel(LogLevel::Level level) { m_level = level; }

        const std::string &getName() const { return m_name; }

        const uint64_t &getCreateTime() const { return m_creatTime; }

    private:
        std::string m_name;                                 //日志名称
        LogLevel::Level m_level;                            //日志级别
        std::list<LogAppender::ptr> m_appenderList;        //输出地列表
        uint64_t m_creatTime{};                              //创建时间
    };

    class LogEventWrap {
    public:
        LogEventWrap(Logger::ptr logger, LogEvent::ptr event)
            : m_logger(std::move(logger))
            , m_event(std::move(event)) {
        };
        ~LogEventWrap() {
            m_logger->log(m_event);
        };
        LogEvent::ptr getLogEvent() const { return m_event; }
    private:
        Logger::ptr m_logger;
        LogEvent::ptr m_event;
    };

    class LoggerManager {
    public:
        LoggerManager();
        void init();

        Logger::ptr getLogger(const std::string &name);
        Logger::ptr getRoot() const { return m_root; }
    private:
        std::map<std::string, Logger::ptr> m_loggers;
        Logger::ptr m_root;
    };

    typedef liucxi::Singleton<LoggerManager> LoggerMgr;
}
#endif //LUWU_LOG_H
